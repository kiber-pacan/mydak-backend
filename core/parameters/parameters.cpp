//
// Created by akicatt on 01.08.2026.
//

#include <unordered_map>
#include <boost/asio/ip/address.hpp>
#include <boost/core/demangle.hpp>

#include "parameters.hpp"



// Output order is undefined
void mydak::args::help() {
    //std::array<std::string_view, parameters_count> array{};
    // Getting string views pointed to the corresponding options from options_tuple
    tools::constexpr_for<parameters_count>(
    [&](auto i) {
        parameters[i].visit([&](auto&& parameter) {
            logger::log(std::format("{} : {} ({}), default value: {}.", std::get<i>(options_tuple).c_str(), parameter.limits_to_string(), boost::core::demangle(typeid(decltype(parameter.get_data())).name()), parameter.get_data()));
        });
    });


    for (std::size_t i = 0; i < parameters_count; i++) {

    }

    std::exit(1);
}
