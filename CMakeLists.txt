cmake_minimum_required(VERSION 3.14)
project(fastp VERSION 1.01 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-pthread -g -O3)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
include_directories(inc)

add_executable(fastp ${SRC_FILES})

find_package(Threads REQUIRED)
target_link_libraries(fastp PRIVATE 
    Threads::Threads 
    isal 
    deflate
)

option(BUILD_STATIC "Build static binary" OFF)
if(BUILD_STATIC)
    set_target_properties(fastp PROPERTIES LINK_FLAGS "-static")
endif()