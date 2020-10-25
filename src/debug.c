//
// Created by houthacker on 24-10-20.
//
#include <stdio.h>

#include "debug.h"
#include "value.h"
#include "line.h"

static void printValue(Value value) {
    printf("%g", value);
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");

    return offset + 2;
}

static int constantLongInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t array[4] = {
            chunk->code[offset + 3],
            chunk->code[offset + 2],
            chunk->code[offset + 1],
            0
    };
    int index = *(int*)array;
    printf("%-16s %4d '", name, index);
    printValue(chunk->constants.values[index]);
    printf("'\n");

    return offset + 4;
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

void disassembleChunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);
    if (offset > 0 &&
            decodeLine(&chunk->lines, offset) == decodeLine(&chunk->lines, offset - 1)) {
        printf("   | ");
    } else {
        printf("%4d ", decodeLine(&chunk->lines, offset));
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_CONSTANT_LONG:
            return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
