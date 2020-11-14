#include "gtest/gtest.h"

extern "C" {
#include <memory.h>
#include "hdb_tests.h"
}

class HdbMemoryFixture : public ::testing::Test {
protected:
    hdb_heap_view_t* heap;

    virtual void SetUp() {
        heap = hdb_heap_init(256, 2048);
    }

    virtual void TearDown() {
        hdb_heap_free();
    }
};

TEST_F(HdbMemoryFixture, hdb_init_heap_illegal_parameter_order) {
    hdb_heap_free();

    EXPECT_EQ(hdb_heap_init(HDB_HEAP_INITIAL_MIN_SIZE, HDB_HEAP_INITIAL_MIN_SIZE - 1), nullptr);
    EXPECT_EQ(errno, EINVAL);
}

TEST_F(HdbMemoryFixture, hdb_init_heap_min_size_too_small) {
    hdb_heap_free();

    EXPECT_EQ(hdb_heap_init(0, HDB_HEAP_INITIAL_MIN_SIZE), nullptr);
    EXPECT_EQ(errno, EINVAL);
}

TEST_F(HdbMemoryFixture, hdb_heap_states) {
    EXPECT_NE(hdb_heap(), nullptr);

    hdb_heap_free();
    EXPECT_EQ(hdb_heap(), nullptr);
}

TEST_F(HdbMemoryFixture, hdb_malloc_without_initialized_heap) {
    hdb_heap_free();

    void* ptr = hdb_malloc(HDB_HEAP_PAGE_SIZE);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(HdbMemoryFixture, hdb_malloc_zero_bytes) {
    void* ptr = hdb_malloc(0);
    EXPECT_EQ(ptr, nullptr);
}

TEST_F(HdbMemoryFixture, hdb_malloc_single_byte) {
    void* ptr = hdb_malloc(1);
    auto block = HDB_CPP_BLOCK_PTR(ptr);

    // block size is aligned to next power of two of (size + sizeof(block_t))
    EXPECT_EQ(block->size, 32);

    hdb_free(ptr);
}

TEST_F(HdbMemoryFixture, hdb_malloc_max) {
    void* ptr = hdb_malloc(heap->current_free - HDB_HEAP_PAGE_SIZE);
    auto block = HDB_CPP_BLOCK_PTR(ptr);

    EXPECT_EQ(block->size, heap->current_size);
    EXPECT_EQ(heap->current_free, 0);
    EXPECT_EQ(heap->free_blocks, nullptr);

    size_t expected = block->size;
    hdb_free(ptr);

    EXPECT_EQ(heap->current_free, expected);
}

TEST_F(HdbMemoryFixture, hdb_malloc_decreasing_size) {
    for (int i = 5; i > 0; i--) {
        size_t sz = HDB_HEAP_PAGE_SIZE * i;
        void* ptr = hdb_malloc(sz);
        auto block = HDB_CPP_BLOCK_PTR(ptr);

        EXPECT_GE(block->size, sz);
        EXPECT_LE(heap->current_free, heap->current_size - sz);

        hdb_free(ptr);
    }

    EXPECT_EQ(heap->current_free, heap->current_size);
}

TEST_F(HdbMemoryFixture, hdb_malloc_increasing_size) {
    for (int i = 1; i <= 5; i++) {
        EXPECT_EQ(heap->current_size, heap->current_free);

        size_t sz = HDB_HEAP_PAGE_SIZE * i;
        void* ptr = hdb_malloc(sz);
        auto block = HDB_CPP_BLOCK_PTR(ptr);

        EXPECT_GE(block->size, sz);
        EXPECT_LE(heap->current_free, heap->current_size - sz);

        hdb_free(ptr);
    }

    EXPECT_EQ(heap->current_free, heap->current_size);
}

TEST_F(HdbMemoryFixture, hdb_malloc_free_block_fragmentation) {
    // min_size = 256, max_size = 2048

    const size_t max = 256;
    void *array[max];
    memset(array, 0, max);

    for (auto & i : array) {
        void* ptr = hdb_malloc(1);
        auto block = HDB_CPP_BLOCK_PTR(ptr);

        EXPECT_EQ(block->size, 32);

        i = ptr;
    }

    for (auto & i : array) {
        hdb_free(i);
    }
    EXPECT_EQ(heap->current_size, heap->current_free);

    // There should be 'max' blocks of 32 bytes, which is the amount of bytes that must be allocated when
    // the user requests one byte of memory. The remaining block must fill the rest of the free space.
    hdb_memory_block_view_t* current_block = heap->free_blocks;
    for (int i = 0; i < max; i++) {
        EXPECT_EQ(current_block->size, 32);

        current_block = current_block->next;
    }

    EXPECT_EQ(current_block->size, heap->current_size - (max * 32));
    EXPECT_EQ(current_block->next, nullptr);

    // hdb_heap_compact should merge all blocks in a single block, since the space is contiguous.
    hdb_heap_compact();
    EXPECT_NE(heap->free_blocks, nullptr);
    EXPECT_EQ(heap->free_blocks->next, nullptr);
    EXPECT_EQ(heap->free_blocks->size, heap->current_free);
    EXPECT_EQ(heap->current_free, heap->current_size);
}