#include <mutex>
#include "gtest/gtest.h"

extern "C" {
#include <vm.h>
#include <ustring.h>
#include <os.h>
}

class HdbUStringFixture : public ::testing::Test {
protected:

    static void SetUpTestSuite() {
        hdb_vm_init(256, 512);
    }

    static void TearDownTestSuite() {
        hdb_vm_free();
    }
};

TEST_F(HdbUStringFixture, hdb_ustring_create_string_of_zero_chars) {
    const hdb_ustring_t* string = hdb_ustring_create("");

    EXPECT_NE(string, nullptr);
    EXPECT_EQ(string->obj.type, OBJ_STRING);
    EXPECT_EQ(string->length, 0);
}

TEST_F(HdbUStringFixture, hdb_ustring_create_string) {
    const hdb_ustring_t* string = hdb_ustring_create("hello world");

    EXPECT_NE(string, nullptr);
    EXPECT_EQ(string->obj.type, OBJ_STRING);
    EXPECT_EQ(string->length, 11);
}

TEST_F(HdbUStringFixture, hdb_ustring_create_string_null) {
    const hdb_ustring_t* string = hdb_ustring_create(nullptr);

    EXPECT_EQ(string, nullptr);
}

TEST_F(HdbUStringFixture, hdb_ustring_ncreate_null_with_length) {
    const hdb_ustring_t* string = hdb_ustring_ncreate(nullptr, 1);

    EXPECT_EQ(string, nullptr);
}

TEST_F(HdbUStringFixture, hdb_ustring_create_non_ascii) {
    const char* utf8 = u8"i ♥ u"; // 5 characters, 7 bytes
    const hdb_ustring_t* string = hdb_ustring_create(utf8);

    EXPECT_EQ(string->byte_length, 7);
    EXPECT_EQ(string->length, 5);
}

TEST_F(HdbUStringFixture, hdb_ustring_ncreate_non_ascii) {
    const char* utf8 = u8"i ♥ u"; // 5 characters, 7 bytes
    const hdb_ustring_t* string = hdb_ustring_ncreate(utf8, 3);

    EXPECT_EQ(string->byte_length, 5);
    EXPECT_EQ(string->length, 3);
}

TEST_F(HdbUStringFixture, hdb_ustring_concatenate) {
    const hdb_ustring_t* concat = hdb_ustring_concatenate(hdb_ustring_create("hello"), hdb_ustring_create(" world!"));

    EXPECT_STREQ(concat->chars, "hello world!");
}
