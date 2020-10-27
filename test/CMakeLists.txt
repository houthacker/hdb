project(hdb_tests)

add_subdirectory(lib)
include_directories(${PROJECT_SOURCE_DIR}/include ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(hdb_tests chunk_test.cpp line_test.cpp value_test.cpp memory_test.cpp)
target_link_libraries(hdb_tests src)
target_link_libraries(hdb_tests gtest gtest_main)
