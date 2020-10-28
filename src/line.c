#include <stdlib.h>

#include "line.h"
#include "memory.h"

void initLineArray(LineArray* lineArray) {
    lineArray->count = 0;
    lineArray->capacity = 0;
    lineArray->lines = NULL;
}

void freeLineArray(LineArray* lineArray) {
    HDB_FREE_ARRAY(Line, lineArray->lines);
    initLineArray(lineArray);
}

static void grow(LineArray* lineArray) {
    lineArray->capacity = HDB_GROW_CAPACITY(lineArray->capacity);
    lineArray->lines = HDB_GROW_ARRAY(Line, lineArray->lines, lineArray->capacity);
}

static int compareLines(const void* left, const void* right) {
    Line* l = (Line*)left;
    Line* r = (Line*)right;

    if (l->line == r->line) { return 0; }
    else if (l->line < r->line) { return -1; }
    else return 1;
}

static Line createLine(int line) {
    Line l;
    l.line = line;
    l.instructionCount = 1;

    return l;
}

int encodeLine(LineArray* lineArray, int line) {
    if (lineArray->count == 0 || lineArray->capacity < lineArray->count + 1) {
        grow(lineArray);
    }

    if (lineArray->count == 0) {
        lineArray->lines[0] = createLine(line);
        return lineArray->count++;
    } else {
        Line *lastLine = &lineArray->lines[lineArray->count - 1];

        if (lastLine->line == line) {
            lastLine->instructionCount++;
            return lineArray->count - 1;

        } else if (lastLine->line < line) {
            lineArray->lines[lineArray->count] = createLine(line);
            return lineArray->count++;
        } else {
            // This is the most expensive case, because
            // ordering is enforced here.
            // If lines are added in order, this code is never executed.
            // Try to find line, otherwise append
            for (int i = 0; i < lineArray->count; i++) {
                Line* element = &lineArray->lines[i];
                if (element->line == line) {
                    element->instructionCount++;
                    return i;
                }
            }

            lineArray->lines[lineArray->count] = createLine(line);
            qsort(lineArray->lines, lineArray->count + 1, sizeof(Line), compareLines);

            return lineArray->count++;
        }
    }
}

int decodeLine(LineArray* lineArray, int instructionIndex) {
    if (instructionIndex < 0) {
        return -1;
    }

    int maxIndex = -1;

    for (int i = 0; i < lineArray->count; i++) {
        Line* l = &lineArray->lines[i];
        maxIndex += l->instructionCount;

        if (maxIndex >= instructionIndex) {
            return l->line;
        }
    }

    return -1;
}