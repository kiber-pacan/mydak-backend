//
// Created by akicatt on 19.08.2026.
//

#ifndef MYDAK_BACKEND_HANDLER_H
#define MYDAK_BACKEND_HANDLER_H

#include <boost/asio.hpp>

#include "client.hpp"
#include "logger.hpp"
namespace asio = boost::asio;

namespace mydak {
    struct handler : std::enable_shared_from_this<handler> {
        handler(asio::io_context& io, const std::shared_ptr<client>& client)
        : io(io), client(client) {}

        asio::io_context& io;
        const std::shared_ptr<client>& client;

        // [0x67][recipient][message]
        asio::awaitable<void> send_message_co(std::vector<char> message_raw) {
            auto ptr = shared_from_this();

            try {
                const std::uint32_t message_size = std::size(message_raw) - proto::PUBLIC_KEY_L - proto::GREETINGS_PREFIX_L;

                // check if starts with [0x67]
                if (message_raw[0] != 0x67) {
                    logger::log_func_debug_error("Wrong message format!");
                    co_return;
                }

                std::span<char> message_span{message_raw};

                std::span<char> public_key = message_span.subspan(1, 64);
                std::span<char> message = message_span.subspan(65, message_size - 65);

                client->recipient = std::string(public_key.data(), public_key.size());
                client->messages.emplace(message.data(), message.size());

                boost::system::error_code e;
                co_await client->send_channel_ptr->async_send(e, asio::use_awaitable);

            } catch (const std::exception& e) {
                logger::log_func_debug_error(std::format("exception in {} : {}", __func__, e.what()));
            }

            co_return;
        }

        // Receive messages to frontend
        asio::awaitable<void> send_loop() {
            auto ptr = shared_from_this();

            try {
                for (;;) {
                    co_await client->receive_channel_ptr->async_receive();

                }
            } catch (const std::exception& e) {
                logger::log_func_debug_error(std::format("exception in {} : {}", __func__, e.what()));
            }

            co_return;
        }
    };

    void send_message(const char* message_raw) {
        asio::co_spawn(io, send_message_co(message_raw), asio::detached);
    }
}



#endif //MYDAK_BACKEND_HANDLER_H
