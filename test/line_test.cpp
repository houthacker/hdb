#include "gtest/gtest.h"

extern "C" {
#include <memory.h>
#include <line.h>
}

class HdbLineArrayFixture : public ::testing::Test {
protected:
    hdb_line_array_t* lines;

    static void SetUpTestSuite() {
        hdb_heap_init(256, 512);
    }

    static void TearDownTestSuite() {
        hdb_heap_free();
    }

    virtual void SetUp() {
        lines = (hdb_line_array_t *)hdb_malloc(sizeof(hdb_line_array_t));
        hdb_line_array_init(lines);
    }

    virtual void TearDown() {
        hdb_line_array_free(lines);
        hdb_free((void*)lines);
    }
};

TEST_F(HdbLineArrayFixture, initializes_correctly) {
    EXPECT_EQ(lines->count, 0);
    EXPECT_EQ(lines->capacity, 0);
    EXPECT_EQ(lines->lines, nullptr);
}

TEST_F(HdbLineArrayFixture, encode_single_line) {
    int ln = 1337;

    int idx = hdb_line_encode(lines, ln);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(lines->count, 1);
    EXPECT_EQ(lines->capacity, 8);

    hdb_line_t line = lines->lines[0];
    EXPECT_EQ(line.line, 1337);
    EXPECT_EQ(line.instruction_count, 1);
}

TEST_F(HdbLineArrayFixture, encode_single_line_twice) {
    int ln = 1337;

    int idx = hdb_line_encode(lines, ln);
    int idx2 = hdb_line_encode(lines, ln);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(idx2, 0);
    EXPECT_EQ(lines->count, 1);
    EXPECT_EQ(lines->capacity, 8);

    hdb_line_t line = lines->lines[0];
    EXPECT_EQ(line.line, 1337);
    EXPECT_EQ(line.instruction_count, 2);
}

TEST_F(HdbLineArrayFixture, encode_lines_non_ordered) {
    hdb_line_encode(lines, 3);
    hdb_line_encode(lines, 1);
    hdb_line_encode(lines, 2);

    EXPECT_EQ(lines->count, 3);

    hdb_line_t line1 = lines->lines[0];
    hdb_line_t line2 = lines->lines[1];
    hdb_line_t line3 = lines->lines[2];

    EXPECT_EQ(line1.line, 1);
    EXPECT_EQ(line1.instruction_count, 1);

    EXPECT_EQ(line2.line, 2);
    EXPECT_EQ(line2.instruction_count, 1);

    EXPECT_EQ(line3.line, 3);
    EXPECT_EQ(line3.instruction_count, 1);
}

TEST_F(HdbLineArrayFixture, encode_lines_force_grow) {
    for (int i = 0; i < 9; i++) {
        hdb_line_encode(lines, i + 1);
    }

    EXPECT_EQ(lines->count, 9);
    EXPECT_EQ(lines->capacity, 16);
}

TEST_F(HdbLineArrayFixture, encode_lines_in_reverse_order) {
    for (int i = 9; i > 0; i--) {
        hdb_line_encode(lines, i);
    }

    // encode a random line again, to cause one line with two instructions
    hdb_line_encode(lines, 5);

    EXPECT_EQ(lines->count, 9);
    EXPECT_EQ(lines->capacity, 16);

    EXPECT_EQ(lines->lines[0].line, 1);
    EXPECT_EQ(lines->lines[8].line, 9);

    hdb_line_t fifth = lines->lines[4];
    EXPECT_EQ(fifth.line, 5);
    EXPECT_EQ(fifth.instruction_count, 2);
}

TEST_F(HdbLineArrayFixture, decode_negative_index) {
    EXPECT_EQ(hdb_line_decode(lines, -5), -1);
}

TEST_F(HdbLineArrayFixture, decode_non_existing_index) {
    EXPECT_EQ(hdb_line_decode(lines, 0), -1);
}

TEST_F(HdbLineArrayFixture, decode_existing_index) {
    hdb_line_encode(lines, 1);
    hdb_line_encode(lines, 2);
    hdb_line_encode(lines, 2); // 2 instructions on line 2
    hdb_line_encode(lines, 3);
    hdb_line_encode(lines, 5); // no instruction on line 4

    EXPECT_EQ(lines->count, 4);
    EXPECT_EQ(hdb_line_decode(lines, 0), 1);
    EXPECT_EQ(hdb_line_decode(lines, 1), 2);
    EXPECT_EQ(hdb_line_decode(lines, 2), 2);
    EXPECT_EQ(hdb_line_decode(lines, 3), 3);
    EXPECT_EQ(hdb_line_decode(lines, 4), 5);
}