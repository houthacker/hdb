#include "gtest/gtest.h"

extern "C" {
#include <memory.h>
#include <value.h>
}

class HdbValueFixture : public ::testing::Test {
protected:
    value_array_t * values;

    static void SetUpTestSuite() {
        hdb_init_heap(256, 512);
    }

    static void TearDownTestSuite() {
        hdb_destroy_heap();
    }

    virtual void SetUp() {
        values = (value_array_t *)hdb_malloc(sizeof(value_array_t));
        init_value_array(values);
    }

    virtual void TearDown() {
        free_value_array(values);
        hdb_free(values);
    }
};

TEST_F(HdbValueFixture, initializes_correctly) {
    EXPECT_EQ(values->count, 0);
    EXPECT_EQ(values->capacity, 0);
    EXPECT_EQ(values->values, nullptr);
}

TEST_F(HdbValueFixture, write_single_value) {
    write_value_array(values, 1.337);

    EXPECT_EQ(values->count, 1);
    EXPECT_EQ(values->capacity, 8);
    EXPECT_EQ(values->values[0], 1.337);
}

TEST_F(HdbValueFixture, force_value_array_grow) {
    for (int i = 0; i < 9; i++) {
        write_value_array(values, 1.0 * i);
    }

    EXPECT_EQ(values->count, 9);
    EXPECT_EQ(values->capacity, 16);

    for (int i = 0; i < values->count; i++) {
        EXPECT_EQ(values->values[i], 1.0 * i);
    }
}