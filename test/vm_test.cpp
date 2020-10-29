#include "gtest/gtest.h"

extern "C" {
#include <ctime>

#include <vm.h>
#include <memory.h>
#include <chunk.h>
}

class HdbVMFixture : public ::testing::Test {
protected:
    const hdb_vm_t *vm;

    hdb_chunk_t* chunk;

    virtual void SetUp() {
        hdb_vm_init();
        hdb_heap_init(256, 512);
        chunk = (hdb_chunk_t*)hdb_malloc(sizeof(hdb_chunk_t));
        hdb_chunk_init(chunk);

        vm = hdb_vm();
    }

    virtual void TearDown() {
        hdb_free(chunk);
        hdb_heap_free();
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
    hdb_vm_init();
}

TEST_F(HdbVMFixture, hdb_negate_value) {
    hdb_chunk_write_constant(chunk, 1.337, 123);
    hdb_chunk_write(chunk, OP_NEGATE, 123);
    hdb_chunk_write(chunk, OP_RETURN, 123);

    hdb_vm_interpret(chunk);

    hdb_value_t from_stack = *vm->stack->top;
    EXPECT_EQ(from_stack, -1.337);
}

TEST_F(HdbVMFixture, hdb_add_value) {
    hdb_chunk_write_constant(chunk, 1.337, 123);
    hdb_chunk_write_constant(chunk, 0.663, 123);
    hdb_chunk_write(chunk, OP_ADD, 123);

    hdb_chunk_write(chunk, OP_RETURN, 123);

    hdb_vm_interpret(chunk);

    hdb_value_t from_stack = *vm->stack->top;
    EXPECT_EQ(from_stack, 1.337 + 0.663);
}

TEST_F(HdbVMFixture, hdb_subtract_value) {
    hdb_chunk_write_constant(chunk, 1.337, 123);
    hdb_chunk_write_constant(chunk, 0.663, 123);
    hdb_chunk_write(chunk, OP_SUBTRACT, 123);

    hdb_chunk_write(chunk, OP_RETURN, 123);

    hdb_vm_interpret(chunk);

    hdb_value_t from_stack = *vm->stack->top;
    EXPECT_EQ(from_stack, 1.337 - 0.663);
}

TEST_F(HdbVMFixture, hdb_multiply_value) {
    hdb_chunk_write_constant(chunk, 1.337, 123);
    hdb_chunk_write_constant(chunk, 0.663, 123);
    hdb_chunk_write(chunk, OP_MULTIPLY, 123);

    hdb_chunk_write(chunk, OP_RETURN, 123);

    hdb_vm_interpret(chunk);

    hdb_value_t from_stack = *vm->stack->top;
    EXPECT_EQ(from_stack, 0.886431);
}

TEST_F(HdbVMFixture, hdb_divide_value) {
    hdb_chunk_write_constant(chunk, 1.337, 123);
    hdb_chunk_write_constant(chunk, 0.663, 123);
    hdb_chunk_write(chunk, OP_DIVIDE, 123);

    hdb_chunk_write(chunk, OP_RETURN, 123);

    hdb_vm_interpret(chunk);

    hdb_value_t from_stack = *vm->stack->top;
    EXPECT_EQ(from_stack, 1.337 / 0.663);
}

TEST_F(HdbVMFixture, hdb_binary_op_mix) {
    hdb_chunk_write_constant(chunk, 1.337, 123);
    hdb_chunk_write_constant(chunk, 0.663, 123);
    hdb_chunk_write(chunk, OP_ADD, 123);

    hdb_chunk_write_constant(chunk, 100, 123);
    hdb_chunk_write(chunk, OP_DIVIDE, 123);
    hdb_chunk_write(chunk, OP_NEGATE, 123);

    hdb_chunk_write(chunk, OP_RETURN, 123);

    hdb_vm_interpret(chunk);

    hdb_value_t from_stack = *vm->stack->top;
    EXPECT_EQ(from_stack, -((1.337 + 0.663) / 100));
}

TEST_F(HdbVMFixture, hdb_simple_calculation) {
    // 4 - 3 * -2
    hdb_chunk_write_constant(chunk, 4, 123);
    hdb_chunk_write_constant(chunk, 3, 123);
    hdb_chunk_write_constant(chunk, 2, 123);
    hdb_chunk_write(chunk, OP_NEGATE, 123);
    hdb_chunk_write(chunk, OP_MULTIPLY, 123);
    hdb_chunk_write(chunk, OP_SUBTRACT, 123);
    hdb_chunk_write(chunk, OP_RETURN, 123);

    hdb_vm_interpret(chunk);

    hdb_value_t from_stack = *vm->stack->top;
    EXPECT_EQ(from_stack, 4 - 3 * -2 /* 10 */ );
}

TEST_F(HdbVMFixture, DISABLED_hdb_vm_interpretation_performance) {
    // Only test this when DEBUG_TRACE_EXECUTION is off!

    hdb_chunk_write_constant(chunk, 1.337, 123);
    hdb_chunk_write(chunk, OP_NEGATE, 123);
    hdb_chunk_write(chunk, OP_RETURN, 123);

    int32_t sz = 200000000;
    timespec start, finish;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int32_t i = 0; i < sz; i++) {
        hdb_vm_interpret(chunk);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &finish);

    timespec d = diff(start, finish);
    ulong ns = (d.tv_sec * 1000000000 + d.tv_nsec);
    printf("Duration per op: %luns\n", ns / sz);
}