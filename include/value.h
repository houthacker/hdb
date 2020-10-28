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
typedef double hdb_value_t;

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
    hdb_value_t* values;
} hdb_value_array_t;

/**
 * Initializes the given array. Must be called before using it.
 *
 * \param array The value array to be initialized.
 */
void hdb_init_value_array(hdb_value_array_t* array);

/**
 * Writes a new value to the array.
 *
 * \param array The value array to be written to.
 * \param value The value to be written.
 */
void hdb_write_value_array(hdb_value_array_t* array, hdb_value_t value);

/**
 * Returns the memory claimed by the given hdb_value_array_t to the heap.
 *
 * \param array The array to be freed.
 */
void hdb_free_value_array(hdb_value_array_t* array);

#endif //HDB_VALUE_H
