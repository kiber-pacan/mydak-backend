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
#include "channel_holder.hpp"
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
		detail.send_channel_ptr = std::make_unique<signal_channel>(socket->get_executor());
		detail.receive_channel_ptr = std::make_unique<signal_channel>(socket->get_executor());
		channel_holder::set_dialog_channel = std::make_unique<channel_holder::set_dialog_channel_t>(socket->get_executor());
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
asio::awaitable<void> mydak::client::receive_loop() {
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

			add_message(sender_public_key, message_view, message_type::recipient);
			add_dialog(sender_public_key);
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
			co_await detail.send_channel_ptr->async_receive(asio::use_awaitable);

			for (; not detail.messages_queue.empty(); detail.messages_queue.pop()) {
				std::string& message_raw = detail.messages_queue.front();

				#pragma region Command parser
				if (message_raw[0] == '/') {
					if (message_raw[1] == 'r' && message_raw[2] == ' ') {
						if (sodium_hex2bin(
							detail.current_recipient.data(),
							std::size(detail.current_recipient),
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
				if (detail.current_recipient.empty()) {
					logger::log_error("No recipient provided. /r <RECIPIENT>");
					continue;
				}
				#pragma endregion

				// Preparing greetings packet
				std::array prefix{proto::GREETINGS_PREFIX};

				// Compress and encrypt message
				// Compressing -> encoding
				const auto processed_message = id.encode_message(detail.current_recipient, brotli::compress(message_raw));
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
				co_await asio::async_write(*socket, asio::buffer(detail.current_recipient), asio::use_awaitable);

				// MESSAGE
				co_await asio::async_write(*socket, asio::buffer(processed_message), asio::use_awaitable);
				#pragma endregion

				add_message(detail.current_recipient, message_raw, message_type::sender);
			}
		}
	} catch (const std::exception& e) {
		logger::log_func_error(e.what());
		co_return;
	}
	co_return;
}

// Listen to various channels
asio::awaitable<void> mydak::client::listen_loop() {
	try {
		for (;;)  {
			auto client = co_await channel_holder::set_dialog_channel->async_receive();
			set_dialog(client);
		}
	} catch (const std::exception& e) {
		logger::log_func_error(e.what());
		co_return;
	}
	co_return;
}



void mydak::client::send_message(std::string_view message) {
	// Add message to the queue
	detail.messages_queue.emplace(message);

	// Notify client about change
	boost::system::error_code e;
	detail.send_channel_ptr->try_send(e);
}

void mydak::client::set_recipient(const void* ptr) {
	memcpy(
		detail.current_recipient.data(),
		ptr,
		std::size(detail.current_recipient)
	);

	std::uint16_t value;
	memcpy(&value, ptr, sizeof(value));

	QMetaObject::invokeMethod(
		qt.qt_pointers.recipient_bar,
		"set_name",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(namer::get_name(value)))
	);
}

void mydak::client::add_dialog(const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& recipient) {
	const auto it = detail.dialogs.find(recipient);
	// In emplace call the first argument is key and the second is argument for passing it into
	// the dialog constructor
	if (it == detail.dialogs.end()) {
		detail.dialogs.emplace(recipient, recipient);

		qt.qt_add_dialog(recipient);
	}
}

void mydak::client::add_message(
	const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& sender,
	const std::string_view message,
	const message_type type
) {
	dialog* dialog;

	const auto it0 = detail.dialogs.find(sender);
	if (it0 == detail.dialogs.end()) {
		auto [it1, inserted] = detail.dialogs.emplace(sender, sender);

		if (!inserted) return;
		dialog = &it1->second;
	} else {
		dialog = &it0->second;
	}

	dialog->add_message(message, type);
	// We wont add message to the display if current dialog is not
	// with sender
	if (sender != detail.current_recipient) return;

	qt.qt_add_message(message, type);
}

void mydak::client::save_dialogs() {
	std::filesystem::create_directories("dialogs");

	toml::table dialog_file;
	toml::array dialogs;
	for (const auto& [recipient, dialog] : detail.dialogs) {

		toml::array messages;
		for (const auto& [message, type] : dialog.get_messages()) {
			messages.emplace_back(toml::array{message, static_cast<uint8_t>(type)});
		}

		dialogs.emplace_back(toml::array{tools::bin2hex_string(recipient), messages});
	}

	dialog_file.insert_or_assign("dialogs", dialogs);

	std::ofstream file;
	file.open(std::format("dialogs/{}_dialog.toml", parameters.get<"--login">()));
	file << dialog_file;
	file.flush();
	file.close();

	logger::log_func_debug("Saved dialogs to file");
}

void mydak::client::load_dialogs() {
	const std::string filename = std::format("dialogs/{}_dialog.toml", parameters.get<"--login">());
	if (!std::filesystem::exists(filename)) {
		if (!detail.current_recipient.empty()) {
			add_dialog(detail.current_recipient);
		}
		return;
	}

	toml::table file = toml::parse_file(filename);

	toml::array data;
	if (const auto* data_ptr = file["dialogs"].as_array()) data = *data_ptr;

	// pseudo structure for better understanding
	// n: 0-infinite
	// data: {
	//     { client-0, [ {message-0, type}, ... , {message-n, type} ] },
	//     ... ,
	//     { client-n, [ {message-0, type}, ... , {message-n, type} ] },
	// }
	//
	// dialogs -> dialog{ [client, messages], ...}, ...

	// Looping thru dialogs
	for (const auto& dialog : data | tools::toml_to_array) {
		const auto client_opt = dialog->at(0).value<std::string>();
		const auto* message_datas_ptr = dialog->at(1).as_array();

		// Check client and messages array
		if (!client_opt.has_value() && !message_datas_ptr) return;

		const auto& client_hex = client_opt.value();
		std::array<unsigned char, proto::E2E_KEYS_RAW_L> client; // NOLINT(*-pro-type-member-init)
		tools::hex2bin(client_hex, client.data(), std::size(client));

		add_dialog(client);
		for (const auto& message_data : *message_datas_ptr | tools::toml_to_array) {
			const auto message = message_data->at(0).value<std::string>();
			const auto type = message_data->at(1).value<std::uint8_t>();

			if (!message.has_value() && !type.has_value()) return;

			add_message(client, message.value(), static_cast<message_type>(type.value()));
		}
	}
}

void mydak::client::set_dialog(const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& client) {
	qt.qt_clear_current_dialog();
	qt.qt_set_recipient_name(client);

	detail.current_recipient = client;
	for (const auto& [message, type] : detail.dialogs[client].get_messages()) {
		qt.qt_add_message(message, type);
	}
}
#pragma endregion


#pragma region Qt
// Messages
void mydak::qt_handler::qt_add_message(std::string_view message, message_type type) const {
	const std::size_t message_size = std::size(message);
	if (message_size < proto::MIN_MESSAGE_SIZE || message_size > proto::MAX_MESSAGE_SIZE) return;

	QMetaObject::invokeMethod(
		qt_pointers.dialog_rectangle,
		"add_message",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(message.data(), message_size)),
		Q_ARG(QVariant, static_cast<int>(type))
	);
}


// User rectangle
void mydak::qt_handler::qt_set_user_name(const std::string_view name) const {
	const std::size_t string_size = std::size(name);
	if (string_size < 1) return;

	QMetaObject::invokeMethod(
		qt_pointers.user_bar,
		"set_user_name",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(name.data(), string_size))
	);
}

void mydak::qt_handler::qt_set_user_icon(const std::string_view icon) const {
	const std::size_t string_size = std::size(icon);
	if (string_size != 4) return;

	QMetaObject::invokeMethod(
		qt_pointers.user_bar,
		"set_user_icon",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(icon.data(), string_size))
	);
}


// Dialog messages
void mydak::qt_handler::qt_clear_current_dialog() const {
	QMetaObject::invokeMethod(
		qt_pointers.dialog_rectangle,
		"clear",
		Qt::QueuedConnection
	);
}


// Recipient bar
void mydak::qt_handler::qt_set_recipient_name(
	const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& recipient
) const {
	uint16_t value;
	memcpy(&value, recipient.data(), sizeof(value));
	const auto name = namer::get_name(value);

	QMetaObject::invokeMethod(
		qt_pointers.recipient_bar,
		"set_name",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QString::fromUtf8(name.data(), std::size(name)))
	);
}


// Dialogs
void mydak::qt_handler::qt_add_dialog(const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& client) const {
	QMetaObject::invokeMethod(
		qt_pointers.dialog_selector,
		"add_dialog",
		Qt::QueuedConnection,
		Q_ARG(QVariant, QByteArray(reinterpret_cast<const char*>(client.data()), std::size(client)))

	);
}

void mydak::qt_handler::qt_clear_dialogs() const {
	QMetaObject::invokeMethod(
		qt_pointers.dialog_selector,
		"clear",
		Qt::QueuedConnection
	);
}
#pragma endregion