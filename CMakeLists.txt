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
    src/boot_rom_data_out.c
    src/boot_rom_read_handler.c
    src/boot_rom_write_handler.c
    src/reset.c
)

# board configuration
set(BOOT_ROM_Q0_PIN 2)
set(BOOT_ROM_CE_PIN 10)
set(BOOT_ROM_OE_PIN 11)
set(RST_PIN 12)
set(PICO_DEFAULT_WS2812_PIN 23)

# split flash in 3 parts to keep FW and apps separate,
# have an update partition to be able to atomicaly update either partition,
# also reserve a bit of space for keeping settings
set(FLASH_SIZE_MB 2)
set(PICO_FLASH_SIZE_BYTES "680 * 1024")   # 0x10000000, 680 KB

if(FLASH_SIZE_MB EQUAL 2)
    math(EXPR APPS_PARTITION_OFFSET "0x10000000 + (680 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 680 KB
    math(EXPR UPDATE_PARTITION_OFFSET "0x10000000 + (2 * 680 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 680 KB
    math(EXPR SETTINGS_PARTITION_OFFSET "0x10000000 + (3 * 680 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 8 KB
elseif(FLASH_SIZE_MB EQUAL 4)
    math(EXPR APPS_PARTITION_OFFSET "0x10000000 + (680 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 1704 KB
    math(EXPR UPDATE_PARTITION_OFFSET "0x10000000 + (680 * 1024) + (1 * 1704 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 1704 KB
    math(EXPR SETTINGS_PARTITION_OFFSET "0x10000000 + (680 * 1024) + (2 * 1704 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 8 KB
elseif(FLASH_SIZE_MB EQUAL 8)
    math(EXPR APPS_PARTITION_OFFSET "0x10000000 + (680 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 3752 KB
    math(EXPR UPDATE_PARTITION_OFFSET "0x10000000 + (680 * 1024) + (1 * 3752 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 3752 KB
    math(EXPR SETTINGS_PARTITION_OFFSET "0x10000000 + (680 * 1024) + (2 * 3752 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 8 KB
elseif(FLASH_SIZE_MB EQUAL 16)
    math(EXPR APPS_PARTITION_OFFSET "0x10000000 + (680 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 7848 KB
    math(EXPR UPDATE_PARTITION_OFFSET "0x10000000 + (680 * 1024) + (1 * 7848 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 7848 KB
    math(EXPR SETTINGS_PARTITION_OFFSET "0x10000000 + (680 * 1024) + (2 * 7848 * 1024)" OUTPUT_FORMAT HEXADECIMAL) # 8 KB
else()
    # max supported flash size by RP2040 is 16 MB (see 2.6.3.1 in datasheet)
    message(FATAL_ERROR "Unsupported flash size")
endif()

target_compile_definitions(my_project
    PUBLIC
        BOOT_ROM_Q0_PIN=${BOOT_ROM_Q0_PIN}
        BOOT_ROM_CE_PIN=${BOOT_ROM_CE_PIN}
        BOOT_ROM_OE_PIN=${BOOT_ROM_OE_PIN}
        RST_PIN=${RST_PIN}
        APPS_PARTITION_OFFSET=${APPS_PARTITION_OFFSET}
        UPDATE_PARTITION_OFFSET=${UPDATE_PARTITION_OFFSET}
        SETTINGS_PARTITION_OFFSET=${SETTINGS_PARTITION_OFFSET}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/boot_rom.pio ${CMAKE_CURRENT_BINARY_DIR}/boot_rom.pio)

pico_generate_pio_header(my_project
    ${CMAKE_CURRENT_BINARY_DIR}/boot_rom.pio
)

# region: loader

add_dependencies(my_project loader)

add_custom_target(loader
    SOURCES
        ps2bbl/bin/COMPRESSED_PS2BBL.ELF
        ps2bbl/_always_rerun
        loader/ee-stage2/bin/ee-stage2.bin
        loader/ee-stage1/bin/ee-stage1.bin
        loader/_always_rerun
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
    DEPENDS
        ps2bbl/bin/COMPRESSED_PS2BBL.ELF
    COMMAND make clean && make EE_STAGE_3_FILE=../ps2bbl/bin/COMPRESSED_PS2BBL.ELF
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/loader
)

# endregion: loader

# region: apps

add_dependencies(my_project apps)

add_custom_target(apps
    SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
        apps/_always_rerun
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
        apps/_always_rerun
    DEPENDS
        ps2bbl/bin/COMPRESSED_PS2BBL.ELF
    COMMAND python tools/build_apps_partition.py ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

pico_init_picotool()

add_custom_command(
    TARGET apps POST_BUILD
    COMMAND picotool
    ARGS
        uf2
        convert
        ${CMAKE_CURRENT_BINARY_DIR}/apps.bin
        ${CMAKE_CURRENT_BINARY_DIR}/apps.uf2
        --family ${PICO_PLATFORM}
        -o ${APPS_PARTITION_OFFSET}
    VERBATIM
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/apps.uf2
)

# endregion: apps



target_link_libraries(my_project
    pico_stdlib
    pico_stdio_rtt
    pico_multicore
    pico_status_led
    hardware_pio
    hardware_dma
)

pico_enable_stdio_uart(my_project 0)
pico_enable_stdio_rtt(my_project 1)

pico_add_uf2_output(my_project)

set_property(TARGET my_project APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")
