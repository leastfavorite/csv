cmake_minimum_required(VERSION 4.0)
project(csv)

file(GLOB_RECURSE sources src/*.cc)
add_executable(csv ${sources})
target_compile_options(csv PUBLIC -std=c++20 -Wall -Wextra -Wpedantic -Werror)
target_include_directories(csv PUBLIC include/)
