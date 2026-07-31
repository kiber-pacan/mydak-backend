#ifndef MYDAK_BACKEND_CORE_CLIENT_HPP
#define MYDAK_BACKEND_CORE_CLIENT_HPP


#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <queue>

namespace asio = boost::asio;

namespace mydak { using send_channel = asio::experimental::channel<void(boost::system::error_code)>; }

namespace mydak {
	struct client : public std::enable_shared_from_this<client> {
		client(asio::io_context& io, const char*&& ip, const char*&& port)
			:
			io(io),
			ip(ip),
			port(port)
		{

		}

		// Stub method
		void set_pars(size_t counter) {}

		// I think it's just easier to do this magic 
		template<typename... T>
		void set_pars(size_t counter, T&... pars_ref) {
			((pars_ref = pars[counter++]), ...);
		}
	
	
		asio::awaitable<void> initialize(int current_try);
 
		asio::awaitable<void> receive() const;

		asio::awaitable<void> send();

		// VARIABLES	
		std::shared_ptr<asio::io_context> websocket_io;

		asio::io_context& io;
		std::shared_ptr<asio::ip::tcp::socket> socket;

		std::string ip, port;
		std::string recipient{};
		std::vector<int> pars{};
		// WHAT THE FUCK IS THIS

		int connect_tries = 1;
		int wait_time = 1;
		int wait_time_add = 1;
	
		std::shared_ptr<mydak::send_channel> send_channel;
		std::shared_ptr<mydak::send_channel> receive_channel;
	
		std::queue<std::string> messages{};
	};
}

#endif  // MYDAK_BACKEND_CORE_CLIENT_HPP
