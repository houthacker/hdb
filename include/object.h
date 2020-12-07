/**
 * Data structures and operations related to heap allocated types within the HDB Virtual Machine.
 *
 * \since 0.0.1
 * \author houthacker
 */
#ifndef HDB_OBJECT_H
#define HDB_OBJECT_H

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value)     (AS_OBJ(value)->type)
#define IS_STRING(object)   hdb_is_object_type(object, OBJ_STRING)

/**
 * Defines the currently supported object types.
 *
 * Note that all methods in this file assume that the HDB Virtual Machine is initialized. If this is not the case,
 * they will cause a SIGSEGV.
 */
typedef enum {

    /**
     * Indicates the type of an @c hdb_object_t
     */
    OBJ_STRING,
} hdb_object_type_t;

/**
 * Shared data structure for all object types within the HDB Virtual Machine.
 */
typedef struct hdb_object {

    /**
     * The type of object.
     */
    hdb_object_type_t type;

    /**
     * The @c hdb_object that was created just before this @c hdb_object.
     * This is used by the Garbage Collector to walk the object tree.
     */
    struct hdb_object* next;
} hdb_object_t;

/**
 * Creates a new @c hdb_object_t and notifies the HDB Virtual Machine.
 *
 * @param size The amount of bytes to allocate for this object.
 * @param type The type of object to create.
 * @return
 */
hdb_object_t* hdb_object_create(size_t size, hdb_object_type_t type);

static inline bool hdb_is_object_type(hdb_value_t value, hdb_object_type_t type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //HDB_OBJECT_H
