/**
 * Data structures and methods for UTF-8 strings.
 *
 * @since 0.0.1
 * @author houthacker
 */
#ifndef HDB_USTRING_H
#define HDB_USTRING_H

#include "value.h"
#include "object.h"

#define AS_STRING(value)    ((hdb_ustring_t*)AS_OBJ(value))
#define AS_CSTRING(value)   (((hdb_ustring_t*)AS_OBJ(value))->chars)

/**
 * Definition of the string data type. All @c hdb_ustring_t instances are UTF-8 strings. If the @c length
 * is equal to the @c byte_length however, the characters of this string can be treated as a regular ASCII string.
 */
typedef struct hdb_ustring {

    /**
     * Shared object information to indicate this is a string.
     */
    hdb_object_t obj;

    /**
     * The amount of text units, excluding the terminating '\0'.
     */
    size_t length;

    /**
     * The amount of bytes in the string, excluding the terminating '\0'.
     */
    size_t byte_length;

    /**
     * The actual characters of the string, including the terminating '\0'.
     */
    const char* chars;
} hdb_ustring_t;

/**
 * Creates a new @c hdb_ustring_t on the heap of the HDB Virtual Machine, while wrapping the given
 * characters. These characters will not be freed.
 *
 * @param chars The source characters to wrap in a @c hdb_ustring_t.
 * @return A pointer to the new @c hdb_ustring_t.
 */
const hdb_ustring_t* hdb_ustring_create(const char* chars);

/**
 * Creates a new @c hdb_ustring_t on the heap of the HDB Virtual Machine, while wrapping the given
 * characters. These characters will not be freed.
 *
 * @param chars The source characters to wrap in a @c hdb_ustring_t.
 * @param units The amount of code units to use from the given character array.
 * @return A pointer to the new @c hdb_ustring_t.
 */
const hdb_ustring_t* hdb_ustring_ncreate(const char* chars, size_t units);

/**
 * Concatenates the two given strings and returns the result.
 *
 * @param left The string to put on the left side.
 * @param right The string to put on the right side.
 * @return The concatenated string.
 */
const hdb_ustring_t* hdb_ustring_concatenate(const hdb_ustring_t* left, const hdb_ustring_t* right);

#endif //HDB_USTRING_H
