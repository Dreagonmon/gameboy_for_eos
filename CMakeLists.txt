cmake_minimum_required(VERSION 3.16)

include(ExternalProject)

set(CMAKE_TOOLCHAIN_FILE toolchain.cmake)
project(ExistOS VERSION 0.2.0)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

add_subdirectory(src)
