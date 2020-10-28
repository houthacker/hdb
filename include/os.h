#ifndef HDB_OS_H
#define HDB_OS_H

#include <stddef.h> // size_t

/**
 * Tries to allocate the requested memory. If allocation fails, this method will terminate the current process.
 *
 * \param size The amount of bytes to allocate.
 * \return The uninitialized, allocated bytes.
 */
void* os_malloc(size_t size);

/**
 * Frees the memory space pointed to by ptr, which must have been returned by previous call to \c os_malloc().
 * Otherwise, or if \c os_free(ptr) has already been called before, undefined behaviour occurs. If \c ptr is \c NULL,
 * no operation is performed.
 *
 * \param ptr The pointer to the memory to free.
 */
void os_free(void* ptr);

/**
 * Terminates the current process.
 */
void os_abort();

#endif //HDB_OS_H
