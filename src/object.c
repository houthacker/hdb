#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, object_type) \
    (type*)allocate_object(sizeof(type), object_type)

static hdb_object_t* allocate_object(size_t size, hdb_object_type_t type) {
    hdb_object_t* object = (hdb_object_t*)hdb_malloc(size);
    object->type = type;

    hdb_vm_notify_new(object);
    return object;
}

static hdb_string_t* allocate_string(char* chars, uint32_t length) {
    hdb_string_t* string = ALLOCATE_OBJ(hdb_string_t, OBJ_STRING);
    string->length = length;
    string->chars = chars;

    return string;
}

hdb_string_t* hdb_object_take_string(char* chars, uint32_t length) {
    if (chars) {
        return allocate_string(chars, length);
    }

    return NULL;
}

hdb_string_t* hdb_object_copy_string(const char* chars, uint32_t length) {
    if (chars) {
        char *heap_chars = HDB_ALLOCATE(char, length + 1);
        memcpy(heap_chars, chars, length);
        heap_chars[length] = '\0';

        return allocate_string(heap_chars, length);
    }

    return NULL;
}
