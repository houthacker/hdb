#include <string.h>

#include "memory.h"
#include "object.h"
#include "vm.h"

static void init_object(hdb_object_t* object, hdb_object_type_t type) {
    object->type = type;
    hdb_vm_notify_new(object);
}

static hdb_object_t* allocate_object(size_t size, hdb_object_type_t type) {
    hdb_object_t* object = (hdb_object_t*)hdb_malloc(size);
    init_object(object, type);

    return object;
}

hdb_string_t* hdb_object_create_string(uint32_t length) {
    hdb_string_t* string = (hdb_string_t*)allocate_object(sizeof(hdb_string_t) + length + 1, OBJ_STRING);
    string->length = length;

    return string;
}

hdb_string_t* hdb_object_copy_string(const char* chars, uint32_t length) {
    if (chars) {
        hdb_string_t* string = hdb_object_create_string(length);

        memcpy(string->chars, chars, length);
        string->chars[length] = '\0';

        return string;
    }

    return NULL;
}
