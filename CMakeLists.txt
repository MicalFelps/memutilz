cmake_minimum_required(VERSION 3.16)
project(memutilz LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM "x64")
else()
    set(PLATFORM "Win32")
endif()

set(SOLUTION_DIR ${CMAKE_SOURCE_DIR})
option(BUILD_AGENT "Build Agent.dll" OFF)

if(BUILD_AGENT)
    add_subdirectory(agent)
endif()

# Projects
add_subdirectory(memutilz)