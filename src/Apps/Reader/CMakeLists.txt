set(SUBAPP_NAME  app_reader)

aux_source_directory(./                      SRCS)

add_library(${SUBAPP_NAME} STATIC ${SRCS})

target_include_directories(
    ${SUBAPP_NAME}
    PRIVATE ./
)

target_compile_options(
    ${SUBAPP_NAME} PRIVATE
    -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -marm -O3
    -foptimize-sibling-calls
) 
