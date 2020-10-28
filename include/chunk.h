/**
 * Data structures and operations related to a \code hdb_chunk_t\endcode.
 *
 * \since 0.0.1
 * \author houthacker
 */
#ifndef HDB_CHUNK_H
#define HDB_CHUNK_H

#include "common.h"
#include "line.h"
#include "value.h"

/*!
 * This enum contains all opcodes for the byte code of hdb-ql.
 */
typedef enum {

    /*!<
     * opcode to read/write a constant \code hdb_value_t\endcode at an index < 256.
     * Operands:
     * - byte - the constant value index.
     */
    OP_CONSTANT,

    /*!<
     * opcode to read/write a constant \code hdb_value_t\endcode at an index >= 256.
     * Operands:
     * - int - an 24-bits integer containing the constant value index.
     */
    OP_CONSTANT_LONG,

    OP_ADD,

    OP_SUBTRACT,

    OP_MULTIPLY,

    OP_DIVIDE,

    /*!<
     * opcode to negate the value at the top of the stack.
     * Operands: none
     */
    OP_NEGATE,

    /*!<
     * opcode to return the value at the top of the stack to the caller.
     * Operands: none
     */
    OP_RETURN,
} hdb_opcode_t;

/**
 * Structure to contain information about a chunk of byte code.
 */
typedef struct {

    /**
     * The amount of instructions within this hdb_chunk_t.
     */
    int32_t count;

    /**
     * The current capacity of the array containing the instructions.
     */
    int32_t capacity;

    /**
     * The actual byte code instructions.
     */
    uint8_t* code;

    /**
     * run-length encoded line information per instruction.
     */
    hdb_line_array_t lines;

    /**
     * The constant values referred to by code within this hdb_chunk_t.
     */
    hdb_value_array_t constants;
} hdb_chunk_t;

/**
 * Initializes the given hdb_chunk_t. Must be called before use.
 *
 * \param chunk The pointer to the hdb_chunk_t to be initialized.
 */
void hdb_chunk_init(hdb_chunk_t *chunk);

/**
 * Returns the memory claimed by the given hdb_chunk_t to the heap.
 *
 * \param chunk The pointer to the hdb_chunk_t to be freed.
 */
void hdb_chunk_free(hdb_chunk_t *chunk);

/**
 * Stores the instruction in the hdb_chunk_t and encodes the line information.
 *
 * \param chunk The hdb_chunk_t to write to.
 * \param byte The instruction to write.
 * \param line The source code line the instruction originates from.
 */
void hdb_chunk_write(hdb_chunk_t *chunk, uint8_t byte, int32_t line);

/**
 * Stores the hdb_value_t within the hdb_chunk_t and writes the required instructions
 * to retrieve it later.
 *
 * \param chunk The hdb_chunk_t to write to.
 * \param value The constant value to store.
 * \param line The source code line the constant was introduced.
 */
void hdb_chunk_write_constant(hdb_chunk_t *chunk, hdb_value_t value, int32_t line);

/**
 * Reads a constant value based on the bytecode starting at the given offset.
 *
 * \param chunk The chunk containing the bytecode instructions.
 * \param code_pointer The current pointer to the chunk code.
 * \return The constant value.
 */
hdb_value_t hdb_chunk_read_constant(hdb_chunk_t *chunk, uint8_t *code_pointer);

#endif //HDB_CHUNK_H