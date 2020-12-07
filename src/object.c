#include <string.h>

#include "memory.h"
#include "object.h"
#include "vm.h"

static void init_object(hdb_object_t* object, hdb_object_type_t type) {
    object->type = type;
    hdb_vm_notify_new(object);
}

hdb_object_t* hdb_object_create(size_t size, hdb_object_type_t type) {
    hdb_object_t* object = (hdb_object_t*)hdb_malloc(size);
    init_object(object, type);

    return object;
}


