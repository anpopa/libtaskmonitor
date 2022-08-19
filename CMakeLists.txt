cmake_minimum_required(VERSION 3.7.2)
project(libtaskmonitor DESCRIPTION "TaskMonitor interface library")

set(CMAKE_PROJECT_HOMEPAGE_URL "https://gitlab.com/taskmonitor/libtaskmonitor")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
message (STATUS "Module paths: ${CMAKE_MODULE_PATH}")

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set(GIT_SHA1 "local")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
        "Choose the type of build: Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif()

set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "0")

include(GNUInstallDirs)
include(Packing)

configure_file(
    ${CMAKE_SOURCE_DIR}/config/libtaskmonitor.pc.in
    ${CMAKE_BINARY_DIR}/config/libtaskmonitor.pc @ONLY)

configure_file(
    ${CMAKE_SOURCE_DIR}/include/TaskMonitor.h.in
    ${CMAKE_BINARY_DIR}/include/TaskMonitor.h @ONLY)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

# Build
add_subdirectory(config)
add_subdirectory(source)
