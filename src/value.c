#include "memory.h"
#include "value.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        array->capacity = HDB_GROW_CAPACITY(array->capacity);
        array->values = HDB_GROW_ARRAY(Value, array->values, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    HDB_FREE_ARRAY(Value, array->values);
    initValueArray(array);
}