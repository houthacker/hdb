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

static void stack_init() {
    vm->stack = os_malloc(sizeof(hdb_value_t*) * vm->stack_capacity);
    vm->stack_count = 0;
}

static void stack_grow(uint8_t factor) {
    const size_t requested_capacity = vm->stack_capacity * factor;

    // stackoverflow when requesting size > max capacity
    if (requested_capacity > HDB_STACK_MAX_SIZE) {
        if (os_raise(SIGSEGV) != 0) {
            os_abort();
        }
    }

    vm->stack_capacity = requested_capacity;
    vm->stack = os_realloc(vm->stack, sizeof(hdb_value_t*) * vm->stack_capacity);
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

        os_free(vm->stack);
        os_free(vm);
        vm = NULL;

        hdb_heap_free();
    }
}

const hdb_vm_t * hdb_vm() {
    return vm;
}

void hdb_vm_stack_push(hdb_value_t value) {
    // Stack doesn't need to grow here because
    // the stack size is set right before executing a chunk of byte code.
    *(vm->stack + vm->stack_count) = value;
    vm->stack_count++;
}

hdb_value_t hdb_vm_stack_pop() {
    // do not protect against a stack underflow, this will SIGSEGV on its own.
    vm->stack_count--;
    return *(vm->stack + vm->stack_count);
}

static hdb_interpret_result_t run() {
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (hdb_chunk_read_constant(vm->chunk, vm->ip++))
#define BINARY_OP(op) \
    do {              \
        double right = hdb_vm_stack_pop(); \
        double left  = hdb_vm_stack_pop(); \
        hdb_vm_stack_push(left op right);  \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (int32_t slot = 0; slot < vm->stack_count; slot++) {
            hdb_value_t value = vm->stack[slot];
            printf("[ ");
            hdb_dbg_print_value(value);
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
                hdb_vm_stack_push(value);
                break;
            }
            case OP_ADD: BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE: BINARY_OP(/); break;
            case OP_NEGATE:
            {
                hdb_value_t *v = &vm->stack[vm->stack_count] - 1;
                *v = -(*v);
            }
            break;
            case OP_RETURN:
#ifdef DEBUG_TRACE_EXECUTION
                hdb_dbg_print_value(hdb_vm_stack_pop());
                printf("\n");
#else
                hdb_vm_stack_pop();
#endif
                return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

// Increase stack size if required
static void ensure_stack_size(hdb_chunk_t chunk) {
    int32_t stack_free = vm->stack_capacity - vm->stack_count;
    if (stack_free >= chunk.stack_high_water_mark) {
        return;
    }

    uint8_t factor = 1;
    int32_t min_free_required = chunk.stack_high_water_mark;
    while((vm->stack_capacity * factor) - vm->stack_count < min_free_required) {
        factor *= 2;
    }

    stack_grow(factor);
}

hdb_interpret_result_t hdb_vm_interpret(const char* source) {
    hdb_chunk_t chunk;
    hdb_chunk_init(&chunk);

    if (!hdb_compiler_compile(source, &chunk)) {
        return INTERPRET_COMPILE_ERROR;
    }

    vm->chunk = &chunk;
    vm->ip = vm->chunk->code;

    ensure_stack_size(chunk);
    hdb_interpret_result_t result = run();

    hdb_chunk_free(&chunk);
    return result;
}