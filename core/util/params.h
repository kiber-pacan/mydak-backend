//
// Created by akicatt on 31.07.2026.
//

#ifndef MYDAK_BACKEND_PARAMS_H
#define MYDAK_BACKEND_PARAMS_H
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "logger.hpp"


namespace mydak::args {
    inline bool is_a_number(std::string_view string) {
        for (const char& character : string) if (!std::isdigit(character)) return false;
        return true;
    }

    template <uint8_t type>
        struct parameter {
    };

    template <>
    struct parameter<0> {
        static constexpr uint8_t type_val = 0;

        parameter(const int8_t&& min, const int8_t max, const int8_t default_val)
        : data(default_val), min(min), max(max) {}

        void try_set_val(const std::string& value) {
            if (is_a_number(value)) {
                const auto number = static_cast<int8_t>(std::stoi(value));
                if (is_in_limits(number)) {
                    data = number;
                } else {
                    mydak::logger::exception(std::format("{} is not in bounds: {}!", value, limits_to_string()));
                }
            } else {
                mydak::logger::exception(std::format("{} is not a number!", value));
            }
        }

        [[nodiscard]] std::string limits_to_string() const {
            return std::format("from {} to {}", min, max);
        }

        [[nodiscard]] std::string to_string() const {
            return std::to_string(data);
        }

        [[nodiscard]] int8_t get_data() const {
            return data;
        }


    private:
        [[nodiscard]] bool is_in_limits(const int8_t& num) const {
            return num >= min && num <= max;
        }

        int8_t data{};
        int8_t min{};
        int8_t max{};
    };

    template <>
    struct parameter<1> {
        static constexpr uint8_t type_val = 1;

        parameter(const int8_t&& min, const int8_t max, const std::string& default_value)
        : data(default_value), min(min), max(max) {}

        void try_set_val(const std::string& value) {
            if (is_in_limits(value)) {
                data = value;
            } else {
                mydak::logger::exception(std::format("{} is not in bounds: {}!", value, limits_to_string()));
            }
        }

        [[nodiscard]] std::string limits_to_string() const {
            return std::format("from {} to {}", min, max);
        }

        [[nodiscard]] std::string to_string() const {
            return data;
        }

        [[nodiscard]] std::string get_data() const {
            return data;
        }

    private:
        [[nodiscard]] bool is_in_limits(const std::string& string) const {
            return string.size() >= min && string.size() <= max;
        }

        std::string data{};
        int8_t min{};
        int8_t max{};
    };


    using parameter_variant = std::variant<mydak::args::parameter<0>, mydak::args::parameter<1>>;

    void help();

    [[nodiscard]] std::vector<mydak::args::parameter_variant> process_args(int argc, char* argv[]);
}

#endif //MYDAK_BACKEND_PARAMS_H
