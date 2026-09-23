//
// Created by down on 16.09.2026.
//

#ifndef MYDAK_BACKEND_DIALOG_H
#define MYDAK_BACKEND_DIALOG_H

#include "proto.hpp"
#include <array>
#include <string>
#include <vector>


namespace mydak {
    struct dialog {
        ~dialog() = default;
        dialog(const dialog& d) = delete;
        dialog(
            const std::array<unsigned char, proto::E2E_KEYS_RAW_L> &recipient
        ) : recipient(recipient) {}

        void add_message(std::string_view message) {
            messages.emplace_back(message);
        }

        auto& get_recipient() {
            return recipient;
        }

        auto& get_messages() {
            return messages;
        }
    private:
        std::array<unsigned char, proto::E2E_KEYS_RAW_L> recipient;
        std::vector<std::string> messages{};
    };
}

#endif //MYDAK_BACKEND_DIALOG_H
