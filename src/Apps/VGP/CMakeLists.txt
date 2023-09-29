set(SUBAPP_NAME  app_vgp)

aux_source_directory(./                      SRCS)
aux_source_directory(./vgp                   SRCS)
aux_source_directory(./wasm3                 SRCS)

add_library(${SUBAPP_NAME} STATIC ${SRCS})

target_include_directories(
    ${SUBAPP_NAME}
    PRIVATE ./
    PRIVATE ./vgp
    PRIVATE ./wasm3
)

target_compile_options(
    ${SUBAPP_NAME} PRIVATE
    -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -marm -O3
    -foptimize-sibling-calls
) 

target_link_libraries(${SUBAPP_NAME} PRIVATE
    -lm
)
