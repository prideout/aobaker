cmake_minimum_required(VERSION 2.8.8)
project(aobaker)

exec_program(brew
    ARGS "info embree | tr ' ' '\n' | grep Cellar"
    OUTPUT_VARIABLE EMBREE_PATH)

message(EMBREE_PATH = ${EMBREE_PATH})

add_subdirectory(thekla)
add_subdirectory(vendor/poshlib)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -std=c++11")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_definitions(
    -O3
    -I${EMBREE_PATH}/include
    -Wall)

link_directories(${EMBREE_PATH}/lib)

include_directories(
    thekla/thekla
    vendor/flag
    vendor/stb
    vendor/tinyobj)

add_executable(
    aobaker
    cmdline.c
    raytrace.cpp
    aobaker.cpp)

target_link_libraries(
    aobaker
    thekla_atlas
    embree)
