#include <stdio.h>

#include "debug.h"
#include "value.h"
#include "line.h"

static int constant_instruction(const char* name, hdb_chunk_t* chunk, int32_t offset) {
    uint8_t index = chunk->code[offset + 1];
    printf("%-16s %4d '", name, index);
    hdb_dbg_print_value(chunk->constants.values[index]);
    printf("'\n");

    return offset + 2;
}

static int constant_long_instruction(const char* name, hdb_chunk_t* chunk, int32_t offset) {
    uint8_t array[4] = {
            chunk->code[offset + 3],
            chunk->code[offset + 2],
            chunk->code[offset + 1],
            0
    };
    int32_t index = *(int32_t*)array;
    printf("%-16s %4d '", name, index);
    hdb_dbg_print_value(chunk->constants.values[index]);
    printf("'\n");

    return offset + 4;
}

static int simple_instruction(const char* name, int32_t offset) {
    printf("%s\n", name);
    return offset + 1;
}

void hdb_dbg_disassemble_chunk(hdb_chunk_t* chunk, const char* name) {
    printf("== %s ==\n", name);

    for (int32_t offset = 0; offset < chunk->count;) {
        offset = hdb_dbg_disassemble_instruction(chunk, offset);
    }
}

int hdb_dbg_disassemble_instruction(hdb_chunk_t* chunk, int32_t offset) {
    printf("%04d ", offset);
    if (offset > 0 &&
            hdb_line_decode(&chunk->lines, offset) == hdb_line_decode(&chunk->lines, offset - 1)) {
        printf("   | ");
    } else {
        printf("%4d ", hdb_line_decode(&chunk->lines, offset));
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset);
        case OP_CONSTANT_LONG:
            return constant_long_instruction("OP_CONSTANT_LONG", chunk, offset);
        case OP_NULL:
            return simple_instruction("OP_NULL", offset);
        case OP_TRUE:
            return simple_instruction("OP_TRUE", offset);
        case OP_FALSE:
            return simple_instruction("OP_FALSE", offset);
        case OP_EQUAL:
            return simple_instruction("OP_EQUAL", offset);
        case OP_NOT_EQUAL:
            return simple_instruction("OP_NOT_EQUAL", offset);
        case OP_LESS:
            return simple_instruction("OP_LESS", offset);
        case OP_LESS_EQUAL:
            return simple_instruction("OP_LESS_EQUAL", offset);
        case OP_GREATER:
            return simple_instruction("OP_GREATER", offset);
        case OP_GREATER_EQUAL:
            return simple_instruction("OP_GREATER_EQUAL", offset);
        case OP_ADD:
            return simple_instruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simple_instruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset);
        case OP_NOT:
            return simple_instruction("OP_NOT", offset);
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset);
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

void hdb_dbg_print_value(hdb_value_t value) {
    switch (value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false"); break;
        case VAL_NULL: printf("null"); break;
        case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
    }
}
