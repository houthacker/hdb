#include <stdio.h> // printf
#include <signal.h> // signal numbers

#include "memory.h"
#include "os.h"
#include "common.h"
#include "debug.h"
#include "vm.h"
#include "compiler.h"

#define HDB_STACK_CUR_IP (vm->stack_top - vm->stack)
#define HDB_STACK_NEXT_IP (vm->stack_top - vm->stack + 1)

hdb_vm_t* vm;

static void stack_reset() {
    vm->stack->top = vm->stack->values;
}

static void stack_init() {
    hdb_stack_t stack;

    vm->stack = &stack; // os_malloc(sizeof(hdb_stack_t));
    vm->stack->count = 0;
    vm->stack->capacity = 8;
    //vm->stack->values = os_malloc(sizeof(hdb_value_t*) * 8);
    vm->stack->top = vm->stack->values;
}

static void stack_grow() {
    const size_t requested_capacity = vm->stack->capacity * 2;

    // stackoverflow when requesting size > max capacity
    if (requested_capacity > HDB_STACK_MAX_SIZE) {
        if (os_raise(SIGSEGV) != 0) {
            os_abort();
        }
    }

    //intptr_t sp = vm->stack->top - vm->stack->values;
    vm->stack->capacity = requested_capacity;
    //vm->stack->values = os_realloc(vm->stack->values, sizeof(hdb_value_t*) * vm->stack->capacity);
    //vm->stack->top = vm->stack->values + sp;
}

void hdb_vm_init(size_t heap_min_size, size_t heap_max_size) {
    if (!vm) {
        hdb_heap_init(heap_min_size, heap_max_size);

        vm = os_malloc(sizeof(hdb_vm_t));
        stack_init();

        hdb_compiler_init();
    }
}

void hdb_vm_free() {
    if (vm) {
        hdb_compiler_free();

        os_free(vm);
        vm = NULL;

        hdb_heap_free();
    }
}

const hdb_vm_t * hdb_vm() {
    return vm;
}

void hdb_vm_push(hdb_value_t value) {
    if (vm->stack->count + 1 > vm->stack->capacity) {
        stack_grow();
    }

    *vm->stack->top = value;
    vm->stack->top++;
}

hdb_value_t hdb_vm_pop() {
    if (vm->stack->top == vm->stack->values) {
        // stack underflow
        if (os_raise(SIGSEGV) != 0) {
            os_abort();
        }
    }

    return *(--vm->stack->top);
}

static hdb_interpret_result_t run() {
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT(offset) (hdb_chunk_read_constant(vm->chunk, vm->ip++))
#define BINARY_OP(op) \
    do {              \
        double right = hdb_vm_pop(); \
        double left  = hdb_vm_pop(); \
        hdb_vm_push(left op right);  \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (hdb_value_t * slot = vm->stack->values; slot < vm->stack->top; slot++) {
            printf("[ ");
            hdb_dbg_print_value(*slot);
            printf(" ]");
        }
        printf("\n");
        hdb_dbg_disassemble_instruction(vm->chunk, (int32_t) (vm->ip - vm->chunk->code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT:
            case OP_CONSTANT_LONG: {
                hdb_value_t value = READ_CONSTANT();
                hdb_vm_push(value);
                break;
            }
            case OP_ADD: BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE: BINARY_OP(/); break;
            case OP_NEGATE:
            {
                hdb_value_t *v = (vm->stack->top - 1);
                *v = -(*v);
            }
            break;
            case OP_RETURN:
#ifdef DEBUG_TRACE_EXECUTION
                hdb_dbg_print_value(hdb_vm_pop());
                printf("\n");
#else
                hdb_vm_pop();
#endif
                return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

hdb_interpret_result_t hdb_vm_interpret(const char* source) {
    hdb_compiler_compile(source);
    return INTERPRET_OK;
}