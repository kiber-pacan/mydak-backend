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
    struct detail {
        // public key - shared secret
        mutable std::map<std::array<unsigned char, proto::E2E_KEYS_RAW_L>, std::array<unsigned char, proto::E2E_KEYS_RAW_L>> shared_secrets_cache;
    };
    struct identity {
        #pragma region variables
        std::array<unsigned char, proto::E2E_KEYS_RAW_L> public_key{};
        std::array<unsigned char, proto::E2E_KEYS_RAW_L> private_key{};
        std::string public_hex;
        std::uint16_t public_value;

        std::array<unsigned char, crypto_pwhash_SALTBYTES> salt;
        std::array<unsigned char, 32> password_hash;

        std::array<unsigned char, crypto_secretbox_NONCEBYTES> nonce;
        std::array<unsigned char, 48> private_key_encoded;

        std::string_view password_view;
        std::string filename;

        detail detail;
        #pragma endregion


        #pragma region initialization
        void initialize_credentials() {
            if (crypto_box_keypair(public_key.data(), private_key.data()) != 0)
                throw std::runtime_error("Failed to generate key!");


            randombytes_buf(salt.data(), std::size(salt));
            randombytes_buf(nonce.data(), std::size(nonce));

            if (crypto_pwhash(
                password_hash.data(),
                std::size(password_hash),
                password_view.data(),
                password_view.size(),
                salt.data(),
                2,
                134217728,
                crypto_pwhash_ALG_DEFAULT
            ) != 0) throw std::runtime_error("Failed to create password hash!");

            if (crypto_secretbox_easy(
                private_key_encoded.data(),
                private_key.data(),
                32,
                nonce.data(),
                password_hash.data()
            ) != 0) throw std::runtime_error("Failed to encode private key!");

            std::memcpy(&public_value, public_key.data(), sizeof(decltype(public_value)));

            filename = std::format("{}.toml", public_hex);
            logger::log_debug(std::format("Initialized credentials with public key {}", public_hex));
        }

        void initialize(const std::string_view public_key_hex, const std::string_view password) {
            this->password_view = password;
            this->public_hex = public_key_hex;

            if (std::size(public_key_hex) > 0) {
                if (filename = std::format("{}.toml", std::string_view(reinterpret_cast<const char *>(public_key_hex.data()), std::size(public_key_hex))); std::filesystem::exists(filename)) {

                    this->public_hex = public_key_hex;

                    load_keypair();
                    std::memcpy(&public_value, public_key.data(), sizeof(decltype(public_value)));
                    return;
                }
            }

            initialize_credentials();
            save_keypair();
        }
        #pragma endregion


        #pragma region save-load
        void save_keypair() {
            try {
                toml::table keypair_file;

                keypair_file.insert_or_assign("salt", tools::bin2hex_string(salt));
                keypair_file.insert_or_assign("nonce", tools::bin2hex_string(nonce));
                keypair_file.insert_or_assign("opslimit", 2);
                keypair_file.insert_or_assign("memlimit", 134217728);
                keypair_file.insert_or_assign("password_hash", tools::bin2hex_string(password_hash));

                keypair_file.insert_or_assign("public_key", public_hex);
                keypair_file.insert_or_assign("private_key_encoded", tools::bin2hex_string(private_key_encoded));

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
                toml::table keypair_file = toml::parse_file(filename);
                std::cout <<"load" << std::endl;
                try_load_value(salt, "salt");
                try_load_value(salt, "salt");

                std::size_t opslimit;
                try_load_value(opslimit, "opslimit");
                std::size_t memlimit;
                try_load_value(memlimit, "memlimit");
                try_load_value(password_hash, "password_hash");

                try_load_value(nonce, "nonce");
                try_load_value(public_key, "public_key");
                try_load_value(private_key_encoded, "private_key_encoded");


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

                if (crypto_secretbox_open_easy(
                    private_key.data(),
                    private_key_encoded.data(),
                    std::size(private_key_encoded),
                    nonce.data(),
                    password_hash.data()
                ) != 0) throw std::runtime_error("Failed to encode private key!");


                logger::log_debug("Loaded credentials from file");
            } catch (const std::exception& e) {
                logger::exit_func(e.what());
            }
        }

        #pragma endregion


        #pragma region secret
        [[nodiscard]] std::array<unsigned char, proto::E2E_KEYS_RAW_L> get_shared_secret(
            std::array<unsigned char, proto::E2E_KEYS_RAW_L> &recipient_key
        ) {
            std::cout << "get_shared_secret start" << std::endl;
            const auto it = detail.shared_secrets_cache.find(recipient_key);
            if (it != detail.shared_secrets_cache.end()) return it->second;

            std::array<unsigned char, proto::E2E_KEYS_RAW_L> shared_secret; // NOLINT(*-pro-type-member-init)
            if (crypto_box_beforenm(
                shared_secret.data(),
                recipient_key.data(),
                private_key.data()
            ) != 0) throw std::runtime_error("Failed to create shared secret!");

            detail.shared_secrets_cache[recipient_key] = shared_secret;
            std::cout << "get_shared_secret end" << std::endl;

            return shared_secret;
        }
        #pragma endregion


        #pragma region messages
        [[nodiscard]] auto encode_message(
            std::array<unsigned char, proto::E2E_KEYS_RAW_L> &recipient_key,
            std::vector<unsigned char> message
        ) {
            try {
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

                for (int i = 0; i < std::size(encrypted_message); ++i) {
                    std::cout << i << " " << encrypted_message[i] << std::endl;
                }

                return encrypted_message;
            } catch (const std::exception& e) {
                logger::exit_func(e.what());
            }
        }

        [[nodiscard]] auto decode_message(
            std::array<unsigned char, proto::E2E_KEYS_RAW_L> &recipient_key,
            // [Nonce][MAC][Message]
            const std::vector<unsigned char>& encrypted_message
        ) {
            try {
                std::cout << "decoding start" << std::endl;
                std::vector<unsigned char> decoded_message;

                decoded_message.resize(std::size(encrypted_message) - crypto_box_NONCEBYTES - crypto_box_MACBYTES);

                for (int i = 0; i < std::size(encrypted_message); ++i) {
                    std::cout << i << " " << encrypted_message[i] << std::endl;
                }

                if (crypto_box_open_easy_afternm(
                    decoded_message.data(), // Pointer to the decoded message
                    encrypted_message.data() + crypto_box_NONCEBYTES, // Pointer to the encoded message portion
                    std::size(encrypted_message) - crypto_box_NONCEBYTES, // Size of encoded message portion
                    encrypted_message.data(), // Pointer to the nonce
                    get_shared_secret(recipient_key).data() // Shared secret
                ) != 0) throw std::runtime_error("Failed to decode message!");

                std::cerr << "decoding end" << std::endl;

                return decoded_message;
            } catch (const std::exception& e) {
                logger::exit_func(e.what());
            }
        }
        #pragma endregion
    };
}

#endif //MYDAK_BACKEND_KEYPAIR_H
