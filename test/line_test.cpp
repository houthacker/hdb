#include "gtest/gtest.h"

extern "C" {
#include <memory.h>
#include <line.h>
}

class HdbLineArrayFixture : public ::testing::Test {
protected:
    line_array_t* lines;

    static void SetUpTestSuite() {
        hdb_init_heap(256, 512);
    }

    static void TearDownTestSuite() {
        hdb_destroy_heap();
    }

    virtual void SetUp() {
        lines = (line_array_t *)hdb_malloc(sizeof(line_array_t));
        init_line_array(lines);
    }

    virtual void TearDown() {
        free_line_array(lines);
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

    int idx = encode_line(lines, ln);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(lines->count, 1);
    EXPECT_EQ(lines->capacity, 8);

    line_t line = lines->lines[0];
    EXPECT_EQ(line.line, 1337);
    EXPECT_EQ(line.instructionCount, 1);
}

TEST_F(HdbLineArrayFixture, encode_single_line_twice) {
    int ln = 1337;

    int idx = encode_line(lines, ln);
    int idx2 = encode_line(lines, ln);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(idx2, 0);
    EXPECT_EQ(lines->count, 1);
    EXPECT_EQ(lines->capacity, 8);

    line_t line = lines->lines[0];
    EXPECT_EQ(line.line, 1337);
    EXPECT_EQ(line.instructionCount, 2);
}

TEST_F(HdbLineArrayFixture, encode_lines_non_ordered) {
    encode_line(lines, 3);
    encode_line(lines, 1);
    encode_line(lines, 2);

    EXPECT_EQ(lines->count, 3);

    line_t line1 = lines->lines[0];
    line_t line2 = lines->lines[1];
    line_t line3 = lines->lines[2];

    EXPECT_EQ(line1.line, 1);
    EXPECT_EQ(line1.instructionCount, 1);

    EXPECT_EQ(line2.line, 2);
    EXPECT_EQ(line2.instructionCount, 1);

    EXPECT_EQ(line3.line, 3);
    EXPECT_EQ(line3.instructionCount, 1);
}

TEST_F(HdbLineArrayFixture, encode_lines_force_grow) {
    for (int i = 0; i < 9; i++) {
        encode_line(lines, i + 1);
    }

    EXPECT_EQ(lines->count, 9);
    EXPECT_EQ(lines->capacity, 16);
}

TEST_F(HdbLineArrayFixture, encode_lines_in_reverse_order) {
    for (int i = 9; i > 0; i--) {
        encode_line(lines, i);
    }

    // encode a random line again, to cause one line with two instructions
    encode_line(lines, 5);

    EXPECT_EQ(lines->count, 9);
    EXPECT_EQ(lines->capacity, 16);

    EXPECT_EQ(lines->lines[0].line, 1);
    EXPECT_EQ(lines->lines[8].line, 9);

    line_t fifth = lines->lines[4];
    EXPECT_EQ(fifth.line, 5);
    EXPECT_EQ(fifth.instructionCount, 2);
}

TEST_F(HdbLineArrayFixture, decode_negative_index) {
    EXPECT_EQ(decode_line(lines, -5), -1);
}

TEST_F(HdbLineArrayFixture, decode_non_existing_index) {
    EXPECT_EQ(decode_line(lines, 0), -1);
}

TEST_F(HdbLineArrayFixture, decode_existing_index) {
    encode_line(lines, 1);
    encode_line(lines, 2);
    encode_line(lines, 2); // 2 instructions on line 2
    encode_line(lines, 3);
    encode_line(lines, 5); // no instruction on line 4

    EXPECT_EQ(lines->count, 4);
    EXPECT_EQ(decode_line(lines, 0), 1);
    EXPECT_EQ(decode_line(lines, 1), 2);
    EXPECT_EQ(decode_line(lines, 2), 2);
    EXPECT_EQ(decode_line(lines, 3), 3);
    EXPECT_EQ(decode_line(lines, 4), 5);
}