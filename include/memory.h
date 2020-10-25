/**
 * Base memory operations
 *
 * \since 0.0.1
 * \author houthacker
 */
#ifndef HDB_MEMORY_H
#define HDB_MEMORY_H

#include "common.h"

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * oldCount, \
        sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

/**
 * Allocates \code newSize\endcode bytes of memory preserving and returns a pointer to it. The data in
 * \code pointer\endcode is preserved.
 * If a \code newSize\endcode of 0 is provided, the given pointer is freed.
 *
 * \param pointer The pointer to the old blob of data.
 * \param oldSize The current size of data in bytes.
 * \param newSize The requested new data size in bytes.
 * \return A pointer to the new data, or \code NULL\endcode if no memory has been allocated.
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif //HDB_MEMORY_H
