cmake_minimum_required(VERSION 3.20)
project(
  war-simulator
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
# set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.cpp src/war-simulator.cpp)
target_include_directories(${PROJECT_NAME}
                           PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_options(
  ${PROJECT_NAME}
  PRIVATE -Wall
          -Wextra
          -Wpedantic
          -O2
          -march=native
          -fno-exceptions
          -fno-rtti
          -fomit-frame-pointer
          -flto
          -funroll-loops
          -ffast-math)

target_precompile_headers(${PROJECT_NAME} PRIVATE <iostream> <vector> <deque>)

# --- Fetch GoogleTest ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip)
# Prevent GoogleTest from overriding compiler/linker options
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Tests ---
enable_testing()

add_executable(${PROJECT_NAME}_tests tests/src/test_main.cpp)

target_include_directories(${PROJECT_NAME}_tests
                           PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}_tests PRIVATE GTest::gtest
                                                    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)
