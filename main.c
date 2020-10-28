#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "memory.h"

int main() {
    chunk_t chunk;
    init_chunk(&chunk);

    //write_constant(&chunk, 1.2, 123);
    for (int i = 0; i < 260; i++) {
        write_constant(&chunk, 1.0 * i, i + 1);
    }
    write_chunk(&chunk, OP_RETURN, 261);

    disassembleChunk(&chunk, "test chunk");

    free_chunk(&chunk);

    return 0;
}
