
SET(vTAL_SRC ../../../vTAL/vTAL-src/vTAL.c)

FILE(GLOB_RECURSE ARCH_SRC_FILES "../../../vTAL/vTAL-src/arch/*/*.c")

add_executable(HTAL_Test_0 ${vTAL_SRC} ${ARCH_SRC_FILES} HTAL_Test_0.c)