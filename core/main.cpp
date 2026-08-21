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

#include "local_server.hpp"
#include "client.hpp"
#include "coh.hpp"
#include "identity.hpp"
#include "namer.hpp"

namespace asio = boost::asio;

namespace mydak {
	static void input(const std::shared_ptr<client>& client, asio::io_context& io, const std::shared_ptr<local_server>& local_server) {
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
					local_server->local_receive();
				});
			} else {
				client->messages.emplace(input);
				boost::system::error_code e;

				asio::co_spawn(
					io,
					[client]() -> asio::awaitable<void>{
						boost::system::error_code e;
						co_await client->send_channel_ptr->async_send(e, asio::use_awaitable);
						co_return;
					},
					asio::detached
				);
			}
		}
	}
}





int main(int argc, char* argv[]) {
	if (sodium_init() != 0)
		throw std::runtime_error("Failed to init sodium!");


	auto& io = mydak::coh::io();
	auto client = std::make_shared<mydak::client>(io, "127.0.0.1", "8888", argc, argv);

	auto local_server = std::make_shared<mydak::local_server>(io, client);

	mydak::coh::detached(client->initialize(0));

	// TODO MAYBE REMOVE
	io.run();
	io.restart();

	mydak::coh::detached(client->receive());
	mydak::coh::detached(client->send());

	std::thread thread(mydak::input, client, std::ref(io), local_server);
	thread.detach();

	io.run();

	return 0;
}
