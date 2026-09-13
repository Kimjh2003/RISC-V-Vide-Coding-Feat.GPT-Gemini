cmake_minimum_required(VERSION 3.22)
project(riscv_accelerator_simulator LANGUAGES CXX)

add_library(riscv_accelerator_simulator STATIC
    src/rv64_decoder.cpp
    src/xvec_queue.cpp
    src/pmp_policy.cpp)
target_include_directories(riscv_accelerator_simulator PUBLIC include)
target_compile_features(riscv_accelerator_simulator PUBLIC cxx_std_20)
target_compile_options(riscv_accelerator_simulator PRIVATE -Wall -Wextra -Werror)

option(RVSIM_BUILD_TESTS "Build simulator tests" OFF)
if (RVSIM_BUILD_TESTS)
    enable_testing()
    add_executable(rvsim_tests tests/rvsim_tests.cpp)
    target_link_libraries(rvsim_tests PRIVATE riscv_accelerator_simulator)
    add_test(NAME rvsim_tests COMMAND rvsim_tests)
endif()

