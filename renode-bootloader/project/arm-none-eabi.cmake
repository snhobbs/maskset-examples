set(ROOT
    $ENV{HOME}/ToolChains/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi/bin)
set(CMAKE_C_COMPILER ${ROOT}/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${ROOT}/arm-none-eabi-g++)
set(CMAKE_OBJCOPY ${ROOT}/arm-none-eabi-objcopy)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
