#include "line.h"
#include "chunk.h"
#include "memory.h"

void hdb_chunk_init(hdb_chunk_t *chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    hdb_line_array_init(&chunk->lines);
    hdb_init_value_array(&chunk->constants);
}

void hdb_chunk_free(hdb_chunk_t *chunk) {
    HDB_FREE_ARRAY(uint8_t, chunk->code);
    hdb_line_array_free(&chunk->lines);
    hdb_free_value_array(&chunk->constants);
    hdb_chunk_init(chunk);
}

void hdb_chunk_write(hdb_chunk_t *chunk, uint8_t byte, int32_t line) {
    if (chunk->capacity < chunk->count + 1) {
        int32_t oldCapacity = chunk->capacity;
        chunk->capacity = HDB_GROW_CAPACITY(oldCapacity);
        chunk->code = HDB_GROW_ARRAY(uint8_t, chunk->code, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    hdb_line_encode(&chunk->lines, line);
    chunk->count++;
}

void hdb_chunk_write_constant(hdb_chunk_t *chunk, hdb_value_t value, int32_t line) {
    hdb_write_value_array(&chunk->constants, value);
    int32_t idx = chunk->constants.count - 1;

    if (idx < 256) {
        hdb_chunk_write(chunk, OP_CONSTANT, line);
        hdb_chunk_write(chunk, (uint8_t) idx, line);
    } else {
        uint8_t array[3] = {
                (idx >> 16) & 0xff,
                (idx >> 8) & 0xff,
                idx & 0xff
        };

        hdb_chunk_write(chunk, OP_CONSTANT_LONG, line);
        hdb_chunk_write(chunk, array[0], line);
        hdb_chunk_write(chunk, array[1], line);
        hdb_chunk_write(chunk, array[2], line);
    }
}

hdb_value_t hdb_chunk_read_constant(hdb_chunk_t *chunk, uint8_t *code_pointer) {
    const uint8_t op = *(code_pointer - 1);

    if (op == OP_CONSTANT) {
        return chunk->constants.values[*code_pointer];
    }

    uint8_t b2 = *++code_pointer;
    uint8_t b1 = *++code_pointer;
    uint8_t b0 = *code_pointer;
    uint8_t array[4] = {b0, b1, b2, 0};

    int32_t index = *(int32_t*)array;
    return chunk->constants.values[index];
}
