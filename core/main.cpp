#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <iostream>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <thread>
#include <sodium.h>

#include "local_server.hpp"
#include "client.hpp"
#include "coh.hpp"
#include "identity.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

namespace asio = boost::asio;

namespace mydak {
	static void input(
	client& client,
		asio::io_context& io,
		const std::shared_ptr<local_server>& local_server
	) {
		for (;;) {
			// INPUT
			std::string input{};
			std::getline(std::cin, input);


			if (input == "/server") {
				asio::post(io, [local_server, &io]() {
					local_server->acceptor = std::make_shared<asio::ip::tcp::acceptor>(
						io,
						asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), 6767)
					);
					local_server->local_receive();
				});
			} else {
				client.messages.emplace(input);

				coh::detached(
					[client]() -> asio::awaitable<void>{
						const boost::system::error_code e;
						co_await client.send_channel_ptr->async_send(e, asio::use_awaitable);
						co_return;
					}
				);
			}
		}
	}

	static int qt(
		int argc, char* argv[],
		QObject*& messages_rectangle
	) {
		QGuiApplication app(argc, argv);
		QQmlApplicationEngine app_engine{};

		app_engine.loadFromModule("mydak_backend", "Main");

		if (app_engine.rootObjects().isEmpty()) return -1;
		auto main = app_engine.rootObjects().constFirst();
		messages_rectangle = main->findChild<QObject*>("messages_rectangle");

		return app.exec();
	}
}



int main(int argc, char* argv[]) {
	if (sodium_init() != 0)
		throw std::runtime_error("Failed to init sodium!");

	// QT
	QObject* messages_rectangle;
	std::thread qt_thread(mydak::qt, argc, argv, std::ref(messages_rectangle));
	qt_thread.detach();

	auto& io = mydak::coh::io();
	mydak::client client(io, "127.0.0.1", "8888", messages_rectangle, argc, argv);
	//89d8deaddeffef6e8479965176daaeb88e253c25046a65f180d030423f881d73
	//03d4e313161c0b208a514452e1171032e2025182ae7ed6233d9384bfdb18d210
	mydak::coh::detached(client.initialize(0));

	io.run();
	io.restart();

	mydak::coh::detached(client.receive());
	mydak::coh::detached(client.send());


	// INPUT
	auto local_server = std::make_shared<mydak::local_server>(io, client);

	std::thread input_thread(mydak::input, std::ref(client), std::ref(io), local_server);
	input_thread.detach();

	io.run();

	return 0;
}
