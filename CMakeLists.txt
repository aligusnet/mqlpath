cmake_minimum_required(VERSION 3.24)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("cpp-starter")

find_package(Catch2 REQUIRED)
find_package(Bison REQUIRED)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# Include sub-proj§ects.
add_subdirectory("src")
