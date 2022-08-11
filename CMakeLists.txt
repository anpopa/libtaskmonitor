# CMake general setup
cmake_minimum_required(VERSION 3.7.2)
project(taskmonitor VERSION 1.0.0 DESCRIPTION "TaskMonitor interface library")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Build time configuration setup
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set(GIT_SHA1 "local")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

include(GNUInstallDirs)

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
