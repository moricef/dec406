# libsgb406 — COSPAS-SARSAT SGB decoder, for Android NDK or desktop.
#   Android: add_subdirectory(<path>/lib/sgb406) from the app's CMakeLists
#   Desktop: cmake -B build && cmake --build build
cmake_minimum_required(VERSION 3.10)
project(sgb406 C)

# Standalone archive (src/ and include/ next to this file) or dec406 tree.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/dsss_demod.c)
    set(DEC406_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
else()
    set(DEC406_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
endif()

add_library(sgb406 STATIC
    sgb406.c
    ${DEC406_ROOT}/src/dsss_demod.c
    ${DEC406_ROOT}/src/rrc_filter.c
    ${DEC406_ROOT}/src/symbol_sync.c
    ${DEC406_ROOT}/src/costas4.c
    ${DEC406_ROOT}/src/despread.c
    ${DEC406_ROOT}/src/freq_acq.c
    ${DEC406_ROOT}/src/dec406_v2g.c
    ${DEC406_ROOT}/src/display_utils.c
)
target_include_directories(sgb406
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${DEC406_ROOT}/include)
set_target_properties(sgb406 PROPERTIES C_STANDARD 11 POSITION_INDEPENDENT_CODE ON)
set_source_files_properties(
    ${DEC406_ROOT}/src/dec406_v2g.c
    ${DEC406_ROOT}/src/display_utils.c
    PROPERTIES COMPILE_OPTIONS "-include;${CMAKE_CURRENT_SOURCE_DIR}/sgb406_printf.h")
find_library(MATH_LIB m)
if(MATH_LIB)
    target_link_libraries(sgb406 PUBLIC ${MATH_LIB})
endif()

if(NOT ANDROID)
    add_executable(sgb406_test sgb406_test.c)
    target_link_libraries(sgb406_test sgb406)
endif()
