cmake_minimum_required(VERSION 3.21)

set(PICO_USE_FASTEST_SUPPORTED_CLOCK 1)
set(PICO_COMPILER "pico_arm_cortex_m0plus_clang")
include(../pico-sdk/pico_sdk_init.cmake)

project(fruitchip-fw)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 23)

pico_sdk_init()

include(../fruitchip-fw-board/board_configuration.cmake)

add_executable(fruitchip-fw
    lib/fnv/hash_64a.c

    lib/wear_leveling/wear_leveling.c
    lib/wear_leveling/wear_leveling_rp2040_flash.c

    src/main.c
    src/boot_rom/handler.c
    src/boot_rom/data_out.c
    ${CMAKE_CURRENT_BINARY_DIR}/src/boot_rom/loader.c
    src/boot_rom/read/idle.c
    src/boot_rom/read/osdsys.c
    src/boot_rom/write/disable_next_osdsys_hook.c
    src/boot_rom/write/flash.c
    src/boot_rom/write/get_payload.c
    src/boot_rom/write/idle.c
    src/boot_rom/write/kv.c
    src/boot_rom/write/read_app.c
    src/boot_rom/write/version.c
    src/reset.c
    src/settings.c
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)

target_include_directories(fruitchip-fw
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/fnv
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/wear_leveling
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../fruitchip-include
)

target_compile_definitions(fruitchip-fw
    PUBLIC
        # dont initialize PICO_DEFAULT_LED_PIN in status_led to avoid "detaching" it from PIO
        PICO_STATUS_LED_AVAILABLE=0
        BOOT_ROM_Q0_PIN=${BOOT_ROM_Q0_PIN}
        BOOT_ROM_CE_PIN=${BOOT_ROM_CE_PIN}
        BOOT_ROM_OE_PIN=${BOOT_ROM_OE_PIN}
        RST_PIN=${RST_PIN}
        PICO_FLASH_SIZE_BYTES=${PICO_FLASH_SIZE_BYTES}
        PICO_DEFAULT_WS2812_PIN=${PICO_DEFAULT_WS2812_PIN}
        APPS_PARTITION_OFFSET=${APPS_PARTITION_OFFSET}
        UPDATE_PARTITION_OFFSET=${UPDATE_PARTITION_OFFSET}
        WEAR_LEVELING_BACKING_SIZE=${WEAR_LEVELING_BACKING_SIZE}
        WEAR_LEVELING_RP2040_FLASH
        WEAR_LEVELING_RP2040_FLASH_BASE=${WEAR_LEVELING_RP2040_FLASH_BASE}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/boot_rom/boot_rom.pio ${CMAKE_CURRENT_BINARY_DIR}/boot_rom.pio)

pico_generate_pio_header(fruitchip-fw
    ${CMAKE_CURRENT_BINARY_DIR}/boot_rom.pio
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/boot_rom/loader.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/boot_rom/loader.c
)

target_link_libraries(fruitchip-fw
    pico_stdlib
    pico_stdio_rtt
    pico_multicore
    pico_status_led
    hardware_pio
    hardware_dma
    hardware_flash
)

pico_enable_stdio_uart(fruitchip-fw 0)
pico_enable_stdio_rtt(fruitchip-fw 1)

pico_add_uf2_output(fruitchip-fw)

set_property(TARGET fruitchip-fw APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/_version.cpp  # always re-run before the build
    COMMAND
        ${CMAKE_COMMAND} -P
        ${CMAKE_CURRENT_SOURCE_DIR}/../version.cmake
)
