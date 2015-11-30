cmake_minimum_required(VERSION 2.8.8)
project(aobaker)

exec_program(brew
    ARGS "info embree | tr ' ' '\n' | grep Cellar"
    OUTPUT_VARIABLE EMBREE_PATH)

exec_program(brew
    ARGS "info tbb | tr ' ' '\n' | grep Cellar"
    OUTPUT_VARIABLE TBB_PATH)

message(EMBREE_PATH = ${EMBREE_PATH})
message(TBB_PATH = ${TBB_PATH})

add_subdirectory(thekla)
add_subdirectory(vendor/poshlib)

add_definitions(
    -O3
    -std=c++11
    -I${EMBREE_PATH}/include
    -I${TBB_PATH}/include
    -Wall)

link_directories(
    ${EMBREE_PATH}/lib
    ${TBB_PATH}/lib)

include_directories(
    thekla/thekla
    vendor/stb
    vendor/tinyobj)

add_executable(
    aobaker
    raytrace.cpp
    aobaker.cpp)

target_link_libraries(
    aobaker
    thekla_atlas
    embree
    tbb)
