#ifndef HDB_VM_H
#define HDB_VM_H

#include "chunk.h"

// Max stack size is 4MB (a pointer to a hdb_value_t uses 8 bytes)
#define HDB_STACK_MAX_SIZE 524288

/**
 * Representation of the HDB Virtual Machine stack.
 */
typedef struct {

    /**
     * The current amount of elements on the stack.
     */
    int32_t count;

    /**
     * The current maximum capacity.
     */
    int32_t capacity;

    /**
     * The values residing on the stack.
     */
    hdb_value_t* values;
} hdb_stack_t;

/**
 * A handle to the HDB Virtual Machine
 */
typedef struct {

    /**
     * The array of chunks containing instructions to execute.
     */
    hdb_chunk_t *chunk;

    /**
     * The Instruction Pointer.
     */
    uint8_t* ip;

    /**
     * The current stack.
     */
    hdb_value_t* stack;

    /**
     * The current amount of elements on the stack.
     */
    int32_t stack_count;

    /**
     * The current capacity of the stack.
     */
    int32_t stack_capacity;
} hdb_vm_t;

/**
 * Description of interpretation result states.
 */
typedef enum {

    /**
     * Interpretation was successful.
     */
    INTERPRET_OK,

    /**
     * An error occurred during compilation of the current chunk.
     */
    INTERPRET_COMPILE_ERROR,

    /**
     * Compilation was successful, but executing the instructions in the current
     * chunk failed.
     */
    INTERPRET_RUNTIME_ERROR
} hdb_interpret_result_t;

/**
 * Initializes the HDB Virtual Machine.
 *
 * \param heap_min_size The minimum heap size in bytes.
 * \param heap_max_size The maximum heap size in bytes.
 */
void hdb_vm_init(size_t heap_min_size, size_t heap_max_size);

/**
 * Stops and destroys the HDB Virtual Machine.
 */
void hdb_vm_free(void);

/**
 * \return The current HDB Virtual Machine
 */
const hdb_vm_t* hdb_vm(void);

/**
 * Interprets the byte code in the given chunk and returns the result state.
 *
 * \param source The source code to interpret.
 * \return The result state.
 */
hdb_interpret_result_t hdb_vm_interpret(const char* source);

/**
 * Pushes the given value onto the stack.
 *
 * \param value The value to push onto the stack.
 */
void hdb_vm_stack_push(hdb_value_t value);

/**
 * Yields and removes the top item from the stack.
 *
 * \return The top item from the stack.
 */
hdb_value_t hdb_vm_stack_pop(void);

#endif //HDB_VM_H
