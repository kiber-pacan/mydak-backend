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
#include <QFontDatabase>

#include "qt_ptrs.hpp"

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
		qt_ptrs& ptrs, std::promise<void>& qt_signal_promise
	) {
		QGuiApplication app(argc, argv);

		// FONT
		const int font_id = QFontDatabase::addApplicationFont(":/core/Qt/fonts/fantasque_sans_mono/ttf/regular.ttf");
		if (QStringList font_families = QFontDatabase::applicationFontFamilies(font_id);
		!font_families.empty()) {
			QGuiApplication::setFont(font_families.first());
		} else {
			logger::log_func_debug_error("Failed to load font");
		}

		QQmlApplicationEngine app_engine{};
		app_engine.loadFromModule("mydak_backend", "Main");

		if (app_engine.rootObjects().isEmpty()) return -1;
		auto main = app_engine.rootObjects().constFirst();

		// QT PTRS
		ptrs.messages = main->findChild<QObject*>("messages_rectangle");
		qt_signal_promise.set_value();



		return QGuiApplication::exec();
	}
}



int main(int argc, char* argv[]) {
	if (sodium_init() != 0)
		throw std::runtime_error("Failed to init sodium!");

	#pragma region QT
	mydak::qt_ptrs qt_pointers;
	std::promise<void> qt_signal;
	std::thread qt_thread(mydak::qt, argc, argv, std::ref(qt_pointers), std::ref(qt_signal));
	qt_signal.get_future().get(); // Wait for qt_thread to get all the refs

	qt_thread.detach();
	#pragma endregion

	auto& io = mydak::coh::io();
	mydak::client client(io, "127.0.0.1", "8888", qt_pointers, argc, argv);

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
