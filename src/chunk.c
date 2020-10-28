#include "line.h"
#include "chunk.h"
#include "memory.h"

void init_chunk(chunk_t *const chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    init_line_array(&chunk->lines);
    init_value_array(&chunk->constants);
}

void free_chunk(chunk_t *const chunk) {
    HDB_FREE_ARRAY(uint8_t, chunk->code);
    free_line_array(&chunk->lines);
    free_value_array(&chunk->constants);
    init_chunk(chunk);
}

void write_chunk(chunk_t *const chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = HDB_GROW_CAPACITY(oldCapacity);
        chunk->code = HDB_GROW_ARRAY(uint8_t, chunk->code, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    encode_line(&chunk->lines, line);
    chunk->count++;
}

void write_constant(chunk_t *const chunk, value_t value, int line) {
    write_value_array(&chunk->constants, value);
    int idx = chunk->constants.count - 1;

    if (idx < 256) {
        write_chunk(chunk, OP_CONSTANT, line);
        write_chunk(chunk, (uint8_t) idx, line);
    } else {
        uint8_t array[3] = {
                (idx >> 16) & 0xff,
                (idx >> 8) & 0xff,
                idx & 0xff
        };

        write_chunk(chunk, OP_CONSTANT_LONG, line);
        write_chunk(chunk, array[0], line);
        write_chunk(chunk, array[1], line);
        write_chunk(chunk, array[2], line);
    }
}
