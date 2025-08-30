cmake_minimum_required(VERSION 3.25)
project(nrf52_bootloader C ASM)

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

# Directories
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(COMMON_DIR ${ROOT_DIR}/common)
set(BOOTLOADER_DIR ${ROOT_DIR}/bootloader)
set(EXTERNAL_DIR ${ROOT_DIR}/external)
set(BUILD_DIR ${BOOTLOADER_DIR}/build)

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
  -T${LDSCRIPT}
  -Wl,-Map=${BUILD_DIR}/nrf52-bootloader.map
  -Wl,--gc-sections
  --specs=nano.specs
  -lc
  -lgcc
  -lc)

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE MCUBOOT_HAVE_LOG=0
                                                       CONFIG_MCUBOOT=0)

# Create build directory
file(MAKE_DIRECTORY ${BUILD_DIR})
