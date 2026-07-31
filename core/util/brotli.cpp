//
// Created by akicatt on 31.07.2026.
//

#include "brotli.hpp"

#include <brotli/encode.h>
#include <brotli/decode.h>
#include <string>
#include <sstream>
#include <array>

#ifndef BROTLI_BUFFER_SIZE
#define BROTLI_BUFFER_SIZE 1024
#endif

static std::string mydak::brotli::compress(const std::string& data) {
    auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
    std::array<uint8_t, BROTLI_BUFFER_SIZE> buffer{};
    std::stringstream result;

    size_t available_in = data.length(), available_out = buffer.size();
    const auto* next_in = reinterpret_cast<const uint8_t*>(data.c_str());
    uint8_t* next_out = buffer.data();

    while (!(available_in == 0 && BrotliEncoderIsFinished(instance))) {
        BrotliEncoderCompressStream
        (
            instance, BROTLI_OPERATION_FINISH,
            &available_in, &next_in, &available_out, &next_out, nullptr
        );
        result.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() - available_out);
        available_out = buffer.size();
        next_out = buffer.data();
    }


    BrotliEncoderDestroyInstance(instance);
    return result.str();
}

inline std::string mydak::brotli::decompress(const std::string& data) {
    auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    std::array<uint8_t, BROTLI_BUFFER_SIZE> buffer{};
    std::stringstream result;

    size_t available_in = data.length(), available_out = buffer.size();
    const auto* next_in = reinterpret_cast<const uint8_t*>(data.c_str());
    uint8_t* next_out = buffer.data();
    BrotliDecoderResult oneshot_result{};

    while (!(available_in == 0 && oneshot_result == BROTLI_DECODER_RESULT_SUCCESS)) {
        oneshot_result = BrotliDecoderDecompressStream
        (
            instance,
            &available_in, &next_in, &available_out, &next_out, nullptr
        );
        result.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() - available_out);
        available_out = buffer.size();
        next_out = buffer.data();
    }


    BrotliDecoderDestroyInstance(instance);
    return result.str();
}
