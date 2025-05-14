cmake_minimum_required(VERSION 3.30)
project(DOAS_projekt)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        zlib
        URL https://zlib.net/zlib-1.3.1.tar.gz
)
FetchContent_MakeAvailable(zlib)

add_executable(DOAS_projekt main.cpp
        Filters.cpp
        Filters.h
        Heuristics.cpp
        Heuristics.h
        CompressionResult.cpp
        CompressionResult.h
)
add_definitions(-DZLIB_STATIC)
target_link_libraries(DOAS_projekt PRIVATE zlibstatic)