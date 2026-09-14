//
// Created by akicatt on 21.08.2026.
//

#ifndef MYDAK_BACKEND_KEYPAIR_H
#define MYDAK_BACKEND_KEYPAIR_H
#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

#include "logger.hpp"
#include "proto.hpp"
#include "sodium.h"
#include "tools.hpp"
#include "toml++/toml.hpp"

namespace mydak {
    struct identity_detail {
        // public key - shared secret
        mutable std::map<std::array<unsigned char, proto::E2E_KEYS_RAW_L>, std::array<unsigned char, proto::E2E_KEYS_RAW_L>> shared_secrets_cache;
    };
    struct identity { // NOLINT(*-pro-type-member-init)
        #pragma region variables
        // TODO I THINK MOST OF IT IS REDUNDANT TRASH, REMOVE IT
        std::array<unsigned char, proto::E2E_KEYS_RAW_L> public_key{};
        std::array<unsigned char, proto::E2E_KEYS_RAW_L> private_key{};

        std::array<unsigned char, crypto_pwhash_SALTBYTES> salt;
        std::array<unsigned char, crypto_secretbox_NONCEBYTES> nonce;

        std::array<unsigned char, 32> password_hash;
        std::array<unsigned char, 48> private_key_encoded;

        std::string_view login_view;
        std::string_view password_view;
        std::string filename;

        std::uint16_t public_key_value;

        identity_detail identity_detail;

        static constexpr std::uint32_t OPSLIMIT = 2;
        static constexpr std::uint32_t MEMLIMIT = 134217728;
        #pragma endregion

        identity() = default;

        identity(const std::string_view login, const std::string_view password) { // NOLINT(*-pro-type-member-init)
            this->password_view = password;
            this->login_view = login;

            // If login provided
            if (std::size(login_view) > 0) {
                // And file for that login exists
                if (filename = std::format("{}.toml", std::string_view(login_view.data(), std::size(login_view))); std::filesystem::exists(filename)) {
                    // Read credentials
                    load_keypair();
                    std::memcpy(&public_key_value, public_key.data(), sizeof(decltype(public_key_value)));
                    return;
                }
            }

            // Generate fresh credentials
            initialize_credentials();
            save_keypair();
        }


        #pragma region initialization
        void initialize_credentials() {
            if (crypto_box_keypair(public_key.data(), private_key.data()) != 0)
                throw std::runtime_error("Failed to generate key!");

            logger::log(std::format("Your public key is: {}", tools::bin2hex_string(public_key)));

            // Generate salt and nonce
            randombytes_buf(salt.data(), std::size(salt));
            randombytes_buf(nonce.data(), std::size(nonce));

            // Generate password hash
            if (crypto_pwhash(
                password_hash.data(),
                std::size(password_hash),
                password_view.data(),
                password_view.size(),
                salt.data(),
                OPSLIMIT,
                MEMLIMIT,
                crypto_pwhash_ALG_DEFAULT
            ) != 0) throw std::runtime_error("Failed to create password hash!");

            // Encode private key with newly generated password hash
            if (crypto_secretbox_easy(
                private_key_encoded.data(),
                private_key.data(),
                32,
                nonce.data(),
                password_hash.data()
            ) != 0) throw std::runtime_error("Failed to encode private key!");

            // Get public_key_value from public key
            std::memcpy(&public_key_value, public_key.data(), sizeof(decltype(public_key_value)));

            filename = std::format("{}.toml", login_view);
        }
        #pragma endregion


        #pragma region save-load
        void save_keypair() {
            try {
                toml::table keypair_file;

                keypair_file.insert_or_assign("salt", tools::bin2hex_string(salt));
                keypair_file.insert_or_assign("nonce", tools::bin2hex_string(nonce));
                keypair_file.insert_or_assign("opslimit", OPSLIMIT);
                keypair_file.insert_or_assign("memlimit", MEMLIMIT);
                keypair_file.insert_or_assign("password_hash", tools::bin2hex_string(password_hash));

                keypair_file.insert_or_assign("private_key_encoded", tools::bin2hex_string(private_key_encoded));

                // Save file to the system
                std::ofstream file;
                file.open(filename);
                file << keypair_file;
                file.flush();
                file.close();

                logger::log_debug("Saved credentials to file");
            } catch (const std::exception& e) {
                logger::exit_func(e.what());
            }
        }


        template <std::size_t N, typename T>
        void try_load_value(std::array<T, N>& value, const char* name) {
            toml::table keypair_file = toml::parse_file(filename);

            if (const auto value_opt = keypair_file[name].value<std::string>(); value_opt.has_value())
                value = tools::hex2bin<N>(value_opt.value());
            else throw std::runtime_error(std::format("Invalid {}!", name));
        }

        template <typename T>
        void try_load_value(T& value, const char* name) {
            toml::table keypair_file = toml::parse_file(filename);

            if (const auto value_opt = keypair_file[name].value<T>(); value_opt.has_value())
                value = value_opt.value();
            else throw std::runtime_error(std::format("Invalid {}!", name));
        }

        void load_keypair() {
            try {
                // Load data from file
                toml::table keypair_file = toml::parse_file(filename);
                try_load_value(salt, "salt");

                std::size_t opslimit;
                try_load_value(opslimit, "opslimit");
                std::size_t memlimit;
                try_load_value(memlimit, "memlimit");
                try_load_value(password_hash, "password_hash");

                try_load_value(nonce, "nonce");
                try_load_value(private_key_encoded, "private_key_encoded");


                // Generate new hash from given password and compare that hash with the file one
                std::array<unsigned char, 32> password_hash_copy; // NOLINT(*-pro-type-member-init)
                if (crypto_pwhash(
                    password_hash_copy.data(),
                    std::size(password_hash_copy),
                    password_view.data(),
                    password_view.size(),
                    salt.data(),
                    opslimit,
                    memlimit,
                    crypto_pwhash_ALG_DEFAULT
                ) != 0) throw std::runtime_error("Failed to create password hash!");
                if (password_hash != password_hash_copy) throw std::runtime_error("Failed to get the same password hash!");


                // Decode private key
                if (crypto_secretbox_open_easy(
                    private_key.data(),
                    private_key_encoded.data(),
                    std::size(private_key_encoded),
                    nonce.data(),
                    password_hash.data()
                ) != 0) throw std::runtime_error("Failed to decode private key!");

                // Generate public key from private key
                crypto_scalarmult_base(
                    public_key.data(),
                    private_key.data()
                );

                logger::log(std::format("Your public key is: {}", tools::bin2hex_string(public_key)));
            } catch (const std::exception& e) {
                logger::exit_func(e.what());
            }
        }

        #pragma endregion


        #pragma region secret
        [[nodiscard]] std::array<unsigned char, proto::E2E_KEYS_RAW_L> get_shared_secret(
            const std::array<unsigned char, proto::E2E_KEYS_RAW_L> &recipient_key
        ) const {
            // Try to find shared secret in cache
            const auto it = identity_detail.shared_secrets_cache.find(recipient_key);
            if (it != identity_detail.shared_secrets_cache.end()) return it->second;

            // Generate shared secret
            std::array<unsigned char, proto::E2E_KEYS_RAW_L> shared_secret; // NOLINT(*-pro-type-member-init)
            if (crypto_box_beforenm(
                shared_secret.data(),
                recipient_key.data(),
                private_key.data()
            ) != 0) throw std::runtime_error("Failed to create shared secret!");

            // Add it to cache
            identity_detail.shared_secrets_cache[recipient_key] = shared_secret;

            return shared_secret;
        }
        #pragma endregion


        #pragma region messages
        [[nodiscard]] auto encode_message(
            const std::array<unsigned char, proto::E2E_KEYS_RAW_L> &recipient_key,
            const std::vector<unsigned char> &message
        ) const {
            std::vector<unsigned char> encrypted_message;
            // [Nonce][MAC][Message]
            encrypted_message.resize(crypto_box_NONCEBYTES + std::size(message) + crypto_box_MACBYTES);

            // Generating nonce in encrypted_message vector
            randombytes_buf(encrypted_message.data(), crypto_box_NONCEBYTES);

            if (crypto_box_easy_afternm(
                encrypted_message.data() + crypto_box_NONCEBYTES, // Setting pointer after nonce
                message.data(), // Pointer to the raw message
                std::size(message), // Size of the raw message
                encrypted_message.data(), // Pointer to the nonce
                get_shared_secret(recipient_key).data() // Shared secret
            ) != 0) throw std::runtime_error("Failed to encode message!");

            return encrypted_message;
        }

        [[nodiscard]] auto decode_message(
            const std::array<unsigned char, proto::E2E_KEYS_RAW_L> &recipient_key,
            // [Nonce][MAC][Message]
            const std::vector<unsigned char>& encrypted_message
        ) const {
            std::vector<unsigned char> decoded_message;

            decoded_message.resize(std::size(encrypted_message) - crypto_box_NONCEBYTES - crypto_box_MACBYTES);

            if (crypto_box_open_easy_afternm(
                decoded_message.data(), // Pointer to the decoded message
                encrypted_message.data() + crypto_box_NONCEBYTES, // Pointer to the encoded message portion
                std::size(encrypted_message) - crypto_box_NONCEBYTES, // Size of encoded message portion
                encrypted_message.data(), // Pointer to the nonce
                get_shared_secret(recipient_key).data() // Shared secret
            ) != 0) throw std::runtime_error("Failed to decode message!");

            return decoded_message;
        }
        #pragma endregion
    };
}

#endif //MYDAK_BACKEND_KEYPAIR_H
