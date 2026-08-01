#ifndef MYDAK_BACKEND_CORE_CLIENT_HPP
#define MYDAK_BACKEND_CORE_CLIENT_HPP


#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <queue>

#include "params.hpp"

namespace asio = boost::asio;

namespace mydak { using send_channel = asio::experimental::channel<void(boost::system::error_code)>; }

namespace mydak {
	struct client : public std::enable_shared_from_this<client> {

		client(asio::io_context& io, const char*&& ip, const char*&& port, const std::vector<mydak::args::parameter_variant>& parameters)
			:
			io(io),
			ip(ip),
			port(port),
			parameters_vector(parameters)
		{
			set_parameters(
				0,
				connect_tries,
				wait_time,
				wait_time_add,
				public_key,
				recipient
			);
		}

		// Stub method
		void set_pars(size_t counter) {}

		// I think it's just easier to do this shit
		template<typename... T>
		void set_parameters(size_t counter, T&... parameters) {
			(([&](auto& parameter_ref){
				auto& variant = parameters_vector[counter++];
				variant.visit([&](auto&& parameter) {
					if constexpr (std::is_assignable_v<decltype(parameter_ref), decltype(parameter.get_data())>)
						parameter_ref = parameter.get_data();
				});
			}(parameters)), ...);
		}
	
	
		asio::awaitable<void> initialize(int current_try);
 
		asio::awaitable<void> receive() const;

		asio::awaitable<void> send();

		// VARIABLES	
		std::shared_ptr<asio::io_context> websocket_io;

		asio::io_context& io;
		std::shared_ptr<asio::ip::tcp::socket> socket;

		std::string ip, port;

		const std::vector<mydak::args::parameter_variant>& parameters_vector;
		int8_t connect_tries = 1;
		int8_t wait_time = 1;
		int8_t wait_time_add = 1;
		std::string	public_key;
		std::string recipient;
	
		std::shared_ptr<mydak::send_channel> send_channel;
		std::shared_ptr<mydak::send_channel> receive_channel;
	
		std::queue<std::string> messages{};
	};
}

#endif  // MYDAK_BACKEND_CORE_CLIENT_HPP
