cmake_minimum_required(VERSION 3.25)
project(nrf52_bootloader C ASM)

# Set C standard
set(CMAKE_C_STANDARD 23) # C11, C17, or C23, etc.
set(CMAKE_C_STANDARD_REQUIRED ON) # Make it an error if compiler doesn’t support
set(CMAKE_C_EXTENSIONS OFF) # Use strictly standard C (no GNU extensions)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Cortex-M4 flags
set(MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfloat-abi=soft
    # -mfpu=fpv4-sp-d16
    -fno-exceptions # Turns off exceptions for languages that support it
    -g3
    -Os
    # -fdebug-prefix-map=${ROOT_DIR}=.
    -ffunction-sections
    -fdata-sections
    -Wall)
# -Werror)

# Directories
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(BOOTLOADER_DIR ${ROOT_DIR}/bootloader)
set(COMMON_DIR ${ROOT_DIR}/common)
set(EXTERNAL_DIR ${ROOT_DIR}/external)
set(BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Include paths
include_directories(
  ${BOOTLOADER_DIR}/include ${COMMON_DIR}/include
  ${EXTERNAL_DIR}/mcuboot/boot/bootutil/include
  ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/include)

# Bootloader sources
set(BOOTLOADER_SRC
    ${BOOTLOADER_DIR}/src/log_port.c
    ${BOOTLOADER_DIR}/src/main.c
    ${BOOTLOADER_DIR}/src/minimal_nrf52_uart.c
    ${BOOTLOADER_DIR}/src/minimal_nrf52_flash.c
    ${BOOTLOADER_DIR}/src/shell.c
    ${BOOTLOADER_DIR}/src/shell_commands.c
    ${BOOTLOADER_DIR}/src/shell_port.c
    ${BOOTLOADER_DIR}/src/startup.c)

# Common sources
set(COMMON_SRC ${COMMON_DIR}/src/mcuboot_port.c)

# MCUboot sources
set(MCUBOOT_SRC
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/boot_record.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/bootutil_misc.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/caps.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/encrypted.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/fault_injection_hardening.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/fault_injection_hardening_delay_rng_mbedtls.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/image_ec.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/image_ec256.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/image_ed25519.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/image_rsa.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/image_validate.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/loader.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/swap_misc.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/swap_move.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/bootutil_public.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/swap_scratch.c
    ${EXTERNAL_DIR}/mcuboot/boot/bootutil/src/tlv.c)

# TinyCrypt sources
set(TINYCRYPT_SRC
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/aes_decrypt.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/aes_encrypt.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/cbc_mode.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/ccm_mode.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/cmac_mode.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/ctr_mode.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/ctr_prng.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/hmac.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/hmac_prng.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/sha256.c
    ${EXTERNAL_DIR}/mcuboot/ext/tinycrypt/lib/source/utils.c)

# Combined sources
set(ALL_SRC ${BOOTLOADER_SRC} ${COMMON_SRC} ${MCUBOOT_SRC} ${TINYCRYPT_SRC})

# Add executable
add_executable(${PROJECT_NAME}.elf ${ALL_SRC})

# Compile flags
target_compile_options(${PROJECT_NAME}.elf PRIVATE ${MCU_FLAGS})

# Linker flags
set(LDSCRIPT "${BOOTLOADER_DIR}/nrf52-bootloader.ld")

target_link_options(
  ${PROJECT_NAME}.elf
  PRIVATE
  ${MCU_FLAGS}
  -T${LDSCRIPT}
  -Wl,-Map=${BUILD_DIR}/nrf52-bootloader.map
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

# Create build directory
file(MAKE_DIRECTORY ${BUILD_DIR})
