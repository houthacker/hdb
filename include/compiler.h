#ifndef HDB_COMPILER_H
#define HDB_COMPILER_H

void hdb_compiler_init(void);

void hdb_compiler_free(void);

void hdb_compiler_compile(const char* source);

#endif //HDB_COMPILER_H
