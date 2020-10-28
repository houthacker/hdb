/**
 * Methods to introspect data structures within hdb context.
 *
 * \since 0.0.1
 * \author houthacker
 */
#include "chunk.h"

#ifndef HDB_DEBUG_H
#define HDB_DEBUG_H

/**
 * Prints a string representation of all bytecode instructions within the hdb_chunk_t to standard out.
 *
 * \param chunk The hdb_chunk_t to disassemble.
 * \param name The name of the hdb_chunk_t to refer to.
 */
void hdb_dbg_disassemble_chunk(hdb_chunk_t* chunk, const char* name);

/**
 * Prints a string representation of the bytecode instruction at the given offset to standard out.
 *
 * \param chunk The hdb_chunk_t containing the instruction to print.
 * \param offset The offset to read the instruction from.
 * \return The next offset an instruction can be read from.
 */
int hdb_dbg_disassemble_instruction(hdb_chunk_t* chunk, int32_t offset);

/**
 * Prints the given value to standard out.
 *
 * \param value The value to print
 */
void hdb_dbg_print_value(hdb_value_t value);

#endif //HDB_DEBUG_H
