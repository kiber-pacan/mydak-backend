//
// Created by down on 21.08.2026.
//

#include "local_server.hpp"
#include "client.hpp"

// Receive messages from frontend
asio::awaitable<void> mydak::server_connection::receive_loop() {
    auto ptr = shared_from_this();

    try {
        for (;;) {
            // 4 chars of message size
            std::array<char, 4> message_size_raw{};
            uint32_t message_size{};

            co_await asio::async_read(*socket, asio::buffer(message_size_raw, 4), asio::use_awaitable);
            std::memcpy(&message_size, message_size_raw.data(), message_size_raw.size());

            // Always use little endian
            if constexpr (std::endian::native == std::endian::big) message_size = std::byteswap(message_size);

            // [0x67][recipient][message]
            std::vector<char> message_raw{};
            message_raw.resize(message_size);
            co_await asio::async_read(*socket, asio::buffer(message_raw, message_size), asio::use_awaitable);


            //std::cout << std::string(message_raw.data(), message_raw.size()) << std::endl;

            // check if starts with [0x67]
            if (message_raw[0] != 0x67) {
                std::cout << "Wrong message format! " << std::endl;
                continue;
            }

            std::span<char> message_span{message_raw};

            std::span<char> public_key = message_span.subspan(1, 64);
            std::span<char> message = message_span.subspan(65, message_size - 65);


            client->recipient = std::string(public_key.data(), public_key.size());
            client->messages.emplace(message.data(), message.size());

            const boost::system::error_code e;
            co_await client->send_channel_ptr->async_send(e, asio::use_awaitable);

        }
    } catch (const std::exception& e) {
        std::cout << std::format("exception in {} : {}", __func__, e.what()) << std::endl;
    }

    co_return;
}

// Receive messages to frontend
asio::awaitable<void> mydak::server_connection::send_loop() {
    auto ptr = shared_from_this();

    try {
        for (;;) {
            co_await client->receive_channel_ptr->async_receive();


        }
    } catch (const std::exception& e) {
        std::cout << std::format("exception in {} : {}", __func__, e.what()) << std::endl;
    }

    co_return;
}


void mydak::local_server::local_receive() {
    auto ptr = shared_from_this();

    auto connection = std::make_shared<server_connection>(io, client);

    acceptor->async_accept(
        *connection->socket,
        [this, connection](const boost::system::error_code& e) {
            if (!e) {
                this->handle_connection(connection);
            } else {
                std::cout << std::format("Acceptor exception: {}", e.message()) << std::endl;
            }
        }
    );
}

void mydak::local_server::handle_connection(std::shared_ptr<server_connection> connection) {
    // Repeat connection loop
    local_receive();

    asio::co_spawn(
        io,
        connection->receive_loop(),
        asio::detached
    );

    asio::co_spawn(
        io,
        connection->send_loop(),
        asio::detached
    );
}