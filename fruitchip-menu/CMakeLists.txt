cmake_minimum_required(VERSION 3.13)

include(FetchContent)

project(fruitchip-menu)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(IS_RELEASE_BUILD CMAKE_BUILD_TYPE MATCHES "^Release")

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_executable(fruitchip-menu
    src/components/button_guide.c
    src/components/font.c
    src/components/list.c

    src/input/pad.c

    src/scene/boot_list.c
    src/scene/boot_options.c
    src/scene/init.c
    src/scene/message.c
    src/scene/settings_about_update_apps.c
    src/scene/settings_about_update_fw.c
    src/scene/settings_about.c
    src/scene/settings.c
    src/scene/superscene.c

    src/apps.c
    ${CMAKE_CURRENT_BINARY_DIR}/src/embedded_irx.c
    src/main.c
    src/update.c
    src/utils.c

    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)

target_include_directories(fruitchip-menu
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../fruitchip-include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
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
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    font-dejavu-sans-mono
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/res/dejavu-sans-mono-19px-bold-aa.fnt
        ${CMAKE_CURRENT_BINARY_DIR}/res/dejavu-sans-mono-19px-bold-aa_0.png
    COMMAND
        python
        tools/convert_bmfont.py
        ${CMAKE_CURRENT_BINARY_DIR}/res/dejavu-sans-mono-19px-bold-aa.fnt # fnt
        ${CMAKE_CURRENT_BINARY_DIR}/res                                   # embed_path_root
        ${CMAKE_CURRENT_BINARY_DIR}/include/font/dejavu_sans_mono.h       # out_path
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_dependencies(fruitchip-menu font-dejavu-sans-mono)

# endregion: font

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/_version.cpp  # always re-run before the build
    COMMAND
        ${CMAKE_COMMAND} -P
        ${CMAKE_CURRENT_SOURCE_DIR}/../version.cmake
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
