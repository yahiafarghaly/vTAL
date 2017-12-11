cmake_minimum_required (VERSION 3.0)
project (vTAL)

SET(execName TAL-Test)

SET(vTAL_SRC vTAL/vTAL-src/vTAL.c)

FILE(GLOB_RECURSE ARCH_SRC_FILES "vTAL/vTAL-src/arch/*/*.c")

add_executable(${execName} ${vTAL_SRC} ${ARCH_SRC_FILES} main.c)
 
 # For sweet release, But may face errors with Linux headers.
#add_definitions(-Wall -pedantic-errors -std=iso9899:1990 -s -o2)

add_definitions(-g -D__DEBUG__) # For debugging.

set(EXECUTABLE_OUTPUT_PATH "../")

include_directories(./vTAL)


