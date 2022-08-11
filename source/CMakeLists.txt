set(TARGET_NAME "taskmonitor")

include(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(COLLECTOR_SRCS COLLECTOR_HDRS ../proto/Collector.proto)
protobuf_generate_cpp(MONITOR_SRCS MONITOR_HDRS ../proto/Monitor.proto)
protobuf_generate_cpp(ENVELOPE_SRCS ENVELOPE_HDRS ../proto/Envelope.proto)
protobuf_generate_cpp(CONTROL_SRCS CONTROL_HDRS ../proto/Control.proto)

set(LIB_SRC 
  ${COLLECTOR_SRCS}
  ${MONITOR_SRCS}
  ${CONTROL_SRCS}
  ${ENVELOPE_SRCS})

set(LIB_HDR 
  ${COLLECTOR_HDRS}
  ${MONITOR_HDRS}
  ${CONTROL_HDRS}
  ${ENVELOPE_HDRS})

set(LIB_SRC
  ${LIB_SRC}
  EnvelopeReader.cpp
  EnvelopeWriter.cpp
  Helpers.cpp)

add_library(${TARGET_NAME} SHARED ${LIB_SRC} ${LIB_HDR})
target_link_libraries(${TARGET_NAME} PUBLIC ${PROTOBUF_LIBRARY})

set_target_properties(${TARGET_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${TARGET_NAME} PROPERTIES SOVERSION 1)

set(LIBTM_PUBLIC_HEADERS
    "${CMAKE_BINARY_DIR}/include/TaskMonitor.h"
    "${CMAKE_SOURCE_DIR}/include/IAsyncEnvelope.h"
    "${CMAKE_SOURCE_DIR}/include/EnvelopeReader.h"
    "${CMAKE_SOURCE_DIR}/include/EnvelopeWriter.h"
    "${CMAKE_SOURCE_DIR}/include/Helpers.h"
    "${CMAKE_BINARY_DIR}/source/Collector.pb.h"
    "${CMAKE_BINARY_DIR}/source/Control.pb.h"
    "${CMAKE_BINARY_DIR}/source/Envelope.pb.h"
    "${CMAKE_BINARY_DIR}/source/Monitor.pb.h"
    )
set_target_properties(${TARGET_NAME} PROPERTIES PUBLIC_HEADER "${LIBTM_PUBLIC_HEADERS}")

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/source)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR})
target_include_directories(${TARGET_NAME} PRIVATE ${Protobuf_INCLUDE_DIRS})

install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT bin
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/taskmonitor COMPONENT dev)
