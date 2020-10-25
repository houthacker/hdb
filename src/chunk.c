#include "line.h"
#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    initLineArray(&chunk->lines);
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    freeLineArray(&chunk->lines);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    encodeLine(&chunk->lines, line);
    chunk->count++;
}

void writeConstant(Chunk* chunk, Value value, int line) {
    writeValueArray(&chunk->constants, value);
    int idx = chunk->constants.count - 1;

    if (idx < 256) {
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, (uint8_t) idx, line);
    } else {
        uint8_t array[3] = {
                (idx >> 16) & 0xff,
                (idx >> 8) & 0xff,
                idx & 0xff
        };

        writeChunk(chunk, OP_CONSTANT_LONG, line);
        writeChunk(chunk, array[0], line);
        writeChunk(chunk, array[1], line);
        writeChunk(chunk, array[2], line);
    }
}
