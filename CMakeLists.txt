cmake_minimum_required(VERSION 3.21)

set(PICO_USE_FASTEST_SUPPORTED_CLOCK 1)
set(PICO_COMPILER "pico_arm_cortex_m0plus_clang")
include(sdk/pico_sdk_init.cmake)

project(my_project)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

pico_sdk_init()

add_executable(my_project
    src/main.c
)

pico_generate_pio_header(my_project
    ${CMAKE_CURRENT_SOURCE_DIR}/src/boot_rom_passive_reader.pio
)

# region: loader

add_dependencies(my_project loader)

add_custom_target(loader
    SOURCES
        ps2bbl/bin/COMPRESSED_PS2BBL.ELF
        loader/ee-stage2/bin/ee-stage2.bin
        loader/ee-stage1/bin/ee-stage1.bin
        loader/_always_rerun
        ps2bbl/_always_rerun
)

add_custom_command(
    OUTPUT
        ps2bbl/bin/COMPRESSED_PS2BBL.ELF
        ps2bbl/_always_rerun
    COMMAND make packed MX4SIO=0 MMCE=0 PPCTTY=1 DEBUG=1
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ps2bbl
)

add_custom_command(
    OUTPUT
        loader/ee-stage2/bin/ee-stage2.bin
        loader/ee-stage1/bin/ee-stage1.bin
        loader/_always_rerun
    COMMAND make clean && make EE_STAGE_3_FILE=../ps2bbl/bin/COMPRESSED_PS2BBL.ELF
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/loader
)

# endregion: loader


target_link_libraries(my_project
    pico_stdlib
    pico_stdio_rtt
    pico_multicore
    hardware_pio
    hardware_dma
)

pico_enable_stdio_uart(my_project 0)
pico_enable_stdio_rtt(my_project 1)

pico_add_uf2_output(my_project)

set_property(TARGET my_project APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")
