#include <stdlib.h>

#include "line.h"
#include "memory.h"

void init_line_array(line_array_t* array) {
    array->count = 0;
    array->capacity = 0;
    array->lines = NULL;
}

void free_line_array(line_array_t* array) {
    HDB_FREE_ARRAY(Line, array->lines);
    init_line_array(array);
}

static void grow(line_array_t* lineArray) {
    lineArray->capacity = HDB_GROW_CAPACITY(lineArray->capacity);
    lineArray->lines = HDB_GROW_ARRAY(line_t, lineArray->lines, lineArray->capacity);
}

static int compareLines(const void* left, const void* right) {
    line_t* l = (line_t*)left;
    line_t* r = (line_t*)right;

    if (l->line == r->line) { return 0; }
    else if (l->line < r->line) { return -1; }
    else return 1;
}

static line_t createLine(int line) {
    line_t l;
    l.line = line;
    l.instructionCount = 1;

    return l;
}

int encode_line(line_array_t* array, int line) {
    if (array->count == 0 || array->capacity < array->count + 1) {
        grow(array);
    }

    if (array->count == 0) {
        array->lines[0] = createLine(line);
        return array->count++;
    } else {
        line_t *lastLine = &array->lines[array->count - 1];

        if (lastLine->line == line) {
            lastLine->instructionCount++;
            return array->count - 1;

        } else if (lastLine->line < line) {
            array->lines[array->count] = createLine(line);
            return array->count++;
        } else {
            // This is the most expensive case, because
            // ordering is enforced here.
            // If lines are added in order, this code is never executed.
            // Try to find line, otherwise append
            for (int i = 0; i < array->count; i++) {
                line_t* element = &array->lines[i];
                if (element->line == line) {
                    element->instructionCount++;
                    return i;
                }
            }

            array->lines[array->count] = createLine(line);
            qsort(array->lines, array->count + 1, sizeof(line_t), compareLines);

            return array->count++;
        }
    }
}

int decode_line(line_array_t* array, int instruction_index) {
    if (instruction_index < 0) {
        return -1;
    }

    int maxIndex = -1;

    for (int i = 0; i < array->count; i++) {
        line_t* l = &array->lines[i];
        maxIndex += l->instructionCount;

        if (maxIndex >= instruction_index) {
            return l->line;
        }
    }

    return -1;
}