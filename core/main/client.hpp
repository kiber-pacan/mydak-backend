#ifndef MYDAK_BACKEND_CORE_CLIENT_HPP
#define MYDAK_BACKEND_CORE_CLIENT_HPP


#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <queue>

#include "identity.hpp"
#include "parameters.hpp"
#include "parameters_accessor.hpp"
#include "toml++/toml.h"

namespace asio = boost::asio;

namespace mydak { using send_channel = asio::experimental::channel<void(boost::system::error_code)>; }

namespace mydak {
	struct client : std::enable_shared_from_this<client> {

		client(asio::io_context& io, const char*&& ip, const char*&& port, int argc, char* argv[])
			:
			io(io),
			ip(ip),
			port(port)
		{
			set_parameters(
				args::parameters_accessor(argc, argv),
				connect_tries,
				wait_time,
				wait_time_add,
				recipient_hex,
				local_server,
				login,
				password
			);


			// TODO REWORK THIS PIECE OF SHIT
			// KEYPAIR START
			id = identity(login, password);
			if (std::size(recipient_hex) > 0) {
				auto bin = tools::hex2bin(recipient_hex);
				memcpy(
					recipient.data(),
					bin.data(),
					std::size(recipient)
				);
			}
			// KEYPAIR END
		}

		// I think it's just easier to do this shit
		template<typename... T>
		static void set_parameters(const args::parameters_accessor parameters_accessor, T&... parameters) {
			tools::constexpr_for<args::parameters_count>(
				[&] (auto index) {
					parameters...[index] = parameters_accessor.get<index>();
				}
			);
		}

		static void load_keypair(std::string_view private_key_path) {
			toml::table keypair;
		}

	
		asio::awaitable<void> initialize(int current_try);
 
		asio::awaitable<void> receive();

		asio::awaitable<void> send();

		// VARIABLES	
		std::shared_ptr<asio::io_context> websocket_io;

		asio::io_context& io;
		std::shared_ptr<asio::ip::tcp::socket> socket;

		std::string ip, port;

		std::int8_t connect_tries{};
		std::int8_t wait_time{};
		std::int8_t wait_time_add{};

		std::string recipient_hex{};
		std::int8_t local_server;

		identity id;
		std::array<unsigned char, proto::E2E_KEYS_RAW_L> recipient{};
		std::string login{};
		std::string_view password{};


		std::shared_ptr<send_channel> send_channel_ptr;
		std::shared_ptr<send_channel> receive_channel_ptr;

		std::queue<std::string> messages{};
	};
}

#endif  // MYDAK_BACKEND_CORE_CLIENT_HPP