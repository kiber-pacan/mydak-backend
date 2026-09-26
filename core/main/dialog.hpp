//
// Created by down on 16.09.2026.
//

#ifndef MYDAK_BACKEND_DIALOG_H
#define MYDAK_BACKEND_DIALOG_H

#include "proto.hpp"
#include <array>
#include <string>
#include <vector>

#include "message_type.hpp"


namespace mydak {
    struct dialog {
        ~dialog() = default;
        dialog(const dialog& d) = delete;
        dialog(
            const std::array<unsigned char, proto::E2E_KEYS_RAW_L> &recipient
        ) : recipient(recipient) {}

        void add_message(std::string_view message, message_type type) {
            messages.emplace_back(message, type);
        }

        [[nodiscard]] auto& get_recipient() const  {
            return recipient;
        }

        [[nodiscard]] auto& get_messages() const {
            return messages;
        }
    private:
        std::array<unsigned char, proto::E2E_KEYS_RAW_L> recipient;
        std::vector<std::pair<std::string, message_type>> messages{};
    };
}

#endif //MYDAK_BACKEND_DIALOG_H
