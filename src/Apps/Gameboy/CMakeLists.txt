
 
SET(SUBAPP_NAME  app_gb)

aux_source_directory(./ _SRCS)
aux_source_directory(./gb _SRCS)

add_library(${SUBAPP_NAME} STATIC ${_SRCS})

target_include_directories(
    ${SUBAPP_NAME}
    PRIVATE ./
    PRIVATE ./gb
)

target_compile_options(
    ${SUBAPP_NAME} PRIVATE
    -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -marm -Ofast
    -foptimize-sibling-calls
)

