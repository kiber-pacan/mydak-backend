# Set cmake version
cmake_minimum_required(VERSION 4.3)

# Setting c++ 26 standard
set(CMAKE_CXX_STANDARD 26)

# Declaring project
project(mydak-backend)
cmake_policy(SET CMP0167 OLD)

include(FetchContent)

set(EXECUTABLE_NAME ${PROJECT_NAME})

add_executable(${EXECUTABLE_NAME}
        core/main.cpp

        core/client.hpp
        core/client.cpp

        core/proto.hpp

        core/util/logger.cpp
        core/util/logger.hpp

        core/util/brotli.cpp
        core/util/brotli.hpp
        core/util/parameters.hpp
        core/util/parameters.cpp
        core/util/tools.hpp
        core/util/namer.hpp
        core/util/namer.hpp
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/core/
        ${CMAKE_CURRENT_SOURCE_DIR}/core/util
)


# Adding sources to project
target_sources(${EXECUTABLE_NAME} PRIVATE ${include})


# Dependencies
include(CPM.cmake)

# LIBSODIUM
CPMAddPackage(
        NAME libsodium_cmake
        GIT_REPOSITORY "https://github.com/robinlinden/libsodium-cmake.git"
        GIT_TAG "cfebfd3da486d5a86c644c8b47067e5411c7599c"
)

# TOML++
CPMAddPackage(
        NAME tomlplusplus_cmake
        GIT_REPOSITORY "https://github.com/marzer/tomlplusplus.git"
        GIT_TAG "v3.4.0"
)

# BROTLI
CPMAddPackage(
        NAME brotli_cmake
        GIT_REPOSITORY "https://github.com/google/brotli"
        GIT_TAG "master"
)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC
        sodium
        tomlplusplus::tomlplusplus
        brotlienc
        brotlidec
        brotlicommon
        OpenSSL::Crypto
        OpenSSL::SSL
        Boost::headers
)

target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)
