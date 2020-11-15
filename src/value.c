#include <string.h>

#include "memory.h"
#include "value.h"
#include "object.h"

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

bool hdb_values_equal(hdb_value_t left, hdb_value_t right) {
    if (left.type != right.type) {
        return false;
    }

    switch(left.type) {
        case VAL_BOOL: return AS_BOOL(left) == AS_BOOL(right);
        case VAL_NULL: return true;
        case VAL_NUMBER: return AS_NUMBER(left) == AS_NUMBER(right);
        case VAL_OBJ: {
            hdb_string_t* a = AS_STRING(left);
            hdb_string_t* b = AS_STRING(right);
            return a->length == b->length &&
                memcmp(a->chars, b->chars, a->length) == 0;
        }
        default:
            return false; // unreachable
    }
}

void hdb_free_value_array(hdb_value_array_t* array) {
    HDB_FREE_ARRAY(Value, array->values);
    hdb_init_value_array(array);
}