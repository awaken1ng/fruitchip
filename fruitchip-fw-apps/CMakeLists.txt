cmake_minimum_required(VERSION 3.21)

include(../pico-sdk/pico_sdk_init.cmake)

project(fruitchip-fw-apps)

pico_sdk_init()

include(../fruitchip-fw-board/board_configuration.cmake)

add_custom_target(
    fruitchip-fw-apps
    ALL
    SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/ps2bbl_mx4sio.elf
        ${CMAKE_CURRENT_BINARY_DIR}/ps2bbl_mmce.elf
        ${CMAKE_CURRENT_BINARY_DIR}/osdmenu.elf
        ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
        apps/_always_rerun
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/ps2bbl_mx4sio.elf
    COMMAND make clean packed MX4SIO=1
    COMMAND cmake -E copy bin/COMPRESSED_PS2BBL.ELF ${CMAKE_CURRENT_BINARY_DIR}/ps2bbl_mx4sio.elf
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ps2bbl
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/ps2bbl_mmce.elf
    COMMAND make clean packed MMCE=1
    COMMAND cmake -E copy bin/COMPRESSED_PS2BBL.ELF ${CMAKE_CURRENT_BINARY_DIR}/ps2bbl_mmce.elf
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ps2bbl
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/osdmenu.elf
    COMMAND make -C patcher clean patcher.elf
    COMMAND cmake -E copy patcher/patcher.elf ${CMAKE_CURRENT_BINARY_DIR}/osdmenu.elf
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/osdmenu
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
        apps/_always_rerun
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/ps2bbl_mx4sio.elf
        ${CMAKE_CURRENT_BINARY_DIR}/ps2bbl_mmce.elf
        ${CMAKE_CURRENT_BINARY_DIR}/osdmenu.elf
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/build_apps_partition.py ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

math(EXPR APPS_PARTITION_ADDR "0x10000000 + ${APPS_PARTITION_OFFSET}" OUTPUT_FORMAT HEXADECIMAL)

pico_init_picotool()

add_custom_command(
    TARGET fruitchip-fw-apps POST_BUILD
    COMMAND picotool
    ARGS
        uf2
        convert
        ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
        ${CMAKE_CURRENT_BINARY_DIR}/apps.uf2
        --family rp2040
        -o ${APPS_PARTITION_ADDR}
    VERBATIM
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/apps.uf2
)
