#ifndef MYDAK_BACKEND_CORE_CLIENT_HPP
#define MYDAK_BACKEND_CORE_CLIENT_HPP


#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <queue>

#include "dialog.hpp"
#include "identity.hpp"
#include "namer.hpp"
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

		std::array<unsigned char, proto::E2E_KEYS_RAW_L> current_recipient{};
		std::unordered_map<std::array<unsigned char, proto::E2E_KEYS_RAW_L>,
						   dialog, tools::char_array_hasher> dialogs;
	};

	struct qt_handler {
		// Messages
		void qt_add_sender_message(std::string_view message) const;

		void qt_add_recipient_message(std::string_view message) const;


		// User rectangle
		void qt_set_user_name(std::string_view name) const;

		void qt_set_user_icon(std::string_view icon) const;

		qt_ptrs qt_pointers;
	};

	struct client : std::enable_shared_from_this<client> {
		client(
			asio::io_context& io,
			const qt_ptrs& qt_pointers,
			args::parameters_accessor& parameters
		) : io(io),
			ip(parameters.get<"--ip">()),
			port(parameters.get<"--port">()),
			parameters(parameters)
		{
			id = identity(parameters.get<"--login">(), parameters.get<"--password">());
			if (const auto recipient = parameters.get<"--recipient">(); std::size(recipient) > 0) {
				const auto recipient_bin = tools::hex2bin(recipient);
				set_recipient(recipient_bin.data());
			}

			// Qt
			qt.qt_pointers = qt_ptrs(qt_pointers);
			qt.qt_set_user_name(namer::get_name(id.public_key_value));
			qt.qt_set_user_icon(namer::get_icon(id.public_key_value));

			if (!detail.current_recipient.empty()) {
				add_dialog(detail.current_recipient);
			}
		}

		#pragma region Main
		asio::awaitable<void> initialize(int current_try);
 
		asio::awaitable<void> receive_loop();

		asio::awaitable<void> send_loop();

		void send_message(std::string_view message);

		void set_recipient(const void* ptr);

		void add_dialog(const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& recipient);

		enum class message_type {
			sender,
			recipient
		};

		void add_message(
			const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& recipient,
			std::string_view message,
			message_type type
		);
		#pragma endregion



		// VARIABLES	
		std::shared_ptr<asio::io_context> websocket_io;

		asio::io_context& io;
		std::shared_ptr<asio::ip::tcp::socket> socket;

		std::string ip;
		std::uint16_t port;

		args::parameters_accessor& parameters;

		identity id;

		qt_handler qt;
		client_detail detail;
	};
}

#endif  // MYDAK_BACKEND_CORE_CLIENT_HPP