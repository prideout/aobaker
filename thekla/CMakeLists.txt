cmake_minimum_required(VERSION 2.8.8)
project(thekla_atlas)

include_directories(
    .
    nvcore
    ../vendor/stb
    ../vendor/poshlib)

add_subdirectory(nvcore)
add_subdirectory(nvimage)
add_subdirectory(nvmath)
add_subdirectory(nvmesh)

add_library(
    thekla_atlas
    thekla/thekla_atlas.cpp)

target_link_libraries(
    thekla_atlas
    nvmesh)
