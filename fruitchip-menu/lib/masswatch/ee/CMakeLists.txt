cmake_minimum_required(VERSION 3.13)

project(masswatch-ee)

add_library(masswatch-ee
    STATIC
    src/masswatch.c
)

target_include_directories(masswatch-ee
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(masswatch-ee
    PUBLIC
        -O3
)

install(
    FILES $<TARGET_FILE:masswatch-ee>
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
