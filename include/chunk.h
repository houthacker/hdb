/**
 * Data structures and operations related to a \code Chunk\endcode.
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
     * OpCode to read/write a constant \code Value\endcode at an index < 256.
     * Operands:
     * - byte - the constant value index.
     */
    OP_CONSTANT,

    /*!<
     * OpCode to read/write a constant \code Value\endcode at an index >= 256.
     * Operands:
     * - int - an 24-bits integer containing the constant value index.
     */
    OP_CONSTANT_LONG,

    /*!<
     * OpCode to return the value at the top of the stack to the caller.
     * Operands: none
     */
    OP_RETURN,
} OpCode;

/**
 * Structure to contain information about a chunk of byte code.
 */
typedef struct {

    /**
     * The amount of instructions within this Chunk.
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
    LineArray lines;

    /**
     * The constant values referred to by code within this Chunk.
     */
    ValueArray constants;
} Chunk;

/**
 * Initializes the given Chunk. Must be called before use.
 *
 * \param chunk The pointer to the Chunk to be initialized.
 */
void initChunk(Chunk* const chunk);

/**
 * Returns the memory claimed by the given Chunk to the heap.
 *
 * \param chunk The pointer to the Chunk to be freed.
 */
void freeChunk(Chunk* const chunk);

/**
 * Stores the instruction in the Chunk and encodes the line information.
 *
 * \param chunk The Chunk to write to.
 * \param byte The instruction to write.
 * \param line The source code line the instruction originates from.
 */
void writeChunk(Chunk* const chunk, uint8_t byte, int line);

/**
 * Stores the Value within the Chunk and writes the required instructions
 * to retrieve it later.
 *
 * \param chunk The Chunk to write to.
 * \param value The constant value to store.
 * \param line The source code line the constant was introduced.
 */
void writeConstant(Chunk* const chunk, Value value, int line);

#endif //HDB_CHUNK_H