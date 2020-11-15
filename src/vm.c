#include <stdio.h> // printf
#include <signal.h> // signal numbers
#include <stdarg.h> // va_list
#include <string.h> // memcpy

#include "memory.h"
#include "os.h"
#include "common.h"
#include "debug.h"
#include "vm.h"
#include "compiler.h"
#include "object.h"

hdb_vm_t* vm;

static void stack_init(void) {
    vm->stack = os_malloc(sizeof(hdb_value_t*) * vm->stack_capacity);
    vm->stack_count = 0;
}

static void stack_reset(void) {
    vm->stack_count = 0;
}

static void runtime_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    uint8_t instruction = vm->ip - vm->chunk->code - 1;
    int32_t line = hdb_line_decode(&vm->chunk->lines, instruction);
    fprintf(stderr, "[line %d] in script\n", line);

    stack_reset();
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
        hdb_heap_view_t* heap = hdb_heap_init(heap_min_size, heap_max_size);

        vm = os_malloc(sizeof(hdb_vm_t));
        vm->objects = NULL;

        // Set initial stack size so stack_init() will claim some memory for it.
        int32_t heap_based_stack_capacity = heap->current_size / 4096;
        vm->stack_capacity = heap_based_stack_capacity == 0 ? 512 : heap_based_stack_capacity;

        stack_init();
        hdb_compiler_init();
    }
}

void hdb_vm_free(void) {
    if (vm) {
        hdb_compiler_free();

        os_free(vm->stack);
        os_free(vm);
        vm = NULL;

        hdb_heap_free();
    }
}

const hdb_vm_t * hdb_vm(void) {
    return vm;
}

void hdb_vm_stack_push(hdb_value_t value) {
    // Stack doesn't need to grow here because
    // the stack size is set right before executing a chunk of byte code.
    *(vm->stack + vm->stack_count) = value;
    vm->stack_count++;
}

hdb_value_t hdb_vm_stack_pop(void) {
    // do not protect against a stack underflow, this will SIGSEGV on its own.
    vm->stack_count--;
    return *(vm->stack + vm->stack_count);
}

void hdb_vm_notify_new(hdb_object_t* object) {
    if (object) {
        object->next = vm->objects;
        vm->objects = object;
    }
}

static hdb_value_t stack_peek(int32_t distance) {
    // todo prevent stack underflow
    return *(vm->stack + vm->stack_count - 1 - distance);
}

static void concatenate() {
    hdb_string_t* right = AS_STRING(hdb_vm_stack_pop());
    hdb_string_t* left = AS_STRING(hdb_vm_stack_pop());

    int32_t length = left->length + right->length;
    char* chars = HDB_ALLOCATE(char, length + 1);
    memcpy(chars, left->chars, left->length);
    memcpy(chars + left->length, right->chars, right->length);
    chars[length] = '\0';

    hdb_string_t* result = hdb_object_take_string(chars, length);
    hdb_vm_stack_push(OBJ_VAL(result));

}

static hdb_interpret_result_t run(void) {
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (hdb_chunk_read_constant(vm->chunk, vm->ip++))
#define BINARY_OP(value_type, op) \
    do {              \
        if (!IS_NUMBER(stack_peek(0)) || !IS_NUMBER(stack_peek(1))) { \
            runtime_error("Operands must be numbers.");   \
            return INTERPRET_RUNTIME_ERROR;               \
        } \
        double right = AS_NUMBER(hdb_vm_stack_pop()); \
        double left  = AS_NUMBER(hdb_vm_stack_pop()); \
        hdb_vm_stack_push(value_type(left op right));  \
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

            case OP_NULL:       hdb_vm_stack_push(NULL_VAL); break;
            case OP_TRUE:       hdb_vm_stack_push(BOOL_VAL(true)); break;
            case OP_FALSE:      hdb_vm_stack_push(BOOL_VAL(false)); break;
            case OP_MINUS_ONE:  hdb_vm_stack_push(NUMBER_VAL(-1.0)); break;
            case OP_ZERO:       hdb_vm_stack_push(NUMBER_VAL(0.0)); break;
            case OP_ONE:        hdb_vm_stack_push(NUMBER_VAL(1.0)); break;
            case OP_TWO:        hdb_vm_stack_push(NUMBER_VAL(2.0)); break;

            case OP_EQUAL: {
                hdb_value_t right = hdb_vm_stack_pop();
                hdb_value_t left  = hdb_vm_stack_pop();
                hdb_vm_stack_push(BOOL_VAL(hdb_values_equal(left, right)));
                break;
            }

            case OP_NOT_EQUAL: {
                hdb_value_t right = hdb_vm_stack_pop();
                hdb_value_t left = hdb_vm_stack_pop();
                hdb_vm_stack_push(BOOL_VAL(!hdb_values_equal(left, right)));
                break;
            }

            case OP_LESS:           BINARY_OP(NUMBER_VAL, <); break;
            case OP_LESS_EQUAL:     BINARY_OP(NUMBER_VAL, <=); break;
            case OP_GREATER:        BINARY_OP(NUMBER_VAL, >); break;
            case OP_GREATER_EQUAL:  BINARY_OP(NUMBER_VAL, >=); break;
            case OP_ADD: {
                if (IS_STRING(stack_peek(0)) && IS_STRING(stack_peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(stack_peek(0)) && IS_NUMBER(stack_peek(1))) {
                    BINARY_OP(NUMBER_VAL, +);
                } else {
                    runtime_error("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SUBTRACT:       BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:       BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:         BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT: {
                hdb_value_t *v = &vm->stack[vm->stack_count] - 1;

                if (!IS_BOOL(*v)) {
                    runtime_error("Operand must be a boolean value.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                // Update in-place
                v->as.boolean = !v->as.boolean;
                break;
            }

            case OP_NEGATE: {
                if (!IS_NUMBER(stack_peek(0))) {
                    runtime_error("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                // Update in-place
                hdb_value_t *v = &vm->stack[vm->stack_count] - 1;
                v->as.number = -v->as.number;
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