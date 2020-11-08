/**
 * Data structures and operations related to constant values.
 *
 * \since 0.0.1
 * \author houthacker
 */
#ifndef HDB_VALUE_H
#define HDB_VALUE_H

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER
} hdb_value_type_t;

/**
 * Type definition for the supported value types in hdb
 */
typedef struct {
    hdb_value_type_t type;
    union {
        bool boolean;
        double number;
    } as;
} hdb_value_t;

#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NIL(value)       ((value).type == VAL_NIL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)

#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)

#define BOOL_VAL(value)     ((hdb_value_t){VAL_BOOL,    {.boolean   = value}})
#define NIL_VAL             ((hdb_value_t){VAL_NIL,     {.number    = 0}})
#define NUMBER_VAL(value)   ((hdb_value_t){VAL_NUMBER,  {.number    = value}})

/**
 * Structure to store multiple values.
 */
typedef struct {

    /**
     * The amount of values in this array.
     */
    int32_t count;

    /**
     * The current maximum value array capacity.
     */
    int32_t capacity;

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
