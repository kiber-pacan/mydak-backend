//
// Created by akicatt on 27.09.2026.
//

#ifndef MYDAK_BACKEND_CHANNEL_HOLDER_H
#define MYDAK_BACKEND_CHANNEL_HOLDER_H

#include "proto.hpp"
#include "boost/asio/experimental/channel.hpp"
namespace asio = boost::asio;

namespace mydak {
    // Singleton for easier connection of mydak backend and qt backend
    struct channel_holder {
        using set_dialog_channel_t =
            asio::experimental::channel<
                void(
                    boost::system::error_code,
                    std::array<unsigned char, proto::E2E_KEYS_RAW_L>
                )
            >;

        static inline std::unique_ptr<set_dialog_channel_t> set_dialog_channel;

    };
}


#endif //MYDAK_BACKEND_CHANNEL_HOLDER_H
