# Set cmake version
cmake_minimum_required(VERSION 4.3)

find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE})
endif()

# Setting c++ 26 standard
set(CMAKE_CXX_STANDARD 26)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(EXECUTABLE_NAME mydak-backend)
set(EXECUTABLE_NAME_LOWER_DASH mydak_backend)

# Declaring project
project(${EXECUTABLE_NAME})
cmake_policy(SET CMP0167 OLD)

include(FetchContent)

find_package(Qt6 6.11.2 COMPONENTS Quick REQUIRED)

qt_standard_project_setup(REQUIRES 6.11.2)

qt_add_executable(${EXECUTABLE_NAME}
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


qt_add_qml_module(${EXECUTABLE_NAME}
        URI ${EXECUTABLE_NAME_LOWER_DASH}
        VERSION 1.0
        QML_FILES
        core/qml/widgets/Main.qml
        core/qml/widgets/MessagesRectangle.qml
        core/qml/widgets/RecipientRectangle.qml
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

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        sodium
        tomlplusplus::tomlplusplus
        brotlienc
        brotlidec
        brotlicommon
        OpenSSL::Crypto
        OpenSSL::SSL
        Boost::headers
        Qt6::Quick
)