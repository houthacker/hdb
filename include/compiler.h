#ifndef HDB_COMPILER_H
#define HDB_COMPILER_H

#include "chunk.h"

void hdb_compiler_init(void);

void hdb_compiler_free(void);

bool hdb_compiler_compile(const char* source, hdb_chunk_t* chunk);

#endif //HDB_COMPILER_H
