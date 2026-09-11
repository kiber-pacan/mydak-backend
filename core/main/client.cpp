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

#include "brotli.hpp"
#include "coh.hpp"
#include "logger.hpp"
#include "namer.hpp"
#include "util/proto.hpp"

namespace asio = boost::asio;

asio::awaitable<void> mydak::client::initialize(const int current_try) {
	try {
		int wait_seconds = wait_time * (current_try > 0) + (wait_time_add == -1 ? wait_time : wait_time_add) * std::max(0, current_try - 1);
		
		if (wait_seconds > 0) logger::log_debug(std::format("Waiting: {} seconds", wait_seconds));
		
		asio::steady_timer timer(io, asio::chrono::seconds(wait_seconds));
		co_await timer.async_wait(asio::use_awaitable);
		
		if (wait_seconds > 0) logger::log_debug("Trying to connect...");

		// Trying to connect
		asio::ip::tcp::resolver resolver(io);
		socket = std::make_shared<asio::ip::tcp::socket>(io);
		co_await asio::async_connect(*socket, resolver.resolve(ip, port));

		logger::log_debug("Connected!");

		// Creating channels
		send_channel_ptr = std::make_shared<send_channel>(socket->get_executor());
		receive_channel_ptr = std::make_shared<send_channel>(socket->get_executor());
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
asio::awaitable<void> mydak::client::receive() {
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
			std::string formatted = std::format("{}{}", gap, std::string_view(reinterpret_cast<const char*>(message.data()), std::size(message)));
			logger::log(std::format("{} : {}", namer::get_name(id.public_key_value), formatted));
		}
	} catch (const std::exception& e) {
		logger::log_func_error(e.what());
		co_return;
	}
	co_return;
}

// Send messages to the server loop
asio::awaitable<void> mydak::client::send() {
	try {
		/*
		// Getting new public key if public_key is not the right size (Probably empty!)
		if (std::size(id.public_hex) != proto::E2E_KEYS_HEX_L) {
			constexpr size_t bin_len = proto::E2E_KEYS_RAW_L;

			std::array<char, proto::E2E_KEYS_HEX_L> hex{};
			randombytes_buf(public_key.data(), bin_len);

			tools::bin2hex(public_key, hex.data(), std::size(hex));

			logger::log(std::format("Generated key: {}", std::string_view(hex.data(), std::size(hex) - 1))); // THROWING OUT NULL TERMINATOR
		}
		*/

		// Sending our public key so we can get registered on the server
		co_await asio::async_write(*socket, asio::buffer(id.public_key), asio::use_awaitable);

		// Main loop
		for (;;) {
			co_await send_channel_ptr->async_receive(asio::use_awaitable);

			for (; not messages.empty(); messages.pop()) {
				std::string& message_raw = messages.front();

				#pragma region Command parser
				if (message_raw[0] == '/') {
					if (message_raw[1] == 'r' && message_raw[2] == ' ') {
						if (sodium_hex2bin(
							recipient.data(),
							std::size(recipient),
							message_raw.data() + 3,
							std::size(message_raw) - 3, nullptr, nullptr, nullptr // length of {/r }
						) != 0) {
							logger::exit_func("Failed to convert hex to binary");
						}
						recipient_hex = std::string(message_raw.data() + 3, std::size(message_raw) - 3);

						continue;
					}
					logger::log_error("Something is wrong");

					continue;
				}

				// SET YOUR FUCKING RECIPIENT YOU STUPID WHORE
				if (recipient_hex.empty()) {
					logger::log_error("No recipient provided. /r <RECIPIENT>");
					continue;
				}
				#pragma endregion

				// Preparing greetings packet
				std::array prefix{proto::GREETINGS_PREFIX};

				// Compress and encrypt message
				// Compressing -> encoding
				const auto processed_message = id.encode_message(recipient, brotli::compress(message_raw));
				//const auto processed_message = brotli::compress(message_raw);



				// Getting encrypted message size
				#pragma region Message size
				const auto encrypted_size = static_cast<uint32_t>(std::size(processed_message));
				std::array<char, proto::MESSAGE_SIZE_L> size{};

				// We send message in little-endian,
				// so on the server side we should use byte swap if server using big-endian
				if constexpr (std::endian::native == std::endian::big) {
					size = std::bit_cast<std::array<char, proto::MESSAGE_SIZE_L>>(std::byteswap(encrypted_size));
				} else {
					size = std::bit_cast<std::array<char, proto::MESSAGE_SIZE_L>>(encrypted_size);
				}
				#pragma endregion

				// sending [0x67][message size][recipient][message] packet
				#pragma region Sending
				// GREETINGS
				co_await asio::async_write(*socket, asio::buffer(prefix), asio::use_awaitable);
				co_await asio::async_write(*socket, asio::buffer(size), asio::use_awaitable);
				co_await asio::async_write(*socket, asio::buffer(recipient), asio::use_awaitable);

				// MESSAGE
				co_await asio::async_write(*socket, asio::buffer(processed_message), asio::use_awaitable);
				#pragma endregion
			}
		}
	} catch (const std::exception& e) {
		logger::log_func_error(e.what());
		co_return;
	}
	co_return;
}