//
// Created by akicatt on 31.07.2026.
//

#ifndef MYDAK_BACKEND_BROTLI_H
#define MYDAK_BACKEND_BROTLI_H


#include <brotli/encode.h>
#include <brotli/decode.h>
#include <string>
#include <sstream>
#include <array>

#ifndef BROTLI_BUFFER_SIZE
#define BROTLI_BUFFER_SIZE 1024
#endif

namespace mydak::brotli {
    static std::string compress(const std::string& data);

    inline std::string decompress(const std::string& data);
}


#endif //MYDAK_BACKEND_BROTLI_H
