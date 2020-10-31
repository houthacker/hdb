#ifndef HDB_VM_H
#define HDB_VM_H

#include "chunk.h"

#define HDB_STACK_MAX_SIZE 1024

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
     * The pointer to the current top of the stack
     */
    hdb_value_t* top;

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
     * The current stack
     */
    hdb_stack_t* stack;
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
void hdb_vm_free();

/**
 * \return The current HDB Virtual Machine
 */
const hdb_vm_t* hdb_vm();

/**
 * Interprets the byte code in the given chunk and returns the result state.
 *
 * \param source The source code to interpret.
 * \return The result state.
 */
hdb_interpret_result_t hdb_vm_interpret(const char* source);

void hdb_vm_push(hdb_value_t value);

hdb_value_t hdb_vm_pop();

#endif //HDB_VM_H
