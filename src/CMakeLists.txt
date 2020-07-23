# CMake Version
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

project(CPIMTnL)

# CMake Variables
# cpprestsdk can' not compile on GCC 10
# https://github.com/microsoft/cpprestsdk/issues/1461
# rollback to GCC 9 for now
# and we need to set CONAN_DISABLE_CHECK_COMPILER
# https://docs.conan.io/en/latest/reference/generators/cmake.html#input-variables-for-conanbuildinfo-cmake
# set(CONAN_DISABLE_CHECK_COMPILER TRUE)
# or switch to Clang/LLVM

# Include Library:
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Compile and Run
add_executable(main main.cpp main.h config.h)

# Links Libraries
target_link_libraries(main ${CONAN_LIBS})

# C++ Version:
target_compile_features(main PUBLIC cxx_std_20)
