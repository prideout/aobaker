cmake_minimum_required(VERSION 2.8.8)
project(aobaker)

if(APPLE)
    exec_program(brew
        ARGS "info embree | tr ' ' '\n' | grep Cellar"
        OUTPUT_VARIABLE EMBREE_PATH)

    exec_program(brew
        ARGS "info tbb | tr ' ' '\n' | grep Cellar"
        OUTPUT_VARIABLE TBB_PATH)
else()
    # TODO: Find packages
    set(EMBREE_PATH "/usr/local" CACHE PATH "Embree location")
    set(TBB_PATH "/usr" CACHE PATH "Threading Building Blocks location")
endif()

message(EMBREE_PATH = ${EMBREE_PATH})
message(TBB_PATH = ${TBB_PATH})

add_subdirectory(thekla)
add_subdirectory(vendor/poshlib)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -std=c++11")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
endif()

add_definitions(
    -O3
    -I${EMBREE_PATH}/include
    -I${TBB_PATH}/include
    -Wall)

link_directories(
    ${EMBREE_PATH}/lib
    ${TBB_PATH}/lib)

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
    embree
    tbb)
