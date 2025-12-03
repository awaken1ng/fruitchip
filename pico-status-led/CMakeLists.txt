pico_add_library(status_led)

target_sources(status_led INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/status_led.c
    ${CMAKE_CURRENT_LIST_DIR}/colored_status_led.c
)

target_include_directories(status_led_headers SYSTEM INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

pico_mirrored_target_link_libraries(status_led INTERFACE
    hardware_gpio
    hardware_pio
)

pico_generate_pio_header(status_led ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

get_target_property(OUT status_led LINK_LIBRARIES)
