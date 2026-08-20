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

#include "main/client.hpp"

namespace asio = boost::asio;


int main(int argc, char* argv[]) {
	if (sodium_init() != 0) throw std::runtime_error("Failed to init sodium");

	asio::io_context io{};
	std::shared_ptr<mydak::client> client = std::make_shared<mydak::client>(io, "127.0.0.1", "8888", argc, argv);
	


	local_server->io.run();

	return 0;
}
