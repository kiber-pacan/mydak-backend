#ifndef MYDAK_BACKEND_CORE_CLIENT_HPP
#define MYDAK_BACKEND_CORE_CLIENT_HPP


#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <queue>

#include "parameters.hpp"

namespace asio = boost::asio;

namespace mydak { using send_channel = asio::experimental::channel<void(boost::system::error_code)>; }

namespace mydak {
	struct client : public std::enable_shared_from_this<client> {

		client(asio::io_context& io, const char*&& ip, const char*&& port, const mydak::args::parameters_accessor& parameters)
			:
			io(io),
			ip(ip),
			port(port)
		{
			set_parameters(
				parameters,
				connect_tries,
				wait_time,
				wait_time_add,
				public_key,
				recipient
			);
		}

		// I think it's just easier to do this shit
		template<typename... T>
		void set_parameters(const mydak::args::parameters_accessor parameters_accessor, T&... parameters) {
			mydak::tools::constexpr_for<args::parameters_count{}>(
				[&] (auto index) {
					parameters...[index] = parameters_accessor.get<index>();
				}
			);

		}
	
	
		asio::awaitable<void> initialize(int current_try);
 
		asio::awaitable<void> receive() const;

		asio::awaitable<void> send();

		// VARIABLES	
		std::shared_ptr<asio::io_context> websocket_io;

		asio::io_context& io;
		std::shared_ptr<asio::ip::tcp::socket> socket;

		std::string ip, port;

		int8_t connect_tries{};
		int8_t wait_time{};
		int8_t wait_time_add{};
		std::string public_key{};
		std::string recipient{};
	
		std::shared_ptr<mydak::send_channel> send_channel;
		std::shared_ptr<mydak::send_channel> receive_channel;
	
		std::queue<std::string> messages{};
	};
}

#endif  // MYDAK_BACKEND_CORE_CLIENT_HPP
