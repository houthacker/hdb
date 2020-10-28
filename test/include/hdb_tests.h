#ifndef HDB_HDB_TESTS_H
#define HDB_HDB_TESTS_H

/**
 * Casts the given C99 \c void* to a \c hdb_memory_block_t*
 */
#define HDB_CPP_BLOCK_PTR(ptr) \
    ptr == nullptr ? nullptr : (hdb_memory_block_t*)(static_cast<char*>(ptr) - sizeof(hdb_memory_block_t))

#endif //HDB_HDB_TESTS_H
