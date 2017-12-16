
SET(vTAL_SRC ../../vTAL/vTAL-src/vTAL.c)

FILE(GLOB_RECURSE ARCH_SRC_FILES "../../vTAL/vTAL-src/arch/*/*.c")

add_executable(VTAL_Test_0 ${vTAL_SRC} ${ARCH_SRC_FILES} VTAL_Test_0.c)
add_executable(VTAL_Test_1 ${vTAL_SRC} ${ARCH_SRC_FILES} VTAL_Test_1.c)
add_executable(VTAL_Test_2 ${vTAL_SRC} ${ARCH_SRC_FILES} VTAL_Test_2.c)
add_executable(VTAL_Test_3 ${vTAL_SRC} ${ARCH_SRC_FILES} VTAL_Test_3.c)