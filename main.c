#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "memory.h"

int main() {
    hdb_chunk_t chunk;
    hdb_chunk_init(&chunk);

    //hdb_chunk_write_constant(&chunk, 1.2, 123);
    for (int i = 0; i < 260; i++) {
        hdb_chunk_write_constant(&chunk, 1.0 * i, i + 1);
    }
    hdb_chunk_write(&chunk, OP_RETURN, 261);

    hdb_dbg_disassemble_chunk(&chunk, "test chunk");

    hdb_chunk_free(&chunk);

    return 0;
}
