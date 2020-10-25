#include "gtest/gtest.h"

extern "C" {
    #include <chunk.h>
}

class HdbChunkFixture : public ::testing::Test {
protected:
    Chunk* chunk;

    virtual void SetUp() {
        chunk = (Chunk*)malloc(sizeof(Chunk));
        initChunk(chunk);
    }

    virtual void TearDown() {
        freeChunk(chunk);
        free((void*)chunk);
    }
};

TEST_F(HdbChunkFixture, initializes_correctly) {
    EXPECT_EQ(chunk->count, 0);
}

TEST_F(HdbChunkFixture, increments_count_correctly) {
    writeConstant(chunk, 1.2, 123);
    EXPECT_EQ(chunk->count, 2);
    EXPECT_EQ(chunk->constants.count, 1);
    EXPECT_EQ(chunk->constants.values[0], 1.2);

    writeChunk(chunk, OP_RETURN, 123);
    EXPECT_EQ(chunk->count, 3);
    EXPECT_EQ(chunk->capacity, 8);
}

TEST_F(HdbChunkFixture, write_large_constant) {
    for (int i = 0; i < 257; i++) {
        writeConstant(chunk, 1.23, i + 1);
    }

    EXPECT_EQ(chunk->count, 516);
    EXPECT_EQ(chunk->constants.count, 257);
    EXPECT_EQ(chunk->constants.values[0], 1.23);
    EXPECT_EQ(chunk->constants.values[256], 1.23);

}

