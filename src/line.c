#include <stdlib.h>

#include "line.h"
#include "memory.h"

void hdb_line_array_init(hdb_line_array_t* array) {
    array->count = 0;
    array->capacity = 0;
    array->lines = NULL;
}

void hdb_line_array_free(hdb_line_array_t* array) {
    HDB_FREE_ARRAY(hdb_line_t, array->lines);
    hdb_line_array_init(array);
}

static void grow(hdb_line_array_t* lines) {
    lines->capacity = HDB_GROW_CAPACITY(lines->capacity);
    lines->lines = HDB_GROW_ARRAY(hdb_line_t, lines->lines, lines->capacity);
}

static int compare_lines(const void* left, const void* right) {
    hdb_line_t* l = (hdb_line_t*)left;
    hdb_line_t* r = (hdb_line_t*)right;

    if (l->line == r->line) { return 0; }
    else if (l->line < r->line) { return -1; }
    else return 1;
}

static hdb_line_t create_line(int32_t line) {
    hdb_line_t l;
    l.line = line;
    l.instruction_count = 1;

    return l;
}

int hdb_line_encode(hdb_line_array_t* array, int32_t line) {
    if (array->count == 0 || array->capacity < array->count + 1) {
        grow(array);
    }

    if (array->count == 0) {
        array->lines[0] = create_line(line);
        return array->count++;
    } else {
        hdb_line_t *lastLine = &array->lines[array->count - 1];

        if (lastLine->line == line) {
            lastLine->instruction_count++;
            return array->count - 1;

        } else if (lastLine->line < line) {
            array->lines[array->count] = create_line(line);
            return array->count++;
        } else {
            // This is the most expensive case, because
            // ordering is enforced here.
            // If lines are added in order, this code is never executed.
            // Try to find line, otherwise append
            for (int32_t i = 0; i < array->count; i++) {
                hdb_line_t* element = &array->lines[i];
                if (element->line == line) {
                    element->instruction_count++;
                    return i;
                }
            }

            array->lines[array->count] = create_line(line);
            qsort(array->lines, array->count + 1, sizeof(hdb_line_t), compare_lines);

            return array->count++;
        }
    }
}

int hdb_line_decode(hdb_line_array_t* array, int32_t instruction_index) {
    if (instruction_index < 0) {
        return -1;
    }

    int maxIndex = -1;

    for (int32_t i = 0; i < array->count; i++) {
        hdb_line_t* l = &array->lines[i];
        maxIndex += l->instruction_count;

        if (maxIndex >= instruction_index) {
            return l->line;
        }
    }

    return -1;
}