/**
 * Data structures and operations related to a \code chunk_t\endcode.
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
     * opcode to read/write a constant \code value_t\endcode at an index < 256.
     * Operands:
     * - byte - the constant value index.
     */
    OP_CONSTANT,

    /*!<
     * opcode to read/write a constant \code value_t\endcode at an index >= 256.
     * Operands:
     * - int - an 24-bits integer containing the constant value index.
     */
    OP_CONSTANT_LONG,

    /*!<
     * opcode to return the value at the top of the stack to the caller.
     * Operands: none
     */
    OP_RETURN,
} opcode;

/**
 * Structure to contain information about a chunk of byte code.
 */
typedef struct {

    /**
     * The amount of instructions within this chunk_t.
     */
    int count;

    /**
     * The current capacity of the array containing the instructions.
     */
    int capacity;

    /**
     * The actual byte code instructions.
     */
    uint8_t* code;

    /**
     * run-length encoded line information per instruction.
     */
    line_array_t lines;

    /**
     * The constant values referred to by code within this chunk_t.
     */
    value_array_t constants;
} chunk_t;

/**
 * Initializes the given chunk_t. Must be called before use.
 *
 * \param chunk The pointer to the chunk_t to be initialized.
 */
void init_chunk(chunk_t* const chunk);

/**
 * Returns the memory claimed by the given chunk_t to the heap.
 *
 * \param chunk The pointer to the chunk_t to be freed.
 */
void free_chunk(chunk_t* const chunk);

/**
 * Stores the instruction in the chunk_t and encodes the line information.
 *
 * \param chunk The chunk_t to write to.
 * \param byte The instruction to write.
 * \param line The source code line the instruction originates from.
 */
void write_chunk(chunk_t* const chunk, uint8_t byte, int line);

/**
 * Stores the value_t within the chunk_t and writes the required instructions
 * to retrieve it later.
 *
 * \param chunk The chunk_t to write to.
 * \param value The constant value to store.
 * \param line The source code line the constant was introduced.
 */
void write_constant(chunk_t* const chunk, value_t value, int line);

#endif //HDB_CHUNK_H