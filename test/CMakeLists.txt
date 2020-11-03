project(hdb_tests)

add_subdirectory(lib)
include_directories(${PROJECT_SOURCE_DIR}/include ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} include)

add_executable(hdb_tests chunk_test.cpp line_test.cpp value_test.cpp memory_test.cpp vm_test.cpp scanner_test.cpp test_main.cpp)

target_link_libraries(hdb_tests hdb_api gtest gtest_main)
