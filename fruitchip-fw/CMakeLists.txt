cmake_minimum_required(VERSION 3.21)

include(../fruitchip-fw-board/board_configuration.cmake)

set(PICO_USE_FASTEST_SUPPORTED_CLOCK 1)
set(PICO_COMPILER "pico_arm_cortex_m0plus_clang")
include(../pico-sdk/pico_sdk_init.cmake)

project(fruitchip-fw)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 23)

pico_sdk_init()

add_executable(fruitchip-fw
    lib/fnv/hash_64a.c

    lib/wear_leveling/wear_leveling.c
    lib/wear_leveling/wear_leveling_rp2040_flash.c

    src/git_version.c
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
    src/boot_rom/write/test.c
    src/boot_rom/write/version.c
    src/reset.c
    src/settings.c
)

target_include_directories(fruitchip-fw
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/fnv
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/wear_leveling
        ${CMAKE_CURRENT_SOURCE_DIR}/../fruitchip-include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/status_led/include
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
        FW_PARTITION_SIZE_BYTES=${FW_PARTITION_SIZE_BYTES}
        APPS_PARTITION_SIZE_BYTES=${APPS_PARTITION_SIZE_BYTES}
        UPDATE_PARTITION_SIZE_BYTES=${UPDATE_PARTITION_SIZE_BYTES}
        APPS_PARTITION_OFFSET=${APPS_PARTITION_OFFSET}
        UPDATE_PARTITION_OFFSET=${UPDATE_PARTITION_OFFSET}
        WEAR_LEVELING_BACKING_SIZE=${WEAR_LEVELING_BACKING_SIZE}
        WEAR_LEVELING_RP2040_FLASH
        WEAR_LEVELING_RP2040_FLASH_BASE=${WEAR_LEVELING_RP2040_FLASH_BASE}
        PICO_XOSC_STARTUP_DELAY_MULTIPLIER=0
)

if (DEFINED PICO_DEFAULT_WS2812_PIN)
    target_compile_definitions(fruitchip-fw
        PUBLIC
            PICO_DEFAULT_WS2812_PIN=${PICO_DEFAULT_WS2812_PIN}
    )
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/boot_rom/boot_rom.pio
    ${CMAKE_CURRENT_BINARY_DIR}/include/boot_rom.pio
)

pico_generate_pio_header(fruitchip-fw
    ${CMAKE_CURRENT_BINARY_DIR}/include/boot_rom.pio
    OUTPUT_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/include
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/boot_rom/loader.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/boot_rom/loader.c
)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../pico-status-led
    ${CMAKE_CURRENT_BINARY_DIR}/pico-status-led
)

target_link_libraries(fruitchip-fw
    pico_stdlib
    pico_stdio_rtt
    pico_multicore
    status_led
    hardware_pio
    hardware_dma
    hardware_flash
)

pico_enable_stdio_rtt(fruitchip-fw 1)

if(DEFINED UART_TX_PIN AND DEFINED UART_RX_PIN)
    target_link_libraries(fruitchip-fw
        pico_stdio_uart
    )

    target_compile_definitions(fruitchip-fw
        PUBLIC
            PICO_DEFAULT_UART_TX_PIN=${UART_TX_PIN}
            PICO_DEFAULT_UART_RX_PIN=${UART_RX_PIN}
    )

    pico_enable_stdio_uart(fruitchip-fw 1)
else()
    # UART is enabled by default
    pico_enable_stdio_uart(fruitchip-fw 0)
endif()

pico_add_uf2_output(fruitchip-fw)

set_property(TARGET fruitchip-fw APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")

include(../git_version.cmake)

configure_git_version_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/git_version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/git_version.h
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/pico_flash_region.template.ld
    ${CMAKE_BINARY_DIR}/pico_flash_region.ld
)
