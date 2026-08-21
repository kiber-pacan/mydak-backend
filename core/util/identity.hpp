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
#include "toml++/toml.hpp"

namespace mydak {
    struct identity {
        std::array<unsigned char, proto::E2E_KEYS_L / 2> public_key{};
        std::array<unsigned char, proto::E2E_KEYS_L / 2> private_key{};
        std::string public_hex;
        std::uint16_t public_value;

        std::array<unsigned char, crypto_pwhash_SALTBYTES> salt;
        std::array<unsigned char, 32> password_hash;

        std::array<unsigned char, crypto_secretbox_NONCEBYTES> nonce;
        std::array<unsigned char, 48> private_key_encoded;

        std::string_view password_view;
        std::string filename;

        template <typename T, std::size_t N>
        static std::string bin2hex(const std::array<T, N>& bin) {
            std::array<char, N * 2 + 1> hex; // NOLINT(*-pro-type-member-init)
            sodium_bin2hex(hex.data(), std::size(hex), bin.data(), std::size(bin));
            return {hex.data(), std::size(hex) - 1};
        }

        template <std::size_t N>
        static auto hex2bin(const std::string_view hex) {
            std::array<unsigned char, N> bin; // NOLINT(*-pro-type-member-init)
            sodium_hex2bin(bin.data(), std::size(bin), hex.data(), std::size(hex), nullptr, nullptr, nullptr);
            return bin;
        }

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
                reinterpret_cast<unsigned const char*>(password_view.data())
            ) != 0) throw std::runtime_error("Failed to encode private key!");

            public_hex = bin2hex(public_key);
            //private_hex = bin2hex(private_key);

            std::memcpy(&public_value, public_key.data(), sizeof(decltype(public_value)));

            filename = std::format("{}.toml", public_hex);
            logger::log_debug(std::format("Initialized credentials with public key {}", public_hex));
        }

        void initialize(const std::string_view public_key_input, const std::string_view password) {
            this->password_view = password;


            if (std::size(public_key_input) > 0) {
                if (filename = std::format("{}.toml", public_key_input); std::filesystem::exists(filename)) {
                    this->public_hex = public_key_input;
                    load_keypair();
                    std::memcpy(&public_value, public_key.data(), sizeof(decltype(public_value)));
                    return;
                }
            }

            initialize_credentials();
            save_keypair();
        }



        void save_keypair() {
            try {
                toml::table keypair_file;

                keypair_file.insert_or_assign("salt", bin2hex(salt));
                keypair_file.insert_or_assign("nonce", bin2hex(nonce));
                keypair_file.insert_or_assign("opslimit", 2);
                keypair_file.insert_or_assign("memlimit", 134217728);
                keypair_file.insert_or_assign("password_hash", bin2hex(password_hash));

                keypair_file.insert_or_assign("public_key", public_hex);
                keypair_file.insert_or_assign("private_key_encoded", bin2hex(private_key_encoded));

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
                value = hex2bin<N>(value_opt.value());
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

                if (crypto_secretbox_easy(
                    private_key_encoded.data(),
                    private_key.data(),
                    32,
                    nonce.data(),
                    password_hash.data()
                ) != 0) throw std::runtime_error("Failed to encode private key!");

                logger::log_debug("Loaded credentials from file");
            } catch (const std::exception& e) {
                logger::exit_func(e.what());
            }
        }
    };
}

#endif //MYDAK_BACKEND_KEYPAIR_H
