//
// Created by akicatt on 31.07.2026.
//

#ifndef MYDAK_BACKEND_BROTLI_H
#define MYDAK_BACKEND_BROTLI_H


#include <string>
#include <vector>

#ifndef BROTLI_BUFFER_SIZE
#define BROTLI_BUFFER_SIZE 1024
#endif

namespace mydak::brotli {
    std::vector<unsigned char> compress(const std::string_view string);

    std::vector<unsigned char> decompress(const std::vector<unsigned char>& text);
}


#endif //MYDAK_BACKEND_BROTLI_H
