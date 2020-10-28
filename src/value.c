#include "memory.h"
#include "value.h"

void init_value_array(value_array_t* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void write_value_array(value_array_t* array, value_t value) {
    if (array->capacity < array->count + 1) {
        array->capacity = HDB_GROW_CAPACITY(array->capacity);
        array->values = HDB_GROW_ARRAY(value_t, array->values, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void free_value_array(value_array_t* array) {
    HDB_FREE_ARRAY(Value, array->values);
    init_value_array(array);
}