#include "memory.h"
#include "value.h"

void hdb_init_value_array(hdb_value_array_t* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void hdb_write_value_array(hdb_value_array_t* array, hdb_value_t value) {
    if (array->capacity < array->count + 1) {
        array->capacity = HDB_GROW_CAPACITY(array->capacity);
        array->values = HDB_GROW_ARRAY(hdb_value_t, array->values, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void hdb_free_value_array(hdb_value_array_t* array) {
    HDB_FREE_ARRAY(Value, array->values);
    hdb_init_value_array(array);
}