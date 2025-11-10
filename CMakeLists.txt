cmake_minimum_required(VERSION 3.26)

project(fruitchip)

include(ExternalProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(LOADER_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/loader)
set(FWFS_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/fwfs)

set(BOARD "yd-rp2040")

ExternalProject_Add(
    fruitchip-loader
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fruitchip-loader"
    CMAKE_ARGS
        -DCMAKE_EXPORT_COMPILE_COMMANDS=${CMAKE_EXPORT_COMPILE_COMMANDS}
        -DCMAKE_TOOLCHAIN_FILE=$ENV{PS2SDK}/ps2dev.cmake
        -DCMAKE_INSTALL_PREFIX=${LOADER_INSTALL_PREFIX}
    INSTALL_BYPRODUCTS
        ${LOADER_INSTALL_PREFIX}/fruitchip-loader-ee-stage1.bin
        ${LOADER_INSTALL_PREFIX}/fruitchip-loader-ee-stage2.bin
    BUILD_ALWAYS 1
)

ExternalProject_Add(
    fruitchip-fw-apps
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fruitchip-fw-apps"
    CMAKE_ARGS
        -DBOARD=${BOARD}
    INSTALL_COMMAND cmake -E true
)

ExternalProject_Add(
    fruitchip-fwfs
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fruitchip-fwfs"
    CMAKE_ARGS
        -DCMAKE_EXPORT_COMPILE_COMMANDS=${CMAKE_EXPORT_COMPILE_COMMANDS}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/fruitchip-fwfs/ps2dev_iop.cmake
        -DCMAKE_INSTALL_PREFIX=${FWFS_INSTALL_PREFIX}
    BUILD_ALWAYS 1
)

ExternalProject_Add(
    fruitchip-menu
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fruitchip-menu"
    CMAKE_ARGS
        -DCMAKE_EXPORT_COMPILE_COMMANDS=${CMAKE_EXPORT_COMPILE_COMMANDS}
        -DCMAKE_TOOLCHAIN_FILE=$ENV{PS2SDK}/ps2dev.cmake
        -DFWFS_INSTALL_PREFIX=${FWFS_INSTALL_PREFIX}
    DEPENDS
        fruitchip-fwfs
    INSTALL_COMMAND cmake -E true
    BUILD_ALWAYS 1
)

ExternalProject_Add(
    fruitchip-fw
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fruitchip-fw"
    CMAKE_ARGS
        -DBOARD=${BOARD}
        -DCMAKE_EXPORT_COMPILE_COMMANDS=${CMAKE_EXPORT_COMPILE_COMMANDS}
        -DLOADER_INSTALL_PREFIX=${LOADER_INSTALL_PREFIX}
    INSTALL_COMMAND cmake -E true
    DEPENDS fruitchip-loader
    # helps to avoid reconfiguring the project due to rebuilding loader, even if there were no changes
    CONFIGURE_HANDLED_BY_BUILD 1
)

ExternalProject_Add(
    fruitchip-fw-bootloader
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fruitchip-fw-bootloader"
    CMAKE_ARGS
        -DBOARD=${BOARD}
        -DCMAKE_EXPORT_COMPILE_COMMANDS=${CMAKE_EXPORT_COMPILE_COMMANDS}
    INSTALL_COMMAND cmake -E true
    BUILD_ALWAYS 1
)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    macro(append_to_compilation_databases TARGET)
        ExternalProject_Get_property(${TARGET} BINARY_DIR)
        list(APPEND COMPILATION_DATABASES ${BINARY_DIR}/compile_commands.json)
    endmacro()

    append_to_compilation_databases(fruitchip-fw-bootloader)
    append_to_compilation_databases(fruitchip-fw)
    append_to_compilation_databases(fruitchip-fwfs)
    append_to_compilation_databases(fruitchip-menu)
    append_to_compilation_databases(fruitchip-loader)

    add_custom_target(
        merge-compilation-databases
        ALL
        SOURCES
            ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        DEPENDS
            ${COMPILATION_DATABASES}
    )

    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        COMMAND
            jq --slurp add ${COMPILATION_DATABASES} > compile_commands.json
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
