#include <stdio.h> // printf

#include "os.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

hdb_vm_t* vm;

static void reset_stack() {
    vm->stack_top = vm->stack;
}

void hdb_vm_init() {
    if (!vm) {
        vm = os_malloc(sizeof(hdb_vm_t));
        reset_stack();
    }
}

void hdb_vm_free() {
    if (vm) {
        os_free(vm);
        vm = NULL;
    }
}

const hdb_vm_t * hdb_vm() {
    return vm;
}

void hdb_vm_push(hdb_value_t value) {
    *vm->stack_top = value;
    vm->stack_top++;
}

hdb_value_t hdb_vm_pop() {
    vm->stack_top--;
    return *vm->stack_top;
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
        for (hdb_value_t * slot = vm->stack; slot < vm->stack_top; slot++) {
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
            case OP_NEGATE: (hdb_vm_push(-hdb_vm_pop())); break;
            case OP_RETURN:
                hdb_dbg_print_value(hdb_vm_pop());
                printf("\n");
                return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

hdb_interpret_result_t hdb_vm_interpret(hdb_chunk_t *chunk) {
    if (vm && chunk) {
        vm->chunk = chunk;
        vm->ip = chunk->code;

        return run();
    }

    return INTERPRET_RUNTIME_ERROR;
}