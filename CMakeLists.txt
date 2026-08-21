# Set cmake version
cmake_minimum_required(VERSION 4.3)

find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE})
endif()

# Setting c++ 26 standard
set(CMAKE_CXX_STANDARD 26)

# Declaring project
project(mydak-backend)
cmake_policy(SET CMP0167 OLD)

include(FetchContent)

set(EXECUTABLE_NAME ${PROJECT_NAME})

add_executable(${EXECUTABLE_NAME}
        core/main.cpp

        core/main/client.hpp
        core/main/client.cpp

        core/util/proto.hpp

        core/util/logger.cpp
        core/util/logger.hpp

        core/util/brotli.cpp
        core/util/brotli.hpp

        core/util/tools.hpp

        core/util/namer.hpp

        core/parameters/parameters.hpp
        core/parameters/parameters.cpp

        core/parameters/parameters_base.hpp
        core/parameters/parameters_base.cpp

        core/parameters/parameters_accessor.hpp
        core/parameters/parameters_accessor.cpp
        core/main/local_server.cpp
        core/main/local_server.hpp
        core/util/coh.cpp
        core/util/coh.hpp
        core/util/identity.hpp
        core/util/identity.hpp

)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/core/main
        ${CMAKE_CURRENT_SOURCE_DIR}/core/util
        ${CMAKE_CURRENT_SOURCE_DIR}/core/parameters
)


# Adding sources to project
target_sources(${EXECUTABLE_NAME} PRIVATE ${include})


include(cmake/CPM.cmake)


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
