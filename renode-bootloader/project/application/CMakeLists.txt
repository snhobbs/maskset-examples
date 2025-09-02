cmake_minimum_required(VERSION 3.25)
project(nrf52_application C ASM)
set(LDSCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.ld")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Directories
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(COMMON_DIR ${ROOT_DIR}/common)
set(BOOTLOADER_DIR ${ROOT_DIR}/bootloader)
set(APPLICATION_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(EXTERNAL_DIR ${ROOT_DIR}/external)
set(BUILD_DIR ${APPLICATION_DIR})

# Include paths
include_directories(
  ${BOOTLOADER_DIR}/include ${COMMON_DIR}/include
  ${EXTERNAL_DIR}/mcuboot/boot/bootutil/include
  ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/include)

# Source files
set(APPLICATION_SRC ${APPLICATION_DIR}/src/main.c)

set(MCUBOOT_SRC ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/bootutil_misc.c
                ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/bootutil_public.c)

set(COMMON_SRC
    ${COMMON_DIR}/src/mcuboot_port.c
    ${COMMON_DIR}/src/log_port.c
    ${COMMON_DIR}/src/minimal_nrf52_uart.c
    ${COMMON_DIR}/src/minimal_nrf52_flash.c
    ${COMMON_DIR}/src/shell.c
    ${COMMON_DIR}/src/shell_port.c
    ${COMMON_DIR}/src/shell_commands.c
    ${COMMON_DIR}/src/startup.c)

set(ALL_SRC ${APPLICATION_SRC} ${MCUBOOT_SRC} ${COMMON_SRC})

# Include paths
include_directories(${APPLICATION_DIR}/include ${COMMON_DIR}/include
                    ${MCUBOOT_DIR}/include ${ROOT_DIR}/bootloader/config)

# Set C standard
set(CMAKE_C_STANDARD 23) # C11, C17, or C23, etc.
set(CMAKE_C_STANDARD_REQUIRED ON) # Make it an error if compiler doesn’t support
set(CMAKE_C_EXTENSIONS OFF) # Use strictly standard C (no GNU extensions)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
# Add executable
add_executable(${PROJECT_NAME}.elf ${ALL_SRC})

set(MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfloat-abi=soft
    # -mfpu=fpv4-sp-d16
    -fno-exceptions # Turns off exceptions for languages that support it
    -g3
    -O0
    # -fdebug-prefix-map=${ROOT_DIR}=.
    -ffunction-sections
    -fdata-sections
    -Wall)
# -Werror)

# Compile flags
target_compile_options(${PROJECT_NAME}.elf PRIVATE ${MCU_FLAGS})

# Linker flags
target_link_options(
  ${PROJECT_NAME}.elf
  PRIVATE
  ${MCU_FLAGS}
  -T${LDSCRIPT}
  -Wl,-Map=${BUILD_DIR}/${PROJECT_NAME}.map
  -Wl,-z
  -Wl,muldefs # Send keyword to linker: Allow multiple definitions
  -Wl,--no-export-dynamic # Also keeps CMake from adding rdynamic as a flag
  -Wl,--gc-sections
  -Wl,--sort-section=alignment # Sort names by maximum alignment
  -Wl,-print-memory-usage # Print size of link sections after compilation
  -static # Prevents linking with shared libraries
  # --specs=nano.specs -nostartfiles -nostdlib --specs=nosys.specs
  # -nodefaultlibs                 # Need to have these -nostartfiles # Need to
  # have these -nostdlib # Need to have these
  -Wl,-lc
  -Wl,-lgcc)

target_link_libraries(
  ${PROJECT_NAME}.elf
  PRIVATE $<$<C_COMPILER_ID:GNU>:-Wl,--start-group>
          $<$<C_COMPILER_ID:GNU>:m> # libm math library
          $<$<C_COMPILER_ID:GNU>:c> # libc
          $<$<C_COMPILER_ID:GNU>:gcc> # libgcc
          $<$<C_COMPILER_ID:GNU>:nosys> # bare metal target
          $<$<C_COMPILER_ID:GNU>:-Wl,--end-group>
          $<$<C_COMPILER_ID:GNU>:--specs=nano.specs> # newlibnano library
          $<$<C_COMPILER_ID:GNU>:--specs=nosys.specs> # nosemi hosting
)

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE MCUBOOT_HAVE_LOG=0
                                                       CONFIG_MCUBOOT=0)
# IDN="Maskset,Application Example Slot 0,1,0.1.0")

# Create build directory
file(MAKE_DIRECTORY ${BUILD_DIR})

# Output directories
set_target_properties(${PROJECT_NAME}.elf PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                     ${BUILD_DIR})

# Custom command to create binary
add_custom_command(
  OUTPUT ${BUILD_DIR}/${PROJECT_NAME}.bin
  COMMAND ${CMAKE_OBJCOPY} -O binary ${BUILD_DIR}/${PROJECT_NAME}.elf
          ${BUILD_DIR}/${PROJECT_NAME}.bin
  DEPENDS ${PROJECT_NAME}.elf)

# Custom command to sign MCUboot image
set(IMGTOOL_PY ${ROOT_DIR}/external/mcuboot/scripts/imgtool.py)
add_custom_command(
  OUTPUT ${BUILD_DIR}/${PROJECT_NAME}-signed.bin
  COMMAND
    python3 ${IMGTOOL_PY} sign --header-size 0x200 --align 4 --slot-size 0x20000
    -v 1.0.0 --pad-header --erased-val 0xff --pad
    ${BUILD_DIR}/${PROJECT_NAME}.bin ${BUILD_DIR}/${PROJECT_NAME}-signed.bin
  DEPENDS ${BUILD_DIR}/${PROJECT_NAME}.bin)

# Add a pseudo-target for 'all'
add_custom_target(all_images ALL
                  DEPENDS ${BUILD_DIR}/${PROJECT_NAME}-signed.bin)

# Optional: Flash command
add_custom_target(
  flash
  COMMAND
    arm-none-eabi-gdb-py --eval-command="target remote localhost:2331"
    --ex="mon reset"
    --ex="restore ${BUILD_DIR}/${PROJECT_NAME}.bin binary 0x8000"
    --ex="mon reset" --se=${BUILD_DIR}/${PROJECT_NAME}.elf
  DEPENDS all_images
  COMMENT "Flashing ${PROJECT_NAME} to the target via GDB"
  VERBATIM)
