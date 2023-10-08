
set(APP_NAME  gameboy)
 
include_directories(.)
include_directories(./Corelib)
include_directories(./Fs)
include_directories(./Graphic)
include_directories(./Graphic/UI)
include_directories(./Keyboard)
include_directories(./Time)
include_directories(./Utils)
include_directories(./Utils/Collectc)
include_directories(./Apps)
aux_source_directory(. ${APP_NAME}_SRCS)
aux_source_directory(./Corelib ${APP_NAME}_SRCS)
aux_source_directory(./Fs ${APP_NAME}_SRCS)
aux_source_directory(./Graphic ${APP_NAME}_SRCS)
aux_source_directory(./Graphic/UI ${APP_NAME}_SRCS)
aux_source_directory(./Keyboard ${APP_NAME}_SRCS)
aux_source_directory(./Time ${APP_NAME}_SRCS)
aux_source_directory(./Utils ${APP_NAME}_SRCS)
aux_source_directory(./Utils/Collectc ${APP_NAME}_SRCS)
aux_source_directory(./Apps ${APP_NAME}_SRCS)

add_executable(${APP_NAME}.elf ${${APP_NAME}_SRCS})
target_compile_options(
  ${APP_NAME}.elf PRIVATE
  -mtune=arm926ej-s -mcpu=arm926ej-s -mlittle-endian -mfloat-abi=soft -marm -O3
  -foptimize-sibling-calls
) 

# ==== add apps here ====
add_subdirectory(./Apps/Gameboy)
# ==== add apps end ====

target_link_libraries(${APP_NAME}.elf PRIVATE
  -lnosys -lc -lm -lgcc -lstdc++ -nostartfiles
  # ==== add apps here ====
  app_gb
  # ==== add apps end ====
  -T ${CMAKE_SOURCE_DIR}/script/ld.lds
)

# Connamds
add_custom_command(TARGET ${APP_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_SIZE} ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.elf 
)

add_custom_command(OUTPUT upload_${APP_NAME}
  COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}/script/eosctl.py upload -n "${APP_NAME}.exp" "${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.exp"
  DEPENDS ${APP_NAME}.exp
)

add_custom_command(OUTPUT exec_${APP_NAME}
  COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}/script/eosctl.py exec "${APP_NAME}.exp"
  DEPENDS upload_${APP_NAME}
)

# Targets
add_custom_target(
  ${APP_NAME}.exp ALL
  COMMAND ${CMAKE_OBJCOPY} -I elf32-little -O binary -R .note -R .comment -S ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.elf ${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.exp 
  DEPENDS ${APP_NAME}.elf 
)

add_custom_target(
  run
  COMMAND COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}/script/eosctl.py monitor
  DEPENDS exec_${APP_NAME}
)

add_custom_target(
  upload
  DEPENDS upload_${APP_NAME}
)
