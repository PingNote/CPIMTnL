# CMake Version
cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)

project(TTL)
# Include Library:
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Compile and Run
add_executable(main main.cpp main.h config.h)

# Links Libraries
target_link_libraries(main ${CONAN_LIBS})

# C++ Version:
target_compile_features(main PUBLIC cxx_std_20)
