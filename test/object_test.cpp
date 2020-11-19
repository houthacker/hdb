#include <mutex>
#include "gtest/gtest.h"

extern "C" {
#include <vm.h>
#include <object.h>
#include <os.h>
}

class HdbObjectFixture : public ::testing::Test {
protected:

    static void SetUpTestSuite() {
        hdb_vm_init(256, 512);
    }

    static void TearDownTestSuite() {
        hdb_vm_free();
    }
};

TEST_F(HdbObjectFixture, hdb_object_create_string_of_zero_chars) {
    hdb_string_t* string = hdb_object_create_string(0);

    EXPECT_NE(string, nullptr);
    EXPECT_EQ(string->obj.type, OBJ_STRING);
    EXPECT_EQ(string->length, 0);
}

TEST_F(HdbObjectFixture, hdb_object_create_string) {
    hdb_string_t* string = hdb_object_create_string(10);

    EXPECT_NE(string, nullptr);
    EXPECT_EQ(string->obj.type, OBJ_STRING);
    EXPECT_EQ(string->length, 10);
}

TEST_F(HdbObjectFixture, hdb_object_copy_string) {
    const char* original = "test_string";

    hdb_string_t* string = hdb_object_copy_string(original, strlen(original));

    EXPECT_NE(string, nullptr);
    EXPECT_EQ(string->obj.type, OBJ_STRING);
    EXPECT_EQ(string->length, strlen(original));
    EXPECT_STREQ(string->chars, original);
}

TEST_F(HdbObjectFixture, hdb_object_copy_string_null) {
    hdb_string_t* string = hdb_object_copy_string(nullptr, 0);

    EXPECT_EQ(string, nullptr);
}

TEST_F(HdbObjectFixture, hdb_object_copy_string_null_with_length) {
    hdb_string_t* string = hdb_object_copy_string(nullptr, 1);

    EXPECT_EQ(string, nullptr);
}