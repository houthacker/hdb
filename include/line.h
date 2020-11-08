/**
 * Data structures and operations related to line encoding.
 *
 * \since 0.0.1
 * \author houthacker
 */
#ifndef HDB_LINE_H
#define HDB_LINE_H

#include "common.h"

/*!
 * Encoded line information.
 */
typedef struct {

    /*!<
     * The amount of bytecode instructions originating at this line.
     */
    int32_t instruction_count;

    /*!<
     * The source code line number.
     */
    int32_t line;
} hdb_line_t;

/**
 * Structure to store multiple encoded lines.
 */
typedef struct {

    /**
     * The amount of encoded lines in this array.
     */
    int32_t count;

    /**
     * The current line array capacity.
     */
    int32_t capacity;

    /**
     * The lines, sorted by line number.
     */
    hdb_line_t* lines;
} hdb_line_array_t;

/**
 * Initializes the given array. Must be called before using it.
 *
 * \param array The line array to initialize.
 */
void hdb_line_array_init(hdb_line_array_t* array);

/**
 * Returns the memory claimed by the given hdb_line_array_t to the heap.
 *
 * \param array The pointer to the hdb_line_array_t to be freed.
 */
void hdb_line_array_free(hdb_line_array_t* array);

/**
 * Stores the given line, and increments the instruction count by one.
 *
 * \param array The array of lines to encode the given line in.
 * \param line The source code line number.
 * \return The new size of the line array.
 */
int32_t hdb_line_encode(hdb_line_array_t* array, int32_t line);

/**
 * Decodes the line number at the given instruction index.
 *
 * \param array The line array containing the encoded lines.
 * \param instruction_index The index of the related bytecode instruction.
 * \return The line number, or -1 if no such line exists.
 */
int32_t hdb_line_decode(hdb_line_array_t* array, int32_t instruction_index);

#endif //HDB_LINE_H
