cmake_minimum_required(VERSION 3.13)

project(masswatch-iop)

add_executable(masswatch-iop
    ${CMAKE_CURRENT_BINARY_DIR}/imports.c
    src/main.c
)

target_include_directories(masswatch-iop
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

build_iop_imports(
    ${CMAKE_CURRENT_BINARY_DIR}/imports.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imports.lst
)

add_custom_command(
    TARGET masswatch-iop POST_BUILD
    COMMAND
        ${CMAKE_STRIP}
        --strip-unneeded
        --remove-section=.pdr
        --remove-section=.comment
        --remove-section=.mdebug.abi32
        --remove-section=.gnu.attributes
        ${CMAKE_CURRENT_BINARY_DIR}/masswatch-iop
)

add_custom_command(
    TARGET masswatch-iop POST_BUILD
    COMMAND
        iopfixup
        --irx1
        -o ${CMAKE_CURRENT_BINARY_DIR}/masswatch.irx
        ${CMAKE_CURRENT_BINARY_DIR}/masswatch-iop
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/masswatch.irx
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
