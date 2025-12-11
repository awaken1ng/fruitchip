cmake_minimum_required(VERSION 3.21)

include(../fruitchip-fw-board/board_configuration.cmake)

set(PICO_COMPILER "pico_arm_cortex_m0plus_clang")
include(../pico-sdk/pico_sdk_init.cmake)

project(fruitchip-fw-bootloader)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 23)

pico_sdk_init()

add_executable(fruitchip-fw-bootloader
    src/main.c
)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../pico-status-led
    ${CMAKE_CURRENT_BINARY_DIR}/pico-status-led
)

target_link_libraries(fruitchip-fw-bootloader
    pico_stdlib
    pico_stdio_rtt
    status_led
    hardware_dma
    hardware_flash
)

target_include_directories(fruitchip-fw-bootloader
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../fruitchip-include
)

target_compile_definitions(fruitchip-fw-bootloader
    PUBLIC
        FW_PARTITION_OFFSET=${FW_PARTITION_OFFSET}
        FW_PARTITION_SIZE_BYTES=${FW_PARTITION_SIZE_BYTES}
        UPDATE_PARTITION_OFFSET=${UPDATE_PARTITION_OFFSET}
)

if (DEFINED PICO_DEFAULT_WS2812_PIN)
    target_compile_definitions(fruitchip-fw-bootloader
        PUBLIC
            PICO_DEFAULT_WS2812_PIN=${PICO_DEFAULT_WS2812_PIN}
    )
endif()

pico_enable_stdio_rtt(fruitchip-fw-bootloader 1)

if(DEFINED UART_TX_PIN AND DEFINED UART_RX_PIN)
    target_link_libraries(fruitchip-fw-bootloader
        pico_stdio_uart
    )

    target_compile_definitions(fruitchip-fw-bootloader
        PUBLIC
            PICO_DEFAULT_UART_TX_PIN=${UART_TX_PIN}
            PICO_DEFAULT_UART_RX_PIN=${UART_RX_PIN}
    )

    pico_enable_stdio_uart(fruitchip-fw-bootloader 1)
else()
    # UART is enabled by default
    pico_enable_stdio_uart(fruitchip-fw-bootloader 0)
endif()

pico_add_uf2_output(fruitchip-fw-bootloader)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/pico_flash_region.template.ld
    ${CMAKE_BINARY_DIR}/pico_flash_region.ld
)

include(../git_version.cmake)

configure_git_version_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/git_version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/git_version.h
)

set_property(TARGET fruitchip-fw-bootloader APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")
