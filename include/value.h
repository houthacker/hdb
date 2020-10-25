/**
 * Data structures and operations related to constant values.
 *
 * \since 0.0.1
 * \author houthacker
 */
#ifndef HDB_VALUE_H
#define HDB_VALUE_H

/**
 * Type definition for the (only) supported value type in hdb
 */
typedef double Value;

/**
 * Structure to store multiple values.
 */
typedef struct {

    /**
     * The amount of values in this array.
     */
    int count;

    /**
     * The current maximum value array capacity.
     */
    int capacity;

    /**
     * The value array.
     */
    Value* values;
} ValueArray;

/**
 * Initializes the given array. Must be called before using it.
 *
 * \param array The value array to be initialized.
 */
void initValueArray(ValueArray* array);

/**
 * Writes a new value to the array.
 *
 * \param array The value array to be written to.
 * \param value  The value to be written.
 */
void writeValueArray(ValueArray* array, Value value);

/**
 * Returns the memory claimed by the given ValueArray to the heap.
 *
 * \param array The array to be freed.
 */
void freeValueArray(ValueArray* array);

#endif //HDB_VALUE_H
