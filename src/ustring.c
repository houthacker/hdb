#include <string.h>
#include <stddef.h> // offsetof()

#include "ustring.h"

static size_t byte_length(const char* chars, size_t unit_length) {
    int32_t bytes = 0;
    size_t units = 0;
    while (chars[bytes] && units <= unit_length) {
        if ((chars[bytes] & 0xc0) != 0x80) {
            // Start of code point
            if (++units > unit_length) {
                return bytes;
            }
        }
        bytes++;
    }

    return bytes;
}

static size_t unit_length(const char* chars, size_t byte_length) {
    size_t unit_length = 0;
    for (int i = 0; i < byte_length; i++) {
        if ((chars[i] & 0xc0) != 0x80) { unit_length++; }
    }

    return unit_length;
}

static hdb_ustring_t* ustring_create(const char* chars, size_t len, size_t units) {
    hdb_ustring_t* string = (hdb_ustring_t *) hdb_object_create(
            sizeof(hdb_ustring_t) + len + 1, OBJ_STRING);
    string->length = units;
    string->byte_length = len;

    char* content = (char*)(string + offsetof(hdb_ustring_t, chars));
    memcpy(content, chars, len);
    content[len] = '\0';
    string->chars = content;

    return string;
}

const hdb_ustring_t* hdb_ustring_create(const char* chars) {
    if (chars) {
        size_t len = strlen(chars);
        size_t units = unit_length(chars, len);
        return ustring_create(chars, len, units);
    }

    return NULL;
}

const hdb_ustring_t* hdb_ustring_ncreate(const char* chars, size_t units) {
    if (chars) {
        size_t len = byte_length(chars, units);
        return ustring_create(chars, len, units);
    }

    return NULL;
}

const hdb_ustring_t* hdb_ustring_concatenate(const hdb_ustring_t* left, const hdb_ustring_t* right) {
    if (left && right) {
        size_t len = left->byte_length + right->byte_length;
        size_t units = left->length + right->length;

        char concat[left->byte_length + right->byte_length + 1];
        memcpy(concat, left->chars, left->byte_length);
        memcpy(concat + left->byte_length, right->chars, right->byte_length);

        return ustring_create(concat, len, units);
    } else if (left) {
        return left;
    }

    return right;
}
