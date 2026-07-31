#ifndef MYDAK_BACKEND_CORE_CLIENT_HPP
#define MYDAK_BACKEND_CORE_CLIENT_HPP


#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <queue>

namespace asio = boost::asio;

namespace mydak { using send_channel = asio::experimental::channel<void(boost::system::error_code)>; }

namespace mydak {
	struct client : public std::enable_shared_from_this<client> {
		client(asio::io_context& io, std::string ip, std::string port, const std::vector<int>& pars)
			:
			ip(ip),
			port(port),
			io(io),
			pars(pars)
		{
			// WHAT THE FUCK IS THIS
			set_pars(
				0,
				connect_tries,
				wait_time,
				wait_time_add
			);
		}

		client(asio::io_context& io, std::string ip, std::string port)
			:
			ip(ip),
			port(port),
			io(io)
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
 
		asio::awaitable<void> receive();

		asio::awaitable<void> send();

		// VARIABLES	
		std::shared_ptr<asio::io_context> websocket_io;

		asio::io_context& io;
		std::shared_ptr<asio::ip::tcp::socket> socket;

		std::string ip, port;
		std::string recipient{};
		std::vector<int> pars{};
		// WHAT THE FUCK IS THIS
		int debug = 1;

		int connect_tries = 1;
		int wait_time = 1;
		int wait_time_add = 1;
	
		std::shared_ptr<mydak::send_channel> send_channel;
		std::shared_ptr<mydak::send_channel> receive_channel;
	
		std::queue<std::string> messages{};
	
	private:

		asio::awaitable<void> connect();
	};
}

#endif  // MYDAK_BACKEND_CORE_CLIENT_HPP
