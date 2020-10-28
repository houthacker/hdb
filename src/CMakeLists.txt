project(hdb)

set(SOURCE_FILES line.c chunk.c memory.c value.c debug.c)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(hdb_api STATIC ${SOURCE_FILES})