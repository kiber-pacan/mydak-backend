//
// Created by down on 21.08.2026.
//

#ifndef MYDAK_BACKEND_LOCAL_SERVER_H
#define MYDAK_BACKEND_LOCAL_SERVER_H


#include "brotli.hpp"
#include <boost/asio.hpp>

namespace asio = boost::asio;

namespace mydak {
	struct client;

	struct server_connection : std::enable_shared_from_this<server_connection> {
		server_connection(asio::io_context& io, const std::shared_ptr<client>& client) :
			socket(std::make_shared<asio::ip::tcp::socket>(io)),
			client(client)
		{}

		std::shared_ptr<asio::ip::tcp::socket> socket;
		const std::shared_ptr<client>& client;


		// Receive messages from frontend
		asio::awaitable<void> receive_loop();

		// Receive messages to frontend
		asio::awaitable<void> send_loop();
	};

	struct local_server : std::enable_shared_from_this<local_server> {
		local_server(asio::io_context& io, const std::shared_ptr<client>& client) :
			io(io),
			client(client)
		{}

		std::string recipient{};
		asio::io_context& io;
		const std::shared_ptr<client>& client;

		std::shared_ptr<asio::ip::tcp::acceptor> acceptor{};


		void local_receive();

		void handle_connection(std::shared_ptr<server_connection> connection);
	};
}

#endif //MYDAK_BACKEND_LOCAL_SERVER_H
