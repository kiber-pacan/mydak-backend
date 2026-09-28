#ifndef MYDAK_BACKEND_CORE_CLIENT_HPP
#define MYDAK_BACKEND_CORE_CLIENT_HPP


#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <queue>

#include "channel_holder.hpp"
#include "dialog.hpp"
#include "identity.hpp"
#include "namer.hpp"
#include "parameters_accessor.hpp"
#include "qt_ptrs.hpp"
#include "toml++/toml.h"


enum class message_type;
namespace asio = boost::asio;
class QObject;

namespace mydak {
	using signal_channel = asio::experimental::channel<void(boost::system::error_code)>; }

namespace mydak {
	struct client_detail {
		// Using unique_ptr because of delayed init
		std::unique_ptr<signal_channel> send_channel_ptr;
		std::unique_ptr<signal_channel> receive_channel_ptr;

		std::queue<std::string> messages_queue{};

		std::array<unsigned char, proto::E2E_KEYS_RAW_L> current_recipient{};
		std::unordered_map<std::array<unsigned char, proto::E2E_KEYS_RAW_L>,
						   dialog, tools::char_array_hasher> dialogs;
	};

	struct qt_handler {
		qt_handler() = delete;
		explicit qt_handler(client_detail* detail_ptr) : detail_ptr(detail_ptr) {}

		// Messages
		void qt_add_message(std::string_view message, message_type type) const;

		// User rectangle
		void qt_set_user_name(std::string_view name) const;

		void qt_set_user_icon(std::string_view icon) const;

		// Dialog messages
		void qt_clear_current_dialog() const;

		// Recipient bar
		void qt_set_recipient_name(
			const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& recipient
		) const;

		// Dialogs
		void qt_add_dialog(
			const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& client
		) const;

		void qt_clear_dialogs() const;

		qt_ptrs qt_pointers;
		client_detail* detail_ptr;
	};

	struct client : std::enable_shared_from_this<client> {
		client(
			asio::io_context& io,
			const qt_ptrs& qt_pointers,
			args::parameters_accessor& parameters
		) : io(io),
			ip(parameters.get<"--ip">()),
			port(parameters.get<"--port">()),
			parameters(parameters),
			qt(&detail)
		{
			id = identity(parameters.get<"--login">(), parameters.get<"--password">());
			if (const auto recipient = parameters.get<"--recipient">(); std::size(recipient) > 0) {
				std::array<char, proto::E2E_KEYS_RAW_L> recipient_bin; // NOLINT(*-pro-type-member-init)
				tools::hex2bin(recipient, recipient_bin.data(), std::size(recipient_bin));
				set_recipient(recipient_bin.data());
			}

			// Qt
			qt.qt_pointers = qt_ptrs(qt_pointers);
			qt.qt_set_user_name(namer::get_name(id.public_key_value));
			qt.qt_set_user_icon(namer::get_icon(id.public_key_value));

			load_dialogs();
			qt.qt_set_recipient_name(detail.current_recipient);
		}



		#pragma region Main
		asio::awaitable<void> initialize(int current_try);
 
		asio::awaitable<void> receive_loop();

		asio::awaitable<void> send_loop();

		asio::awaitable<void> listen_loop();

		void send_message(std::string_view message);

		void set_recipient(const void* ptr);

		void try_add_dialog(const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& recipient);

		void add_message_to_dialog(
			const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& sender,
			std::string_view message,
			message_type type
		);

		void save_dialogs();

		void load_dialogs();

		void set_dialog(const std::array<unsigned char, proto::E2E_KEYS_RAW_L>& client);
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