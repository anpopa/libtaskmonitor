cmake_minimum_required(VERSION 3.7.2)
project(libtaskmonitor
        LANGUAGES CXX
        VERSION 1.0.7
        DESCRIPTION "TaskMonitor interface library"
)

set(PROJECT_HOMEPAGE_URL "https://gitlab.com/taskmonitor/libtaskmonitor")

# config
set(TARGET_NAME "tkm")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WITH_TIDY "Build with clang-tidy" N)
option(WITH_GCC_HARDEN_FLAGS "Build with GCC harden compile flags" N)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
        "Choose the type of build: Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set(GIT_SHA1 "local")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

# include
include(GNUInstallDirs)

# packing
include(cmake/Packing.cmake)

# dependencies
find_package(Protobuf REQUIRED)

# generate
configure_file(
    ${CMAKE_SOURCE_DIR}/libtaskmonitor.pc.in
    ${CMAKE_BINARY_DIR}/libtaskmonitor.pc @ONLY)

configure_file(
    ${CMAKE_SOURCE_DIR}/include/Helpers.h.in
    ${CMAKE_BINARY_DIR}/include/Helpers.h @ONLY
)

# protobuf
protobuf_generate_cpp(COLLECTOR_SRCS COLLECTOR_HDRS proto/Collector.proto)
protobuf_generate_cpp(MONITOR_SRCS MONITOR_HDRS proto/Monitor.proto)
protobuf_generate_cpp(ENVELOPE_SRCS ENVELOPE_HDRS proto/Envelope.proto)
protobuf_generate_cpp(CONTROL_SRCS CONTROL_HDRS proto/Control.proto)

if(WITH_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

if(WITH_GCC_HARDEN_FLAGS)
    # Build flags
    add_compile_options (
        -Wall
        -Wextra
        -Wno-unused-function
        -Wformat
        -Wno-variadic-macros
        -Wno-strict-aliasing
        -fstack-protector-strong
        -fwrapv
        -Wformat-signedness
        -Wmissing-include-dirs
        -Wimplicit-fallthrough=5
        -Wunused-parameter
        -Wuninitialized
        -Walloca
        -Wduplicated-branches
        -Wduplicated-cond
        -Wfloat-equal
        -Wshadow
        -Wcast-qual
        -Wconversion
        -Wlogical-op
        -Werror
        -Wformat-security
        -Walloc-zero
        -Wcast-align
        -Wredundant-decls
    )
endif()

# library
add_library(${TARGET_NAME}_object OBJECT
    source/EnvelopeReader.cpp
    source/EnvelopeWriter.cpp
    source/Helpers.cpp
    ${COLLECTOR_SRCS}
    ${MONITOR_SRCS}
    ${CONTROL_SRCS}
    ${ENVELOPE_SRCS}
    ${COLLECTOR_HDRS}
    ${MONITOR_HDRS}
    ${CONTROL_HDRS}
    ${ENVELOPE_HDRS}
)
add_library(${TARGET_NAME} SHARED $<TARGET_OBJECTS:${TARGET_NAME}_object>)
add_library(${TARGET_NAME}_static STATIC $<TARGET_OBJECTS:${TARGET_NAME}_object>)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROTOBUF_LIBRARY}>
)

target_link_libraries(${TARGET_NAME}_static
    PRIVATE
        $<BUILD_INTERFACE:${PROTOBUF_LIBRARY}>
)

set_target_properties(${TARGET_NAME}_object PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(${TARGET_NAME}_static PROPERTIES VERSION "${PROJECT_VERSION}")
set_target_properties(${TARGET_NAME}_static PROPERTIES OUTPUT_NAME ${TARGET_NAME})
set_target_properties(${TARGET_NAME} PROPERTIES VERSION "${PROJECT_VERSION}")
set_target_properties(${TARGET_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

set(LIBTM_PUBLIC_HEADERS
    "${CMAKE_SOURCE_DIR}/include/IAsyncEnvelope.h"
    "${CMAKE_SOURCE_DIR}/include/EnvelopeReader.h"
    "${CMAKE_SOURCE_DIR}/include/EnvelopeWriter.h"
    "${CMAKE_SOURCE_DIR}/include/taskmonitor.h"
    "${CMAKE_BINARY_DIR}/include/Helpers.h"
    "${CMAKE_BINARY_DIR}/Collector.pb.h"
    "${CMAKE_BINARY_DIR}/Control.pb.h"
    "${CMAKE_BINARY_DIR}/Envelope.pb.h"
    "${CMAKE_BINARY_DIR}/Monitor.pb.h"
)
set_target_properties(${TARGET_NAME} PROPERTIES PUBLIC_HEADER "${LIBTM_PUBLIC_HEADERS}")

target_include_directories(${TARGET_NAME}_object
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${Protobuf_INCLUDE_DIRS}
)

# install
install(FILES ${CMAKE_BINARY_DIR}/libtaskmonitor.pc 
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig 
        COMPONENT dev
)

install(TARGETS ${TARGET_NAME}
    EXPORT tkm-config
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT bin
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/taskmonitor 
    COMPONENT dev
)

install(TARGETS ${TARGET_NAME}_static
    EXPORT tkm-config
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT dev
)

install(EXPORT tkm-config
        NAMESPACE tkm::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tkm
        COMPONENT dev
)

# Status reporting
message (STATUS "CMAKE_BUILD_TYPE: "        ${CMAKE_BUILD_TYPE})
message (STATUS "WITH_TIDY: "               ${WITH_TIDY})
message (STATUS "WITH_GCC_HARDEN_FLAGS: "   ${WITH_GCC_HARDEN_FLAGS})
