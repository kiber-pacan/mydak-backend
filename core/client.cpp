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
#include "logger.hpp"
#include "namer.hpp"
#include "proto.hpp"

namespace asio = boost::asio;

asio::awaitable<void> mydak::client::initialize(const int current_try) {
	try {
		int wait_seconds = wait_time * (current_try > 0) + ((wait_time_add == -1) ? wait_time : wait_time_add) * std::max(0, current_try - 1);
		
		mydak::logger::log_debug(std::format("Waiting: {} seconds", wait_seconds));
		
		asio::steady_timer timer(io, asio::chrono::seconds(wait_seconds));
		
		co_await timer.async_wait(asio::use_awaitable);
		
		mydak::logger::log_debug("Trying to connect...");

		asio::ip::tcp::resolver resolver(io);

		socket = std::make_shared<asio::ip::tcp::socket>(io);

		co_await asio::async_connect(*socket, resolver.resolve(ip, port));
		
		send_channel = std::make_shared<mydak::send_channel>(socket->get_executor());
		receive_channel = std::make_shared<mydak::send_channel>(socket->get_executor());
	}
	catch (const boost::system::system_error& e) {
		mydak::logger::exception_func(e);


		if (current_try >= connect_tries - 1) {
			mydak::logger::exit(std::format("Failed after {} tries!", connect_tries));
		}
		
		asio::co_spawn(
			io,
			initialize(current_try + 1),
			asio::detached
		);
	}
	
	co_return;
}

asio::awaitable<void> mydak::client::receive() const {
	try {
		for (;;)  {
			std::array<char, mydak::proto::PUBLIC_KEY_L + mydak::proto::MESSAGE_SIZE_L> key_and_size{};
			co_await asio::async_read(*socket, asio::buffer(key_and_size, key_and_size.size()), asio::use_awaitable);


			uint32_t message_size;
			std::memcpy(
				&message_size,
				std::span(key_and_size)
				.subspan(
					mydak::proto::PUBLIC_KEY_L,
					mydak::proto::MESSAGE_SIZE_L
				)
				.data(),
				mydak::proto::MESSAGE_SIZE_L
			);
			if (message_size < 1) continue;

			std::string key = std::string(std::span(key_and_size).subspan(0, mydak::proto::PUBLIC_KEY_L).data(), mydak::proto::PUBLIC_KEY_L);

			std::string message{}; message.resize(message_size);
			
			co_await asio::async_read(*socket, asio::buffer(message.data(), message.size()), asio::use_awaitable);
			std::string decomressed = mydak::brotli::decompress(message);

			winsize size{};
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);


			std::string gap;

			const int gap_size = static_cast<int>(size.ws_col - decomressed.size() - 2);

			if (gap_size > 0) {
				gap = std::string(static_cast<size_t>(gap_size), ' ');
			}
			
			std::string formatted = std::format("{}{}", gap, decomressed);
			
			std::cout << mydak::namer::get_name(key) << " : " << formatted << std::endl;
		}
	}
	catch (const boost::system::system_error& e) {
		mydak::logger::exception_func(e);
	}
	co_return;
}

asio::awaitable<void> mydak::client::send() {
	try {
		if (public_key.size() != mydak::proto::PUBLIC_KEY_L) {
			std::array<char, mydak::proto::PUBLIC_KEY_L> public_key_array{};

			constexpr size_t bin_len = mydak::proto::PUBLIC_KEY_L / 2;
			constexpr size_t hex_len = mydak::proto::PUBLIC_KEY_L + 1;

			unsigned char bin[bin_len];
			std::array<char, hex_len> hex{};


			randombytes_buf(bin, bin_len);

			if (sodium_bin2hex(hex.data(), hex_len, bin, bin_len) == nullptr) {
				throw std::runtime_error("Failed to convert bytes to hex string");
			}

			std::ranges::copy_n(hex.begin(), mydak::proto::PUBLIC_KEY_L, public_key_array.begin());
			public_key = std::string(public_key_array.data(), public_key_array.size());

			std::cout << std::string(public_key) << std::endl;
		}

		co_await asio::async_write(*socket, asio::buffer(public_key), asio::use_awaitable);
		
		for (;;) {
			co_await send_channel->async_receive(asio::use_awaitable);

			for (; not messages.empty(); messages.pop()) {
				std::string& message_raw = messages.front();

				// COMMANDS
				if (message_raw[0] == '/') {
					if (message_raw[1] == 'r' && message_raw[2] == ' ') {
						std::string_view recipient_view = std::string_view(message_raw).substr(3, message_raw.size() - 3);
						recipient = std::string(recipient_view);
						
						continue;
					} else {
						std::cout << "Something is wrong" << std::endl;
						
						continue;
					}
				}
				
				if (recipient.empty()) {
					std::cout << "No recipient provided. /r <RECIPIENT>" << std::endl;
					continue;
				}
				
				
				// GREETINGS
				std::array<char, mydak::proto::GREETINGS_PREFIX_L> type = {mydak::proto::GREETINGS_PREFIX};
				std::string message_compressed = mydak::brotli::compress(message_raw);
				const auto raw_size = static_cast<uint32_t>(message_compressed.size());
				std::array<char, mydak::proto::MESSAGE_SIZE_L> size{};

				if constexpr (std::endian::native == std::endian::big) {
					size = std::bit_cast<std::array<char, mydak::proto::MESSAGE_SIZE_L>>(std::byteswap(raw_size));
				} else {
					size = std::bit_cast<std::array<char, mydak::proto::MESSAGE_SIZE_L>>(raw_size);
				}
			   
				co_await asio::async_write(*socket, asio::buffer(type), asio::use_awaitable);
				co_await asio::async_write(*socket, asio::buffer(size), asio::use_awaitable);
				co_await asio::async_write(*socket, asio::buffer(recipient), asio::use_awaitable);

				
				// MESSAGE
				co_await asio::async_write(*socket, asio::buffer(message_compressed), asio::use_awaitable);
			}
		}
	}
	catch (const boost::system::system_error& e) {
		logger::exception_func(e);
	}
	co_return;
}
