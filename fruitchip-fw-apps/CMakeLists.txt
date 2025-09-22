cmake_minimum_required(VERSION 3.21)

include(../pico-sdk/pico_sdk_init.cmake)

project(fruitchip-fw-apps)

pico_sdk_init()

include(../fruitchip-fw-board/board_configuration.cmake)

add_custom_target(
    fruitchip-fw-apps
    ALL
    SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
        apps/_always_rerun
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
        apps/_always_rerun
    DEPENDS
        ../ps2bbl/bin/COMPRESSED_PS2BBL.ELF
    COMMAND python build_apps_partition.py ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

pico_init_picotool()

add_custom_command(
    TARGET fruitchip-fw-apps POST_BUILD
    COMMAND picotool
    ARGS
        uf2
        convert
        ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
        ${CMAKE_CURRENT_BINARY_DIR}/apps.uf2
        --family ${PICO_PLATFORM}
        -o ${APPS_PARTITION_ADDR}
    VERBATIM
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/apps.uf2
)
