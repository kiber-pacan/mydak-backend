#include <algorithm>
#include <boost/asio/detached.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <bits/chrono.h>
#include <cstdint>
#include <cstring>
#include <exception>
#include <iostream>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <sodium.h>
#include <map>
#include <limits>
#include <unordered_set>

#include "client.hpp"

namespace asio = boost::asio;

namespace mydak {
	struct server_connection : public std::enable_shared_from_this<server_connection> {
		server_connection(asio::io_context& io, const std::shared_ptr<mydak::client>& client) :
			socket(std::make_shared<asio::ip::tcp::socket>(io)),
			client(client)
		{}

		std::shared_ptr<asio::ip::tcp::socket> socket;
		const std::shared_ptr<mydak::client>& client;
		

		// Receive messages from frontend
		asio::awaitable<void> receive_loop() {
			auto ptr = shared_from_this();

			try {
				for (;;) {
					// 4 chars of message size
					std::array<char, 4> message_size_raw{};
					uint32_t message_size{};
					
					co_await asio::async_read(*socket, asio::buffer(message_size_raw, 4), asio::use_awaitable);
					std::memcpy(&message_size, message_size_raw.data(), message_size_raw.size());

					// Always use little endian
					if constexpr (std::endian::native == std::endian::big) message_size = std::byteswap(message_size);

					// [0x67][recipient][message]
					std::vector<char> message_raw{};
					message_raw.resize(message_size);
					co_await asio::async_read(*socket, asio::buffer(message_raw, message_size), asio::use_awaitable);


					//std::cout << std::string(message_raw.data(), message_raw.size()) << std::endl;
					
					// check if starts with [0x67]
					if (message_raw[0] != 0x67) {
						std::cout << "Wrong message format! " << std::endl;
						continue;
					}

					std::span<char> message_span{message_raw};
					
					std::span<char> public_key = message_span.subspan(1, 64);
					std::span<char> message = message_span.subspan(65, message_size - 65);
					

					client->recipient = std::string(public_key.data(), public_key.size());
					client->messages.emplace(std::string(message.data(), message.size()));

					boost::system::error_code e;
					co_await client->send_channel->async_send(e, asio::use_awaitable);
							
				}
			} catch (const std::exception& e) {
				std::cout << std::format("exception in {} : {}", __func__, e.what()) << std::endl;
			}
			
			co_return;
		}

		// Receive messages to frontend
		asio::awaitable<void> send_loop() {
			auto ptr = shared_from_this();

			try {
				for (;;) {
					co_await client->receive_channel->async_receive();

					
				}
			} catch (const std::exception& e) {
				std::cout << std::format("exception in {} : {}", __func__, e.what()) << std::endl;
			}
			
			co_return;
		}
	};

	struct local_server : public std::enable_shared_from_this<local_server> {
		local_server(asio::io_context& io, const std::shared_ptr<mydak::client>& client) :
			io(io),
			client(client)
		{}
	
		std::string recipient{};
		asio::io_context& io;
		const std::shared_ptr<mydak::client>& client;

		std::shared_ptr<asio::ip::tcp::acceptor> acceptor{};
	
		void localRecieve() {			
			auto ptr = shared_from_this();
			
			auto connection = std::make_shared<server_connection>(io, client);
		
			acceptor->async_accept(
				*connection->socket,
				[this, connection](const boost::system::error_code& e) {
					if (!e) {
						this->handleConnection(connection);
					} else {
						std::cout << std::format("Acceptor exception: {}", e.message()) << std::endl;
					}
				}
			);
		}

		void handleConnection(std::shared_ptr<server_connection> connection) {
			// Repeat connection loop
			localRecieve();

			asio::co_spawn(
				io,
				connection->receive_loop(),
				asio::detached
			);

			asio::co_spawn(
				io,
				connection->send_loop(),
				asio::detached
			);	
		}
	};

	static void input(const std::shared_ptr<mydak::client>& client, asio::io_context& io, const std::shared_ptr<mydak::local_server>& local_server) {
		for (;;) {
			// INPUT
			std::string input{};
			std::getline(std::cin, input);


			if (input == "/server") {
				asio::post(io, [local_server, &io]() {
					local_server->acceptor = std::make_shared<asio::ip::tcp::acceptor>(
						io, 
						asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), 6767)
					);
					local_server->localRecieve();
				});
			} else {
				client->messages.emplace(input);
				boost::system::error_code e;
			
				asio::co_spawn(
					io,
					[client]() -> asio::awaitable<void>{
						boost::system::error_code e;
						co_await client->send_channel->async_send(e, asio::use_awaitable);
						co_return;
					},
					asio::detached
				);
			}
		}
	}
}
namespace mydak {
	template <uint8_t type>
	struct parameter {
		parameter(uint8_t length) : length(length) {}
		parameter(int8_t min, int8_t max) : min(min), max(max) {}

		bool can_set_value(std::string_view string) {
		}

		bool can_set_value(int8_t num) {
		}

		private:
		uint8_t length{};
		int8_t min{};
		int8_t max{};
	};

	struct parameters {
		inline static const uint8_t max_uint8_t = std::numeric_limits<uint8_t>::max();
		inline static const int8_t max_int8_t = std::numeric_limits<int8_t>::max();
	
		static inline const std::unordered_map<std::string, size_t> existing{
			{"--connect-tries", 0},
			{"--wait-time", 1},
			{"--wait-time-add", 2},
			{"--public-key", 3}
		};

		using variant = std::variant<parameter<0>, parameter<1>>;
		inline static const std::vector<variant> values{
			parameter<1>(1, max_int8_t),
			parameter<1>(0, max_int8_t),
			parameter<1>(-1, max_int8_t),
			parameter<0>(64)
		};
	
	};
}

static bool is_number(std::string_view string) {
	for (const char& character : string) if (!std::isdigit(character)) return false;
	return true;
}

static void args(int argc, char* argv[]) {
	auto values = mydak::parameters::values;
	for (int i = 1; i < argc; i++) {
		std::string raw = argv[i];
		auto equals_pos = raw.find("=");
		if (equals_pos == std::string::npos) {
			std::cerr << std::format("Wrong parameter format: {}. (no equals symbol)", raw) << std::endl;
			std::exit(0);
		}

		std::string parameter = raw.substr(0, equals_pos);
		std::string_view value = raw.subview(equals_pos + 1, raw.size() - equals_pos - 1);

		if (value.empty()) {
			std::cerr << std::format("Empty value: {}", parameter) << std::endl;
			std::exit(0);
		}
		
		auto it = mydak::parameters::existing.find(parameter);
		
		if (it == mydak::parameters::existing.end()) {
			std::cerr << std::format("Wrong parameter: {}. seek help. (--help)", parameter) << std::endl;
			std::exit(0);
		}


		const size_t& index = it->second;
		const auto& data_type = mydak::parameters::values[index];

		std::visit([](auto&& foo) { foo; }, data_type);
		/*
		switch (data_type->) {
			// String
		    case 0: {
				
				break;
			}
			// Integer
		    case 1: {
				break;
			}
		}*/
	}
}


int main(int argc, char* argv[]) {
	if (sodium_init() != 0) throw std::runtime_error("Failed to init sodium");
	args(argc, argv);
	/*
	asio::io_context io{};
	std::shared_ptr<mydak::client> client = std::make_shared<mydak::client>(io, "127.0.0.1", "8888");
	
	std::shared_ptr<mydak::local_server> local_server = std::make_shared<mydak::local_server>(io, client);
	

	asio::co_spawn(
		local_server->io,
		client->initialize(0),
		asio::detached
	);
	
	local_server->io.run();

	local_server->io.restart();

	asio::co_spawn(
		local_server->io,
		client->receive(),
		asio::detached
	);
	
	asio::co_spawn(
		local_server->io,
		client->send(),
		asio::detached
	);

	
	std::thread thread(mydak::input, client, std::ref(io), local_server);
	thread.detach();

	//asio::ip::tcp::acceptor acceptor(local_server->io, asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), 6767));
	//local_server->localRecieve(acceptor);
	
	local_server->io.run();

	return 0;
	*/
}
