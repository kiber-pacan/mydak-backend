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

#include <QFontDatabase>
#include <qqmlcontext.h>

#include "client.hpp"
#include "coh.hpp"
#include "qt_connector.hpp"

namespace asio = boost::asio;

namespace mydak {
	static void input(
	client& client,
		asio::io_context& io
	) {
		for (;;) {
			// INPUT
			std::string input{};
			std::getline(std::cin, input);

			client.send_message(input);
		}
	}

	static int qt_initialization(
		int argc, char* argv[],
		qt_ptrs& qt_pointers, std::promise<void>& qt_signal_promise
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
		const auto main = app_engine.rootObjects().constFirst();

		// QT PTRS
		qt_pointers.messages_rectangle = main->findChild<QObject*>("messages_rectangle");
		qt_pointers.recipient_rectangle = main->findChild<QObject*>("recipient_rectangle");
		qt_pointers.app = &app;
		qt_pointers.app_engine = &app_engine;
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
	std::thread qt_thread(mydak::qt_initialization, argc, argv, std::ref(qt_pointers), std::ref(qt_signal));
	qt_signal.get_future().get(); // Wait for qt_thread to get all the refs

	qt_thread.detach();
	#pragma endregion

	auto& io = mydak::coh::io();
	mydak::client client(io, "127.0.0.1", "8888", qt_pointers, argc, argv);

	// Qt late init
	mydak::qt_connector connector(client);
	qt_pointers.app_engine->rootContext()->setContextProperty("qt_connector", &connector);

	mydak::coh::detached(client.initialize(0));

	io.run();
	io.restart();

	mydak::coh::detached(client.receive_loop());
	mydak::coh::detached(client.send_loop());


	std::thread input_thread(mydak::input, std::ref(client), std::ref(io));
	input_thread.detach();

	io.run();

	return 0;
}
