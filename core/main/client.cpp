#include <algorithm>
#include <bit>
#include <boost/asio/use_awaitable.hpp>
#include <exception>
#include <iostream>
#include <string_view>
#include <random>

#include <sys/ioctl.h>
#include <unistd.h>
#include <sodium.h>

#include "client.hpp"

#include <qobjectdefs.h>
#include <qstring.h>
#include <qvariant.h>

#include "brotli.hpp"
#include "coh.hpp"
#include "logger.hpp"
#include "namer.hpp"
#include "util/proto.hpp"

namespace asio = boost::asio;

constexpr std::string_view FUCKED_UP_MESSAGE_SIZE =
	"Total message size is not in bounds!";


#pragma region Main
asio::awaitable<void> mydak::client::initialize(const int current_try) {
	const auto wait_time = parameters.get<"--wait_time">();
	const auto wait_time_add = parameters.get<"--wait_time_add">();
	const auto connect_tries = parameters.get<"--connect_tries">();
	try {
		int wait_seconds = wait_time * (current_try > 0) + (wait_time_add == -1 ? wait_time : wait_time_add) * std::max(0, current_try - 1);
		
		if (wait_seconds > 0) logger::log_debug(std::format("Waiting: {} seconds", wait_seconds));
		
		asio::steady_timer timer(io, asio::chrono::seconds(wait_seconds));
		co_await timer.async_wait(asio::use_awaitable);
		
		if (wait_seconds > 0) logger::log_debug("Trying to connect...");

		// Trying to connect
		asio::ip::tcp::resolver resolver(io);
		socket = std::make_shared<asio::ip::tcp::socket>(io);
		co_await asio::async_connect(*socket, resolver.resolve(ip, std::to_string(port)));

		logger::log_debug("Connected!");

		// Creating channels
		client_detail.send_channel_ptr = std::make_shared<signal_channel>(socket->get_executor());
		client_detail.receive_channel_ptr = std::make_shared<signal_channel>(socket->get_executor());
	}
	catch (const boost::system::system_error& e) {
		logger::exception_func(e);

		// Exit after N tries
		if (current_try >= connect_tries - 1) {
			logger::exit(std::format("Failed after {} tries!", connect_tries));
		}

		// Another try
		coh::detached(
			initialize(current_try + 1)
		);
	}
	
	co_return;
}

// Receive messages from the server loop
asio::awaitable<void> mydak::client::receive_loop() const {
	try {
		for (;;)  {
			// [message size][public key]
			// normally 4 + 32
			std::array<char, proto::E2E_KEYS_RAW_L + proto::MESSAGE_SIZE_L> greetings{};
			co_await asio::async_read(*socket, asio::buffer(greetings, greetings.size()), asio::use_awaitable);

			uint32_t message_size;
			std::memcpy(
				&message_size,
				greetings.data(),
				proto::MESSAGE_SIZE_L
			);

			if (message_size < 1) continue;

			// Getting first 32 chars aka public key
			std::array<unsigned char, proto::E2E_KEYS_RAW_L> sender_public_key{};
			memcpy(
				sender_public_key.data(),
				greetings.data() + proto::MESSAGE_SIZE_L,
				std::size(sender_public_key)
			);



			std::vector<unsigned char> raw_message{};
			raw_message.resize(message_size);

			// Receiving message
			co_await asio::async_read(*socket, asio::buffer(raw_message.data(), raw_message.size()), asio::use_awaitable);

			// Decoding -> decompressing
			std::vector<unsigned char> message = brotli::decompress(id.decode_message(sender_public_key, raw_message));
			//std::vector<unsigned char> message = brotli::decompress(raw_message);


			#pragma region gap shenanigans
			winsize size{};
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

			std::string gap;

			const int gap_size = static_cast<int>(size.ws_col - message.size() - 2);

			if (gap_size > 0) {
				gap = std::string(static_cast<size_t>(gap_size), ' ');
			}
			#pragma endregion

			// Printing message with name based in public key hash
			auto message_view = std::string_view(reinterpret_cast<const char*>(message.data()), std::size(message));
			std::string formatted = std::format("{}{}", gap, message_view);
			logger::log(std::format("{} : {}", namer::get_name(id.public_key_value), formatted));

			qt_add_recipient_message(message_view);
		}
	} catch (const std::exception& e) {
		logger::log_func_error(e.what());
		co_return;
	}
	co_return;
}

// Send messages to the server loop
asio::awaitable<void> mydak::client::send_loop() {
	try {
		// Sending our public key so we can get registered on the server
		co_await asio::async_write(*socket, asio::buffer(id.public_key), asio::use_awaitable);

		// Main loop
		for (;;) {
			co_await client_detail.send_channel_ptr->async_receive(asio::use_awaitable);

			for (; not client_detail.messages_queue.empty(); client_detail.messages_queue.pop()) {
				std::string& message_raw = client_detail.messages_queue.front();

				#pragma region Command parser
				if (message_raw[0] == '/') {
					if (message_raw[1] == 'r' && message_raw[2] == ' ') {
						if (sodium_hex2bin(
							client_detail.current_recipient.data(),
							std::size(client_detail.current_recipient),
							message_raw.data() + 3,
							std::size(message_raw) - 3, nullptr, nullptr, nullptr // length of {/r }
						) != 0) {
							logger::exit_func("Failed to convert hex to binary");
						}
						set_recipient(message_raw.data() + 3);

						continue;
					}
					logger::log_error("Something is wrong");

					continue;
				}

				// SET YOUR FUCKING RECIPIENT YOU STUPID WHORE
				if (client_detail.current_recipient.empty()) {
					logger::log_error("No recipient provided. /r <RECIPIENT>");
					continue;
				}
				#pragma endregion

				// Preparing greetings packet
				std::array prefix{proto::GREETINGS_PREFIX};

				// Compress and encrypt message
				// Compressing -> encoding
				const auto processed_message = id.encode_message(client_detail.current_recipient, brotli::compress(message_raw));
				//const auto processed_message = brotli::compress(message_raw);



				// Getting encrypted message size
				#pragma region Message size
				const auto encrypted_size = static_cast<uint32_t>(std::size(processed_message));
				std::size_t message_size = std::size(processed_message);
				// TODO MAKE NOTIFICATION IN QT
				if (message_size < proto::MIN_MESSAGE_SIZE || message_size > proto::MAX_MESSAGE_SIZE) {
					logger::log_debug_error(
						std::format(
							"{} ({})",
							FUCKED_UP_MESSAGE_SIZE,
							message_size
						)
					);
					break;
				}

				std::array<char, proto::MESSAGE_SIZE_L> size_array{};

				// We send message in little-endian,
				// so on the server side we should use byte swap if server using big-endian
				if constexpr (std::endian::native == std::endian::big) {
					size_array = std::bit_cast<std::array<char, proto::MESSAGE_SIZE_L>>(std::byteswap(encrypted_size));
				} else {
					size_array = std::bit_cast<std::array<char, proto::MESSAGE_SIZE_L>>(encrypted_size);
				}
				#pragma endregion

				// sending [0x67][message size][recipient][message] packet
				#pragma region Sending
				// GREETINGS
				co_await asio::async_write(*socket, asio::buffer(prefix), asio::use_awaitable);
				co_await asio::async_write(*socket, asio::buffer(size_array), asio::use_awaitable);
				co_await asio::async_write(*socket, asio::buffer(client_detail.current_recipient), asio::use_awaitable);

				// MESSAGE
				co_await asio::async_write(*socket, asio::buffer(processed_message), asio::use_awaitable);
				#pragma endregion

				qt_add_sender_message(message_raw);
			}
		}
	} catch (const std::exception& e) {
		logger::log_func_error(e.what());
		co_return;
	}
	co_return;
}

void mydak::client::send_message(const std::string& message) {
	// Add message to the queue
	client_detail.messages_queue.emplace(message);

	// Notify client about change
	boost::system::error_code e;
	client_detail.send_channel_ptr->try_send(e);
}

void mydak::client::set_recipient(const void* ptr) {
	memcpy(
		client_detail.current_recipient.data(),
		ptr,
		std::size(client_detail.current_recipient)
	);

	std::uint16_t value;
	memcpy(&value, ptr, sizeof(value));

	QMetaObject::invokeMethod(
		qt_pointers.recipient_bar,
		"set_name",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(namer::get_name(value)))
	);
}

void mydak::client::set_recipient(const std::vector<unsigned char>& recipient) {
	memcpy(
		client_detail.current_recipient.data(),
		recipient.data(),
		std::size(client_detail.current_recipient)
	);

	std::uint16_t value;
	memcpy(&value, recipient.data(), sizeof(value));

	QMetaObject::invokeMethod(
		qt_pointers.recipient_bar,
		"set_name",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(namer::get_name(value)))
	);

}
#pragma endregion


#pragma region Qt
void mydak::client::qt_add_sender_message(const std::string_view message) const {
	const std::size_t message_size = std::size(message);
	if (message_size < proto::MIN_MESSAGE_SIZE || message_size > proto::MAX_MESSAGE_SIZE) return;

	QMetaObject::invokeMethod(
		qt_pointers.dialog_rectangle,
		"add_message",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(message.data(), message_size)),
		Q_ARG(QVariant, 0)
	);
}

void mydak::client::qt_add_recipient_message(const std::string_view message) const {
	const std::size_t message_size = std::size(message);
	if (message_size < proto::MIN_MESSAGE_SIZE || message_size > proto::MAX_MESSAGE_SIZE) return;

	QMetaObject::invokeMethod(
		qt_pointers.dialog_rectangle,
		"add_message",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(message.data(), message_size)),
		Q_ARG(QVariant, 1)
	);
}

void mydak::client::qt_set_user_name(const std::string_view name) const {
	const std::size_t string_size = std::size(name);
	if (string_size < 1) return;

	QMetaObject::invokeMethod(
		qt_pointers.user_bar,
		"set_user_name",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(name.data(), string_size))
	);
}

void mydak::client::qt_set_user_icon(const std::string_view icon) const {
	const std::size_t string_size = std::size(icon);
	if (string_size != 4) return;

	QMetaObject::invokeMethod(
		qt_pointers.user_bar,
		"set_user_icon",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(icon.data(), string_size))
	);
}
#pragma endregion