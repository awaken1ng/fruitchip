cmake_minimum_required(VERSION 3.13)

project(fruitchip-loader-ee-stage2)

if(NOT DEFINED EE_STAGE_2_ADDR)
    message(FATAL_ERROR "EE_STAGE_2_ADDR is not set")
endif()

if(NOT DEFINED EE_STAGE_3_ADDR)
    message(FATAL_ERROR "EE_STAGE_3_ADDR is not set")
endif()

add_executable(fruitchip-loader-ee-stage2
    src/main.c
)

target_include_directories(fruitchip-loader-ee-stage2
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../fruitchip-include
)

target_compile_definitions(fruitchip-loader-ee-stage2
    PUBLIC
        EE_STAGE_3_ADDR=${EE_STAGE_3_ADDR}
)

target_compile_options(fruitchip-loader-ee-stage2
    PUBLIC
        -Os
)

target_link_options(fruitchip-loader-ee-stage2
    PUBLIC
        -nostartfiles
        -Wl,--gc-sections
        -s  # strip
)

# region: linkfile

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/linkfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/linkfile
)

set_target_properties(fruitchip-loader-ee-stage2
    PROPERTIES
        LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/linkfile
)

# remove default linkfile
separate_arguments(LINKER_FLAGS UNIX_COMMAND ${CMAKE_EXE_LINKER_FLAGS})
list(REMOVE_ITEM LINKER_FLAGS "-T$ENV{PS2SDK}/ee/startup/linkfile")
list(JOIN LINKER_FLAGS " " CMAKE_EXE_LINKER_FLAGS)

target_link_options(fruitchip-loader-ee-stage2
    PUBLIC
        -T${CMAKE_CURRENT_BINARY_DIR}/linkfile
)

# endregion: linkfile

add_custom_command(
    TARGET fruitchip-loader-ee-stage2
    POST_BUILD
    COMMAND
        ${CMAKE_OBJCOPY}
        -O binary
        -v
        $<TARGET_FILE:fruitchip-loader-ee-stage2>
        $<TARGET_FILE_BASE_NAME:fruitchip-loader-ee-stage2>.bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(
    FILES $<TARGET_FILE:fruitchip-loader-ee-stage2>.bin
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
