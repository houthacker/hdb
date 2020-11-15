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

#define AS_STRING(value)    ((hdb_string_t*)AS_OBJ(value))
#define AS_CSTRING(value)   (((hdb_string_t*)AS_OBJ(value))->chars)

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
 * Definition of the string data type.
 */
typedef struct hdb_string {

    /**
     * Shared information to indicate this is a string.
     */
    hdb_object_t obj;

    /**
     * The length of the string, excluding the terminating '\0'.
     */
    uint32_t length;

    /**
     * The actual characters of the string, including the terminating '\0'.
     */
    char* chars;
} hdb_string_t;

/**
 * Creates a new @c hdb_string on the heap of the HDB Virtual Machine, claiming ownership of @p chars by
 * using the given pointer to create the new @c hdb_string.
 *
 * @param chars The characters to use when creating the new @c hdb_string.
 * @param length The amount of characters to take from @chars, excluding the terminating '\0'.
 * @return A pointer to the new @c hdb_string.
 */
hdb_string_t* hdb_object_take_string(char* chars, uint32_t length);

/**
 * Creates a @c hdb_string on the heap of the HDB Virtual Machine. To create the new @c hdb_string,
 * a copy of the requested characters is made, thus requiring to hdb_free() it.
 *
 * @param chars The source characters to create the new @c hdb_string.
 * @param length The amount of characters to use from @p chars, excluding the terminating '\0'.
 * @return A pointer to the new @c hdb_string.
 */
hdb_string_t* hdb_object_copy_string(const char* chars, uint32_t length);

static inline bool hdb_is_object_type(hdb_value_t value, hdb_object_type_t type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //HDB_OBJECT_H
