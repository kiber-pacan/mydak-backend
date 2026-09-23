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
        core/util/coh.hpp
        core/util/identity.hpp
        core/util/identity.hpp

        core/Qt/util/message_model.hpp
        core/Qt/util/message_model.cpp

        core/Qt/connector/qt_connector.hpp
        core/Qt/connector/qt_connector.cpp
        core/Qt/connector/qt_ptrs.hpp
        core/main/dialog.cpp
        core/main/dialog.hpp
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/core/main
        ${CMAKE_CURRENT_SOURCE_DIR}/core/util
        ${CMAKE_CURRENT_SOURCE_DIR}/core/parameters

        ${CMAKE_CURRENT_SOURCE_DIR}/core/Qt
        ${CMAKE_CURRENT_SOURCE_DIR}/core/Qt/connector
        ${CMAKE_CURRENT_SOURCE_DIR}/core/Qt/util
)


qt_add_qml_module(${EXECUTABLE_NAME}
        URI ${EXECUTABLE_NAME_LOWER_DASH}
        VERSION 1.0

        SOURCES
        core/Qt/util/message_model.hpp
        core/Qt/util/message_model.cpp

        QML_FILES
        core/Qt/QtQuick/Main.qml
        core/Qt/QtQuick/DialogRectangle.qml
        core/Qt/QtQuick/Theme.qml

        core/Qt/QtQuick/bars/top/RecipientBar_Top.qml
        core/Qt/QtQuick/bars/top/UserBar_Top.qml
        core/Qt/QtQuick/bars/bottom/InputBar_Bottom.qml
        core/Qt/QtQuick/bars/bottom/LogoBar_Bottom.qml
)

qt_add_resources(${EXECUTABLE_NAME} "fonts"
        FILES
        core/Qt/fonts/fantasque_sans_mono/ttf/regular.ttf
        core/Qt/fonts/fantasque_sans_mono/ttf/bold.ttf
        core/Qt/fonts/fantasque_sans_mono/ttf/bold_italic.ttf
        core/Qt/fonts/fantasque_sans_mono/ttf/italic.ttf
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