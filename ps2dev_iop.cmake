#
# CMake platform file for PS2 IOP processor
#
# Copyright (C) 2009-2010 Mathias Lafeldt <misfire@debugon.org>
# Copyright (C) 2023 Francisco Javier Trujillo Mata <fjtrujy@gmail.com>
# Copyright (C) 2024 André Guilherme <andregui17@outlook.com>
# Copyright (C) 2024-Present PS2DEV Team
#

cmake_minimum_required(VERSION 3.0...3.12)

include(CMakeForceCompiler)
if(DEFINED ENV{PS2SDK})
    set(PS2SDK $ENV{PS2SDK})
else()
    message(FATAL_ERROR "The environment variable PS2SDK needs to be defined.")
endif()

if(DEFINED ENV{PS2DEV})
    set(PS2DEV $ENV{PS2DEV})
else()
    message(FATAL_ERROR "The environment variable PS2DEV needs to be defined.")
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR mips)
set(CMAKE_C_COMPILER mipsel-none-elf-gcc)
set(CMAKE_CXX_COMPILER mipsel-none-elf-g++)
set(CMAKE_C_COMPILER_WORKS 1) #Hack by f0bes
set(CMAKE_CXX_COMPILER_WORKS 1) #Hack by f0bes

#
# set arch flags depending on gcc version
#
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE IOP_CC_VERSION
)

set(IOP_ASFLAGS_TARGET "-mcpu=r3000")

include_directories($ENV{PS2SDK}/iop/include $ENV{PS2SDK}/common/include $ENV{PS2SDK}/ports_iop/include)

add_definitions(-D_IOP -DPS2 -D__PS2__)

set(IOP_OPTFLAGS "-Os")
set(IOP_WARNFLAGS "-Wall")
set(IOP_DBGINFOFLAGS "-gdwarf-2 -gz")

set(IOP_CFLAGS "-fno-builtin -G0 ${IOP_OPTFLAGS} ${IOP_WARNFLAGS} ${IOP_DBGINFOFLAGS} -msoft-float -mno-explicit-relocs ${IOP_CFLAGS_TARGET}" CACHE STRING "IOP C compiler flags" FORCE)
set(IOP_LDFLAGS "${IOP_LDFLAGS_TARGET} -T$ENV{PS2SDK}/iop/startup/linkfile -L$ENV{PS2SDK}/iop/lib -L$ENV{PS2SDK}/ports_iop/lib -nostdlib -dc -r" CACHE STRING "IOP linker flags" FORCE)
set(IOP_ASFLAGS "${IOP_ASFLAGS_TARGET} -EL -G0" CACHE STRING "IOP assembler flags" FORCE)

set(CMAKE_C_FLAGS_INIT ${IOP_CFLAGS})
set(CMAKE_CXX_FLAGS_INIT ${IOP_CFLAGS})
set(CMAKE_EXE_LINKER_FLAGS_INIT ${IOP_LDFLAGS})

set(CMAKE_TARGET_INSTALL_PREFIX $ENV{PS2DEV}/ports_iop)

set(CMAKE_FIND_ROOT_PATH $ENV{PS2DEV} $ENV{PS2DEV}/iop $ENV{PS2DEV}/iop/iop $ENV{PS2SDK} $ENV{PS2SDK}/ports_iop)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

function(build_iop_imports out_f in_f)
    # disable toplevel reordering by GCC v4.2 and later.
    # without it, the import and export tables can be broken apart by GCC's optimizations.
    set_source_files_properties(
        ${out_f}
        PROPERTIES COMPILE_OPTIONS "-fno-toplevel-reorder"
    )

    add_custom_command(
        OUTPUT ${out_f}
        COMMAND ${CMAKE_COMMAND} -E echo \#include \"irx_imports.h\" > ${out_f}
        COMMAND ${CMAKE_COMMAND} -E cat ${in_f} >> ${out_f}
        DEPENDS ${in_f}
        COMMENT "Creating ${out_f}" VERBATIM
    )
endfunction()

function(build_iop_exports out_f in_f)
    # disable toplevel reordering by GCC v4.2 and later.
    # without it, the import and export tables can be broken apart by GCC's optimizations.
    set_source_files_properties(
        ${out_f}
        PROPERTIES COMPILE_OPTIONS "-fno-toplevel-reorder"
    )

    add_custom_command(OUTPUT ${out_f}
        COMMAND ${CMAKE_COMMAND} -E echo \#include \"irx.h\" > ${out_f}
        COMMAND ${CMAKE_COMMAND} -E cat ${in_f} >> ${out_f}
        DEPENDS ${in_f}
        COMMENT "Creating ${out_f}" VERBATIM
    )
endfunction()

set(PLATFORM_PS2 TRUE)
set(IOP TRUE)
set(PS2 TRUE)
