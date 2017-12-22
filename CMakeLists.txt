cmake_minimum_required (VERSION 3.0)
project (vTAL)

 # For sweet release, But may face errors with Linux headers.
#add_definitions(-Wall -pedantic-errors -std=iso9899:1990 -s -o2)

add_definitions(-g -D__DEBUG__) # For debugging.

include_directories(./vTAL)

add_subdirectory(tests/windows-linux/HTAL)
add_subdirectory(tests/windows-linux/VTAL)

