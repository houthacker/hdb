#include "common.h"
#include "chunk.h"
#include "debug.h"

int main() {
    Chunk chunk;
    initChunk(&chunk);

    //writeConstant(&chunk, 1.2, 123);
    for (int i = 0; i < 260; i++) {
        writeConstant(&chunk, 1.0 * i, i + 1);
    }
    writeChunk(&chunk, OP_RETURN, 261);

    disassembleChunk(&chunk, "test chunk");

    freeChunk(&chunk);

    return 0;
}
