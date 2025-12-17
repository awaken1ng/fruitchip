cmake_minimum_required(VERSION 3.13)

include(FetchContent)
include(../fruitchip-fw-board/board_configuration.cmake)

project(fruitchip-menu)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(IS_RELEASE_BUILD CMAKE_BUILD_TYPE MATCHES "^Release")

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_executable(fruitchip-menu
    ${CMAKE_CURRENT_BINARY_DIR}/include/font/dejavu_sans_mono.h

    src/components/button_guide.c
    src/components/font.c
    src/components/list.c
    src/components/progress_bar.c

    src/input/pad.c

    src/scene/settings/update/checking.c
    src/scene/settings/update/complete.c
    src/scene/settings/update/found.c
    src/scene/settings/update/not_found.c
    src/scene/settings/update/rebooting.c
    src/scene/settings/update/scanning.c
    src/scene/settings/update/writing.c
    src/scene/boot_list.c
    src/scene/boot_options.c
    src/scene/init.c
    src/scene/message.c
    src/scene/settings_about.c
    src/scene/settings.c
    src/scene/superscene.c

    src/apps.c
    ${CMAKE_CURRENT_BINARY_DIR}/src/embedded_irx.c
    src/init.c
    src/main.c
    src/pin_config.c
    src/update.c
    src/utils.c
    src/version.c
)

target_compile_definitions(fruitchip-menu
    PUBLIC
        FW_PARTITION_OFFSET=${FW_PARTITION_OFFSET}
)

target_include_directories(fruitchip-menu
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../fruitchip-include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/masswatch/ee/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../pico-sdk/src/common/pico_base_headers/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../pico-sdk/src/common/pico_binary_info/include
)

target_link_libraries(fruitchip-menu
    patches
    gskit_toolkit
    gskit
    dmakit
    png
    z
    tiff
    pad
    ${MASSWATCH_INSTALL_PREFIX}/${CMAKE_SHARED_LIBRARY_PREFIX}masswatch-ee.a
)

if (${IS_RELEASE_BUILD})
    target_link_libraries(fruitchip-menu elf-loader-nocolour)
else()
    target_link_libraries(fruitchip-menu elf-loader)
endif()

target_compile_options(fruitchip-menu
    PUBLIC
        -O3
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/embedded_irx.c.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/embedded_irx.c
)

# region: font

FetchContent_Declare(
    dejavu-sans-mono
    URL "https://github.com/dejavu-fonts/dejavu-fonts/releases/download/version_2_37/dejavu-sans-ttf-2.37.zip"
    URL_HASH SHA256=5c6e497a2f36552cb5ffb112c413a6af39c0f3c47653662b90b4fa6499822fd7
    DOWNLOAD_EXTRACT_TIMESTAMP false
)

FetchContent_Declare(
    acbmfont
    URL "https://angelcode.com/products/bmfont/bmfont64_1.14b_beta.zip"
    URL_HASH SHA256=3ae24f8bafa9e350f9580c93bd43a68128bf2c3d8083c839c954667067bbf4c7
    DOWNLOAD_EXTRACT_TIMESTAMP false
)

# extract to ${CMAKE_CURRENT_BINARY_DIR}/_deps/acbmfont-src
#            ${CMAKE_CURRENT_BINARY_DIR}/_deps/dejavu-sans-mono-src
FetchContent_MakeAvailable(dejavu-sans-mono acbmfont)

# https://angelcode.com/products/bmfont/doc/command_line.html
set(ACBMFONT "${CMAKE_CURRENT_BINARY_DIR}/_deps/acbmfont-src/bmfont64.exe")

if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    find_program(WINE wine REQUIRED)
    list(PREPEND ACBMFONT "wine")
endif()

list(APPEND ACBMFONT "-c") # Names the configuration file with the options for generating the font.
list(APPEND ACBMFONT "res/dejavu-sans-mono-19px-bold-aa.bmfc")
list(APPEND ACBMFONT "-o") # Names of the output font file.
list(APPEND ACBMFONT "${CMAKE_CURRENT_BINARY_DIR}/res/dejavu-sans-mono-19px-bold-aa.fnt")

# bmfont does not create subdirectories in the output path if they don't eixst
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/res")

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/res/dejavu-sans-mono-19px-bold-aa.fnt
        ${CMAKE_CURRENT_BINARY_DIR}/res/dejavu-sans-mono-19px-bold-aa_0.png
    COMMAND
        ${ACBMFONT}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/res/dejavu-sans-mono-19px-bold-aa.bmfc
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/include/font/dejavu_sans_mono.h
    COMMAND
        python
        tools/convert_bmfont.py
        ${CMAKE_CURRENT_BINARY_DIR}/res/dejavu-sans-mono-19px-bold-aa.fnt # fnt
        ${CMAKE_CURRENT_BINARY_DIR}/res                                   # embed_path_root
        ${CMAKE_CURRENT_BINARY_DIR}/include/font/dejavu_sans_mono.h       # out_path
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/res/dejavu-sans-mono-19px-bold-aa.fnt
        ${CMAKE_CURRENT_BINARY_DIR}/res/dejavu-sans-mono-19px-bold-aa_0.png
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# endregion: font

include(../git_version.cmake)

configure_git_version_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/git_version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/git_version.h
)

add_custom_command(
    TARGET fruitchip-menu
    POST_BUILD
    COMMAND
        ps2-packer
        $<TARGET_FILE:fruitchip-menu>
        $<TARGET_FILE_BASE_NAME:fruitchip-menu>-packed.elf
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

install(
    FILES $<TARGET_FILE_BASE_NAME:fruitchip-menu>-packed.elf
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
