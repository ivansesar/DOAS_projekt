cmake_minimum_required(VERSION 3.31)
project(DirectoryChooser)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})
add_definitions(${GTKMM_CFLAGS_OTHER})

include(FetchContent)
FetchContent_Declare(
        zlib
        URL https://zlib.net/zlib-1.3.1.tar.gz
)
FetchContent_MakeAvailable(zlib)


add_executable(DirectoryChooser main.cpp Filters.cpp
        Filters.h
        Heuristics.cpp
        Heuristics.h
        CompressionResult.cpp
        CompressionResult.h
        PNGWriter.cpp
        PNGWriter.h
        BMPReader.cpp
        BMPReader.h
        ImageWidget.cpp
        ImageWidget.h
)

target_include_directories(DirectoryChooser PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(DirectoryChooser PRIVATE ${GTKMM_LIBRARY_DIRS})
target_compile_options(DirectoryChooser PRIVATE ${GTKMM_CFLAGS_OTHER})
target_link_libraries(DirectoryChooser PRIVATE ${GTKMM_LIBRARIES})

add_definitions(-DZLIB_STATIC)
target_link_libraries(DirectoryChooser PRIVATE zlibstatic)