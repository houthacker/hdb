#include "gtest/gtest.h"

extern "C" {
    #include <memory.h>
    #include <chunk.h>
}

class HdbChunkFixture : public ::testing::Test {
protected:
    hdb_chunk_t* chunk;

    static void SetUpTestSuite() {
        hdb_heap_init(256, 512);
    }

    static void TearDownTestSuite() {
        hdb_heap_free();
    }

    virtual void SetUp() {
        chunk = (hdb_chunk_t*)hdb_malloc(sizeof(hdb_chunk_t));
        hdb_chunk_init(chunk);
    }

    virtual void TearDown() {
        hdb_free(chunk);
    }
};

TEST_F(HdbChunkFixture, initializes_correctly) {
    EXPECT_EQ(chunk->count, 0);
}

TEST_F(HdbChunkFixture, increments_count_correctly) {
    hdb_chunk_write_constant(chunk, 1.2, 123);
    EXPECT_EQ(chunk->count, 2);
    EXPECT_EQ(chunk->constants.count, 1);
    EXPECT_EQ(chunk->constants.values[0], 1.2);

    hdb_chunk_write(chunk, OP_RETURN, 123);
    EXPECT_EQ(chunk->count, 3);
    EXPECT_EQ(chunk->capacity, 8);
}

TEST_F(HdbChunkFixture, write_large_constant) {
    for (int i = 0; i < 257; i++) {
        hdb_chunk_write_constant(chunk, 1.23, i + 1);
    }

    EXPECT_EQ(chunk->count, 516);
    EXPECT_EQ(chunk->constants.count, 257);
    EXPECT_EQ(chunk->constants.values[0], 1.23);
    EXPECT_EQ(chunk->constants.values[256], 1.23);

}

