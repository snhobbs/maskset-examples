cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sample_with_mcuboot)

test_sysbuild()
zephyr_link_libraries(MCUBOOT_BOOTUTIL)

target_sources(app PRIVATE src/main.c)
