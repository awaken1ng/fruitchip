cmake_minimum_required(VERSION 3.13)

project(fruitchip-loader)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(EE_STAGE_1_ADDR 0x001001B0)
set(EE_STAGE_2_ADDR 0x00094000)

add_subdirectory(stage1)
add_subdirectory(stage2)
