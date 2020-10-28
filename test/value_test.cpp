#include "gtest/gtest.h"

extern "C" {
#include <memory.h>
#include <value.h>
}

class HdbValueFixture : public ::testing::Test {
protected:
    ValueArray * values;

    static void SetUpTestSuite() {
        hdb_init_heap(256, 512);
    }

    static void TearDownTestSuite() {
        hdb_destroy_heap();
    }

    virtual void SetUp() {
        values = (ValueArray *)hdb_malloc(sizeof(ValueArray));
        initValueArray(values);
    }

    virtual void TearDown() {
        freeValueArray(values);
        hdb_free(values);
    }
};

TEST_F(HdbValueFixture, initializes_correctly) {
    EXPECT_EQ(values->count, 0);
    EXPECT_EQ(values->capacity, 0);
    EXPECT_EQ(values->values, nullptr);
}

TEST_F(HdbValueFixture, write_single_value) {
    writeValueArray(values, 1.337);

    EXPECT_EQ(values->count, 1);
    EXPECT_EQ(values->capacity, 8);
    EXPECT_EQ(values->values[0], 1.337);
}

TEST_F(HdbValueFixture, force_value_array_grow) {
    for (int i = 0; i < 9; i++) {
        writeValueArray(values, 1.0 * i);
    }

    EXPECT_EQ(values->count, 9);
    EXPECT_EQ(values->capacity, 16);

    for (int i = 0; i < values->count; i++) {
        EXPECT_EQ(values->values[i], 1.0 * i);
    }
}