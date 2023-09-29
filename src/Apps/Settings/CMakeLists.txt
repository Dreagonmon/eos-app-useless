set(SUBAPP_NAME  app_settings)

aux_source_directory(./                      SRCS)

add_library(${SUBAPP_NAME} STATIC ${SRCS})

target_include_directories(
    ${SUBAPP_NAME}
    PRIVATE ./
)

# build time in UTC ISO 8601
include_directories(${CMAKE_CURRENT_BINARY_DIR})
file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/timestamp.cmake "string(TIMESTAMP TIMEZ UTC)\n")
file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/timestamp.cmake "file(WRITE build_timestamp.h \"#ifndef BUILDTIMESTAMP_H\\n\")\n")
file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/timestamp.cmake "file(APPEND build_timestamp.h \"#define BUILDTIMESTAMP_H\\n\\n\")\n")
file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/timestamp.cmake "file(APPEND build_timestamp.h \"#define _BUILD_TIME_ \\\"\${TIMEZ} UTC\\\"\\n\\n\")\n")
file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/timestamp.cmake "file(APPEND build_timestamp.h \"#endif // BUILDTIMESTAMP_H\\n\")\n")

add_custom_target (
    timestamp
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/timestamp.cmake
    ADD_DEPENDENCIES ${CMAKE_CURRENT_BINARY_DIR}/timestamp.cmake
)

add_dependencies(${SUBAPP_NAME} timestamp)
