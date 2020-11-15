/**
 * Data structures and operations related to constant values.
 *
 * \since 0.0.1
 * \author houthacker
 */
#ifndef HDB_VALUE_H
#define HDB_VALUE_H

typedef struct hdb_object hdb_object_t;

typedef enum {
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
    VAL_OBJ
} hdb_value_type_t;

/**
 * Type definition for the supported value types in hdb
 */
typedef struct {
    hdb_value_type_t type;
    union {
        bool boolean;
        double number;
        hdb_object_t* obj;
    } as;
} hdb_value_t;

#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NULL (value)     ((value).type == VAL_NULL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)
#define IS_OBJ(value)       ((value).type == VAL_OBJ)

#define AS_OBJ(value)       ((value).as.obj)
#define AS_BOOL(value)      ((value).as.boolean)
#define AS_NUMBER(value)    ((value).as.number)

#define BOOL_VAL(literal)   ((hdb_value_t){VAL_BOOL,    {.boolean   = literal}})
#define NULL_VAL            ((hdb_value_t){VAL_NULL,    {.number    = 0}})
#define NUMBER_VAL(literal) ((hdb_value_t){VAL_NUMBER,  {.number    = literal}})
#define OBJ_VAL(object)     ((hdb_value_t){VAL_OBJ,     {.obj       = (hdb_object_t*)object}})

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
 * Compares the given values and returns whether they are considered equal.
 *
 * \param left The left operand
 * \param right The right operand
 * \return Whether the given values are equal.
 */
bool hdb_values_equal(hdb_value_t left, hdb_value_t right);

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
