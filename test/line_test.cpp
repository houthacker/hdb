#include "gtest/gtest.h"

extern "C" {
#include <line.h>
}

class HdbLineArrayFixture : public ::testing::Test {
protected:
    LineArray* lines;

    virtual void SetUp() {
        lines = (LineArray *)malloc(sizeof(LineArray));
        initLineArray(lines);
    }

    virtual void TearDown() {
        freeLineArray(lines);
        free((void*)lines);
    }
};

TEST_F(HdbLineArrayFixture, initializes_correctly) {
    EXPECT_EQ(lines->count, 0);
    EXPECT_EQ(lines->capacity, 0);
    EXPECT_EQ(lines->lines, nullptr);
}

TEST_F(HdbLineArrayFixture, encode_single_line) {
    int ln = 1337;

    int idx = encodeLine(lines, ln);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(lines->count, 1);
    EXPECT_EQ(lines->capacity, 8);

    Line line = lines->lines[0];
    EXPECT_EQ(line.line, 1337);
    EXPECT_EQ(line.instructionCount, 1);
}

TEST_F(HdbLineArrayFixture, encode_single_line_twice) {
    int ln = 1337;

    int idx = encodeLine(lines, ln);
    int idx2 = encodeLine(lines, ln);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(idx2, 0);
    EXPECT_EQ(lines->count, 1);
    EXPECT_EQ(lines->capacity, 8);

    Line line = lines->lines[0];
    EXPECT_EQ(line.line, 1337);
    EXPECT_EQ(line.instructionCount, 2);
}

TEST_F(HdbLineArrayFixture, encode_lines_non_ordered) {
    encodeLine(lines, 3);
    encodeLine(lines, 1);
    encodeLine(lines, 2);

    EXPECT_EQ(lines->count, 3);

    Line line1 = lines->lines[0];
    Line line2 = lines->lines[1];
    Line line3 = lines->lines[2];

    EXPECT_EQ(line1.line, 1);
    EXPECT_EQ(line1.instructionCount, 1);

    EXPECT_EQ(line2.line, 2);
    EXPECT_EQ(line2.instructionCount, 1);

    EXPECT_EQ(line3.line, 3);
    EXPECT_EQ(line3.instructionCount, 1);
}

TEST_F(HdbLineArrayFixture, encode_lines_force_grow) {
    for (int i = 0; i < 9; i++) {
        encodeLine(lines, i + 1);
    }

    EXPECT_EQ(lines->count, 9);
    EXPECT_EQ(lines->capacity, 16);
}

TEST_F(HdbLineArrayFixture, encode_lines_in_reverse_order) {
    for (int i = 9; i > 0; i--) {
        encodeLine(lines, i);
    }

    // encode a random line again, to cause one line with two instructions
    encodeLine(lines, 5);

    EXPECT_EQ(lines->count, 9);
    EXPECT_EQ(lines->capacity, 16);

    EXPECT_EQ(lines->lines[0].line, 1);
    EXPECT_EQ(lines->lines[8].line, 9);

    Line fifth = lines->lines[4];
    EXPECT_EQ(fifth.line, 5);
    EXPECT_EQ(fifth.instructionCount, 2);
}

TEST_F(HdbLineArrayFixture, decode_negative_index) {
    EXPECT_EQ(decodeLine(lines, -5), -1);
}

TEST_F(HdbLineArrayFixture, decode_non_existing_index) {
    EXPECT_EQ(decodeLine(lines, 0), -1);
}

TEST_F(HdbLineArrayFixture, decode_existing_index) {
    encodeLine(lines, 1);
    encodeLine(lines, 2);
    encodeLine(lines, 2); // 2 instructions on line 2
    encodeLine(lines, 3);
    encodeLine(lines, 5); // no instruction on line 4

    EXPECT_EQ(lines->count, 4);
    EXPECT_EQ(decodeLine(lines, 0), 1);
    EXPECT_EQ(decodeLine(lines, 1), 2);
    EXPECT_EQ(decodeLine(lines, 2), 2);
    EXPECT_EQ(decodeLine(lines, 3), 3);
    EXPECT_EQ(decodeLine(lines, 4), 5);
}