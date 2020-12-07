#include "gtest/gtest.h"

extern "C" {
#include <ctime>

#include <vm.h>
#include <memory.h>
#include <chunk.h>
#include <object.h>
#include <ustring.h>
}

class HdbVMFixture : public ::testing::Test {
protected:
    const hdb_vm_t *vm;

    virtual void SetUp() {
        hdb_vm_init(256, 512);
        vm = hdb_vm();
    }

    virtual void TearDown() {
        hdb_vm_free();
    }


    timespec diff(timespec start, timespec end)
    {
        timespec temp;
        if ((end.tv_nsec-start.tv_nsec)<0) {
            temp.tv_sec = end.tv_sec-start.tv_sec-1;
            temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
        } else {
            temp.tv_sec = end.tv_sec-start.tv_sec;
            temp.tv_nsec = end.tv_nsec-start.tv_nsec;
        }
        return temp;
    }
};

TEST_F(HdbVMFixture, hdb_init_vm_twice) {
    hdb_vm_init(256, 512);
}

TEST_F(HdbVMFixture, hdb_validate_precedence) {
    const char* source = "(-1 + 2) * 3 - -4";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_NUMBER(vm->stack[vm->stack_count]), 7);
}

TEST_F(HdbVMFixture, hdb_negate_value) {
    const char* source = "-1.337";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_NUMBER(vm->stack[vm->stack_count]), -1.337);
}

TEST_F(HdbVMFixture, hdb_add_value) {
    const char* source = "1.337 + 0.663";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_NUMBER(vm->stack[vm->stack_count]), 1.337 + 0.663);
}

TEST_F(HdbVMFixture, hdb_subtract_value) {
    const char* source = "1.337 - 0.663";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_NUMBER(vm->stack[vm->stack_count]), 1.337 - 0.663);
}

TEST_F(HdbVMFixture, hdb_multiply_value) {
    const char* source = "1.337 * 0.663";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_NUMBER(vm->stack[vm->stack_count]), 1.337 * 0.663);
}

TEST_F(HdbVMFixture, hdb_divide_value) {
    const char* source = "1.337 / 0.663";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_NUMBER(vm->stack[vm->stack_count]), 1.337 / 0.663);
}

TEST_F(HdbVMFixture, hdb_binary_op_mix) {
    const char* source = "-((1.337 + 0.663) / 100)";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_NUMBER(vm->stack[vm->stack_count]), -((1.337 + 0.663) / 100));
}

TEST_F(HdbVMFixture, hdb_equals_different_types) {
    const char* source = "1 = false";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK); // todo compile time type checking
    EXPECT_EQ(AS_BOOL(vm->stack[vm->stack_count]), false);
}

TEST_F(HdbVMFixture, hdb_equals_double) {
    const char* source = "1.337 = 1.337";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_BOOL(vm->stack[vm->stack_count]), true);
}

TEST_F(HdbVMFixture, hdb_equals_null) {
    const char* source = "null = null";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_BOOL(vm->stack[vm->stack_count]), true);
}

TEST_F(HdbVMFixture, hdb_equals_bool) {
    const char* source = "false = false";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_BOOL(vm->stack[vm->stack_count]), true);
}

TEST_F(HdbVMFixture, hdb_equals_string) {
    const char* source = "'stringy' = 'stringy'";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_BOOL(vm->stack[vm->stack_count]), true);
}

TEST_F(HdbVMFixture, hdb_equals_different_strings) {
    const char* source = "'stringa' = 'stringb'";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);
    EXPECT_EQ(AS_BOOL(vm->stack[vm->stack_count]), false);
}

TEST_F(HdbVMFixture, hdb_string_concatenation) {
    const char* source = "'st' + 'ri' + 'ng'";
    hdb_interpret_result_t result = hdb_vm_interpret(source);

    EXPECT_EQ(result, INTERPRET_OK);

    hdb_value_t value = vm->stack[vm->stack_count];
    EXPECT_TRUE(IS_STRING(value));

    hdb_ustring_t* string = AS_STRING(value);
    EXPECT_EQ(string->length, 6);
    EXPECT_STREQ(AS_CSTRING(value), "string");
}

TEST_F(HdbVMFixture, DISABLED_hdb_vm_interpretation_performance) {
    // Only run this test when DEBUG_TRACE_EXECUTION and DEBUG_PRINT_CODE are off! see common.h

    const char* source = "(-1 + 2) * 3 - -4";
    int32_t sz = 20000000;
    timespec start, finish;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int32_t i = 0; i < sz; i++) {
        hdb_vm_interpret(source);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &finish);

    timespec d = diff(start, finish);
#ifdef __APPLE__
    u_long ns = (d.tv_sec * 1000000000 + d.tv_nsec);
#else
    ulong ns = (d.tv_sec * 1000000000 + d.tv_nsec);
#endif
    printf("Duration per chunk: %luns\n", ns / sz);
}