//
// Created by akicatt on 31.07.2026.
//

#include "brotli.hpp"

#include <brotli/encode.h>
#include <brotli/decode.h>
#include <string>
#include <sstream>
#include <array>
#include <iostream>
#include <iterator>
#include <vector>

#ifndef BROTLI_BUFFER_SIZE
#define BROTLI_BUFFER_SIZE 1024
#endif

std::vector<unsigned char> mydak::brotli::compress(std::string_view string) {
    auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
    std::array<uint8_t, BROTLI_BUFFER_SIZE> buffer{};
    std::stringstream result;

    size_t available_in = string.length(), available_out = buffer.size();
    const auto* next_in = reinterpret_cast<const uint8_t*>(string.data());
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
    return std::vector<unsigned char>(
        std::istreambuf_iterator<char>(result),
        std::istreambuf_iterator<char>()
    );
}

std::vector<unsigned char> mydak::brotli::decompress(const std::vector<unsigned char>& text) {
    std::cout << "decompressing text" << std::endl;
    auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    std::array<uint8_t, BROTLI_BUFFER_SIZE> buffer{};
    std::stringstream result;

    size_t available_in = std::size(text), available_out = buffer.size();
    const auto* next_in = text.data();
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
    return {
        std::istreambuf_iterator<char>(result),
        std::istreambuf_iterator<char>()
    };
}
