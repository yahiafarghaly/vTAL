cmake_minimum_required (VERSION 3.0)
project (vTAL)

SET(execName TAL-Test)

SET(vTAL_SRC vTAL/vTAL-src/vTAL.c)

FILE(GLOB_RECURSE ARCH_SRC_FILES "vTAL/vTAL-src/arch/*/*.c")

add_executable(${execName} ${vTAL_SRC} ${ARCH_SRC_FILES} main.c)

#add_definitions(-Wall -pedantic-errors -std=iso9899:1990 -s -o2) # For sweet release

add_definitions(-g) # For debugging.

set(EXECUTABLE_OUTPUT_PATH "../")

include_directories(./vTAL)


