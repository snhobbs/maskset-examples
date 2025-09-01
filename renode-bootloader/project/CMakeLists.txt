cmake_minimum_required(VERSION 3.25)
set(CMAKE_GENERATOR
    "Ninja"
    CACHE INTERNAL "" FORCE)

project(nrf52-project C CXX ASM)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Add subdirectories add_subdirectory(common) add_subdirectory(external/mcuboot)
# add_subdirectory(external/mcuboot/ext/tinycrypt/lib)
add_subdirectory(bootloader)
add_subdirectory(application)
