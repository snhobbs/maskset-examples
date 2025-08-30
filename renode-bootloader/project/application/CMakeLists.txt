cmake_minimum_required(VERSION 3.25)
project(nrf52_application C ASM)

# Directories
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(COMMON_DIR ${ROOT_DIR}/common)
set(BOOTLOADER_DIR ${ROOT_DIR}/bootloader)
set(APPLICATION_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(EXTERNAL_DIR ${ROOT_DIR}/external)
set(BUILD_DIR ${APPLICATION_DIR}/build)

# Cortex-M4 flags
set(MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -g3
    -Os
    -fdebug-prefix-map=${ROOT_DIR}=.
    -ffunction-sections
    -fdata-sections
    -Wall
    -Werror)

# Include paths
include_directories(
  ${BOOTLOADER_DIR}/include ${COMMON_DIR}/include
  ${EXTERNAL_DIR}/mcuboot/boot/bootutil/include
  ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/include)

# Source files
set(APPLICATION_SRC
    ${APPLICATION_DIR}/src/log_port.c
    ${APPLICATION_DIR}/src/main.c
    ${APPLICATION_DIR}/src/minimal_nrf52_uart.c
    ${APPLICATION_DIR}/src/minimal_nrf52_flash.c
    ${APPLICATION_DIR}/src/shell.c
    ${APPLICATION_DIR}/src/shell_commands.c
    ${APPLICATION_DIR}/src/shell_port.c
    ${APPLICATION_DIR}/src/startup.c)

set(MCUBOOT_SRC ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/bootutil_misc.c
                ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/bootutil_public.c)

set(COMMON_SRC ${COMMON_DIR}/src/mcuboot_port.c)

set(ALL_SRC ${APPLICATION_SRC} ${MCUBOOT_SRC} ${COMMON_SRC})

# Include paths
include_directories(${APPLICATION_DIR}/include ${COMMON_DIR}/include
                    ${MCUBOOT_DIR}/include ${ROOT_DIR}/bootloader/config)
# Add executable
add_executable(${PROJECT_NAME}.elf ${ALL_SRC})

# Compile flags
target_compile_options(${PROJECT_NAME}.elf PRIVATE ${MCU_FLAGS})

# Linker flags
set(LDSCRIPT "${APPLICATION_DIR}/nrf52-app.ld")

target_link_options(
  ${PROJECT_NAME}.elf
  PRIVATE
  -T${LDSCRIPT}
  -Wl,-Map=${BUILD_DIR}/nrf52-bootloader.map
  -Wl,--gc-sections
  --specs=nano.specs
  -lc
  -lgcc
  -lc)

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE MCUBOOT_HAVE_LOG=0
                                                       CONFIG_MCUBOOT=0)

# Output directories
set_target_properties(${PROJECT_NAME}.elf PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                     ${BUILD_DIR})

# Custom command to create binary
add_custom_command(
  OUTPUT ${BUILD_DIR}/nrf52-app-no-header.bin
  COMMAND ${CMAKE_OBJCOPY} -O binary ${BUILD_DIR}/nrf52_application.elf
          ${BUILD_DIR}/nrf52-app-no-header.bin
  DEPENDS ${PROJECT_NAME}.elf)

# Custom command to sign MCUboot image
set(IMGTOOL_PY ${ROOT_DIR}/external/mcuboot/scripts/imgtool.py)
add_custom_command(
  OUTPUT ${BUILD_DIR}/nrf52-app.bin
  COMMAND
    python3 ${IMGTOOL_PY} sign --header-size 0x200 --align 8 -S 131072 -v 1.0.0
    --pad-header ${BUILD_DIR}/nrf52-app-no-header.bin ${BUILD_DIR}/nrf52-app.bin
  DEPENDS ${BUILD_DIR}/nrf52-app-no-header.bin)

# Add a pseudo-target for 'all'
add_custom_target(all_images ALL DEPENDS ${BUILD_DIR}/nrf52-app.bin)

# Optional: Flash command
add_custom_target(
  flash
  COMMAND
    arm-none-eabi-gdb-py --eval-command="target remote localhost:2331"
    --ex="mon reset" --ex="restore ${BUILD_DIR}/nrf52-app.bin binary 0x8000"
    --ex="mon reset" --se=${BUILD_DIR}/nrf52_application.elf
  DEPENDS all_images)
