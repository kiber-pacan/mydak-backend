//
// Created by akicatt on 01.08.2026.
//
#include "parameters.hpp"

namespace parameters {
    inline static constexpr uint8_t max_uint8_t = std::numeric_limits<uint8_t>::max();
    inline static constexpr int8_t max_int8_t = std::numeric_limits<int8_t>::max();

    static inline const std::unordered_map<std::string, size_t> existing_arguments{
        {"--connect-tries", 0},
        {"--wait-time", 1},
        {"--wait-time-add", 2},
        {"--public-key", 3},
        {"--recipient", 4}
    };

    inline static const std::vector<mydak::args::parameter_variant> existing_parameters{
        mydak::args::parameter<0>(1, max_int8_t, 3),
        mydak::args::parameter<0>(0, max_int8_t, 2),
        mydak::args::parameter<0>(-1, max_int8_t, 2),
        mydak::args::parameter<1>(64, 64, ""),
        mydak::args::parameter<1>(64, 64, "")
    };
}

inline void help() {
    for (const auto& argument : parameters::existing_arguments) {
        const auto& variant = parameters::existing_parameters[argument.second];
        variant.visit([argument](auto&& parameter) {
            mydak::logger::log(std::format("{} : {}", argument.first, parameter.limits_to_string()));
        });
    }

    std::exit(1);
}

[[nodiscard]] inline  std::vector<mydak::args::parameter_variant> process_args(const int argc, char* argv[]) {
    std::vector<mydak::args::parameter_variant> values = parameters::existing_parameters;
    for (int i = 1; i < argc; i++) {
        std::string raw = argv[i];

        if (raw == "--help") help();

        const auto equals_pos = raw.find('=');
        if (equals_pos == std::string::npos) {
            mydak::logger::exception(std::format("Wrong parameter format: {}! (no equals symbol)", raw));
        }

        std::string parameter_string = raw.substr(0, equals_pos);
        auto value = std::string(raw.subview(equals_pos + 1, raw.size() - equals_pos - 1));

        if (value.empty()) {
            mydak::logger::exception(std::format("Empty value: {}!", parameter_string));
        }

        auto it = parameters::existing_arguments.find(parameter_string);

        if (it == parameters::existing_arguments.end()) {
            mydak::logger::exception(std::format("Wrong parameter: {}! seek help: --help.", parameter_string));
        }

        const size_t& index = it->second;
        auto& variant = values[index];
        variant.visit([value](auto&& parameter) {
            parameter.try_set_val(value);
        });
    }

    return values;
}