project(hdb)

set(SOURCE_FILES os.c memory.c line.c chunk.c value.c vm.c debug.c compiler.c scanner.c object.c)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(hdb_api STATIC ${SOURCE_FILES})