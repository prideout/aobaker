cmake_minimum_required(VERSION 2.8.8)
project(aobaker)

add_subdirectory(thekla)
add_subdirectory(vendor/poshlib)

include_directories(
    thekla/thekla
    vendor/tinyobj)

add_executable(
    aobaker
    aobaker.cpp)

target_link_libraries(
    aobaker
    thekla_atlas)
