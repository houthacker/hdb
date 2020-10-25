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
    int instructionCount;

    /*!<
     * The source code line number.
     */
    int line;
} Line;

/**
 * Structure to store multiple encoded lines.
 */
typedef struct {

    /**
     * The amount of encoded lines in this array.
     */
    int count;

    /**
     * The current line array capacity.
     */
    int capacity;

    /**
     * The lines, sorted by line number.
     */
    Line* lines;
} LineArray;

/**
 * Initializes the given lineArray. Must be called before using it.
 *
 * \param lineArray The line array to initialize.
 */
void initLineArray(LineArray* lineArray);

/**
 * Returns the memory claimed by the given LineArray to the heap.
 *
 * \param lineArray The pointer to the LineArray to be freed.
 */
void freeLineArray(LineArray* lineArray);

/**
 * Stores the given line, and increments the instruction count by one.
 *
 * \param lineArray The array of lines to encode the given line in.
 * \param line The source code line number.
 * \return The new size of the line array.
 */
int encodeLine(LineArray* lineArray, int line);

/**
 * Decodes the line number at the given instruction index.
 *
 * \param lineArray The line array containing the encoded lines.
 * \param instructionIndex The index of the related bytecode instruction.
 * \return The line number, or -1 if no such line exists.
 */
int decodeLine(LineArray* lineArray, int instructionIndex);

#endif //HDB_LINE_H
