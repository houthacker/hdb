cmake_minimum_required(VERSION 3.16)
project(hdb)

set(CMAKE_C_STANDARD 99)
set(SOURCE_FILES main.c)
add_executable(hdb ${SOURCE_FILES})

include_directories(include)
add_subdirectory(src)

target_link_libraries(hdb src)

add_subdirectory(test)