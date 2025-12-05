cmake_minimum_required(VERSION 3.13)

project(fruitchip-test-ps2)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

string(REPLACE "-DNDEBUG" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

set(IS_RELEASE_BUILD CMAKE_BUILD_TYPE MATCHES "^Release")

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_executable(fruitchip-test-ps2
    src/test/apps.c
    src/test/data_out.c
    src/test/menu.c
    src/test/settings.c
    ${CMAKE_CURRENT_BINARY_DIR}/src/embedded_irx.c
    src/main.c
)

target_include_directories(fruitchip-test-ps2
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../fruitchip-include
)

target_link_libraries(fruitchip-test-ps2
    patches
    -ldebug
)

target_compile_options(fruitchip-test-ps2
    PUBLIC
        -O3
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/embedded_irx.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/embedded_irx.c
)

add_custom_command(
    TARGET fruitchip-test-ps2
    POST_BUILD
    COMMAND
        ps2-packer
        $<TARGET_FILE:fruitchip-test-ps2>
        $<TARGET_FILE_BASE_NAME:fruitchip-test-ps2>-packed.elf
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
