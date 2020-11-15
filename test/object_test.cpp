#include <mutex>
#include "gtest/gtest.h"

extern "C" {
#include <vm.h>
#include <object.h>
#include <os.h>
}

class HdbObjectFixture : public ::testing::Test {
protected:
    char* os_chars;

    static void SetUpTestSuite() {
        hdb_vm_init(256, 512);
    }

    static void TearDownTestSuite() {
        hdb_vm_free();
    }

    virtual void SetUp() {
        os_chars = nullptr;
    }

    virtual void TearDown() {
        if (os_chars != nullptr) {
            os_free(os_chars);
        }
    }
};

TEST_F(HdbObjectFixture, hdb_object_take_string_null) {
    hdb_string_t* string = hdb_object_take_string(nullptr, 0);

    EXPECT_EQ(string, nullptr);
}

TEST_F(HdbObjectFixture, hdb_object_take_string_null_with_length) {
    hdb_string_t* string = hdb_object_take_string(nullptr, 1);

    EXPECT_EQ(string, nullptr);
}

TEST_F(HdbObjectFixture, hdb_object_take_string) {
    const char* original = "test_string";

    os_chars = (char*) os_malloc(strlen(original) + 1);
    strcpy(os_chars, original);

    hdb_string_t* string = hdb_object_take_string(os_chars, strlen(os_chars));

    EXPECT_NE(string, nullptr);
    EXPECT_EQ(string->length, strlen(os_chars));
    EXPECT_EQ(string->chars, os_chars);
    EXPECT_EQ(string->obj.type, OBJ_STRING);
}

TEST_F(HdbObjectFixture, hdb_object_copy_string_null) {
    hdb_string_t* string = hdb_object_copy_string(nullptr, 0);

    EXPECT_EQ(string, nullptr);
}

TEST_F(HdbObjectFixture, hdb_object_copy_string_null_with_length) {
    hdb_string_t* string = hdb_object_copy_string(nullptr, 1);

    EXPECT_EQ(string, nullptr);
}

TEST_F(HdbObjectFixture, hdb_object_copy_string) {
    const char* original = "test_string";

    hdb_string_t* string = hdb_object_copy_string(original, strlen(original));

    EXPECT_NE(string, nullptr);
    EXPECT_EQ(string->length, strlen(original));
    EXPECT_NE(string->chars, original);
    EXPECT_EQ(string->obj.type, OBJ_STRING);
    EXPECT_STREQ(original, string->chars);
}