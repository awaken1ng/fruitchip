cmake_minimum_required(VERSION 3.13)

project(fruitchip-fwfs)

add_executable(fruitchip-fwfs
    src/fwfs.c
    ${CMAKE_CURRENT_BINARY_DIR}/imports.c
    src/main.c
)

target_include_directories(fruitchip-fwfs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../fruitchip-include
)

build_iop_imports(
    ${CMAKE_CURRENT_BINARY_DIR}/imports.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imports.lst
)

add_custom_command(
    TARGET fruitchip-fwfs POST_BUILD
    COMMAND
        ${CMAKE_STRIP}
        --strip-unneeded
        --remove-section=.pdr
        --remove-section=.comment
        --remove-section=.mdebug.abi32
        --remove-section=.gnu.attributes
        ${CMAKE_CURRENT_BINARY_DIR}/fruitchip-fwfs
)

add_custom_command(
    TARGET fruitchip-fwfs POST_BUILD
    COMMAND
        iopfixup
        --irx1
        -o ${CMAKE_CURRENT_BINARY_DIR}/fruitchip-fwfs.irx
        ${CMAKE_CURRENT_BINARY_DIR}/fruitchip-fwfs
)
