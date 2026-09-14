#ifndef MYDAK_BACKEND_CORE_CLIENT_HPP
#define MYDAK_BACKEND_CORE_CLIENT_HPP


#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <queue>

#include "identity.hpp"
#include "namer.hpp"
#include "parameters.hpp"
#include "parameters_accessor.hpp"
#include "qt_ptrs.hpp"
#include "toml++/toml.h"


class QObject;
namespace asio = boost::asio;

namespace mydak {
	using signal_channel = asio::experimental::channel<void(boost::system::error_code)>; }

namespace mydak {
	struct client_detail {
		std::shared_ptr<signal_channel> send_channel_ptr;
		std::shared_ptr<signal_channel> receive_channel_ptr;

		std::queue<std::string> messages_queue{};

		std::array<unsigned char, proto::E2E_KEYS_RAW_L> recipient{};

	};
	struct client : std::enable_shared_from_this<client> {
		client(
			asio::io_context& io,
			const char*&& ip,
			const char*&& port,
			const qt_ptrs& qt_pointers,
			int argc,
			char* argv[]
		) : io(io),
			ip(ip),
			port(port),
			qt_pointers(qt_pointers)
		{
			auto p = args::parameters_accessor(argc, argv);
			auto d = p.get<"--connect_tries">();
			set_parameters(
				p,
				connect_tries,
				wait_time,
				wait_time_add,
				recipient_hex,
				login,
				password
			);


			// TODO REWORK THIS PIECE OF SHIT
			// KEYPAIR START
			id = identity(login, password);
			if (std::size(recipient_hex) > 0) {
				auto recipient_bin = tools::hex2bin(recipient_hex);
				set_recipient(recipient_bin);
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

		#pragma region Main
		asio::awaitable<void> initialize(int current_try);
 
		asio::awaitable<void> receive_loop() const;

		asio::awaitable<void> send_loop();

		void send_message(const std::string& message);

		void set_recipient(const std::vector<unsigned char>& recipient) {
			memcpy(
				client_detail.recipient.data(),
				recipient.data(),
				std::size(client_detail.recipient)
			);

			std::uint16_t value;
			memcpy(&value, recipient.data(), sizeof(value));

			QMetaObject::invokeMethod(
				qt_pointers.recipient_rectangle,
				"set_name",
				Qt::QueuedConnection,
				Q_ARG(QVariant, QString::fromUtf8(namer::get_name(value)))
			);

		}
		#pragma endregion

		#pragma region Qt
		void qt_add_sender_message(std::string_view message) const;

		void qt_add_recipient_message(std::string_view message) const;
		#pragma endregion

		// VARIABLES	
		std::shared_ptr<asio::io_context> websocket_io;

		asio::io_context& io;
		std::shared_ptr<asio::ip::tcp::socket> socket;

		std::string ip, port;

		std::int8_t connect_tries{};
		std::int8_t wait_time{};
		std::int8_t wait_time_add{};

		std::string recipient_hex{};

		identity id;
		std::string login{};
		std::string_view password{};

		// QT
		qt_ptrs qt_pointers;

		client_detail client_detail;
	};
}

#endif  // MYDAK_BACKEND_CORE_CLIENT_HPP