#ifndef HDB_OS_H
#define HDB_OS_H

#include <stddef.h> // size_t
#include <stdint.h> // int types

/**
 * Tries to allocate the requested memory. If allocation fails, this method will terminate the current process.
 *
 * \param size The amount of bytes to allocate.
 * \return The uninitialized, allocated bytes.
 */
void* os_malloc(size_t size);

/**
 * Changes the size of the memory block pointed to by ptr to size bytes. The contents will be unchanged in
 * the range from the start of the region up to the minimum of the old and new sizes.
 * If the new size is larger than the old size, the added memory will not be initialized.
 * If ptr is NULL, then the call is equivalent to os_malloc(size), for all values of size; if size is equal to zero,
 * and ptr is not NULL, then the call is equivalent to os_free(ptr). Unless ptr is NULL, it must have been returned
 * by an earlier call to os_malloc() or os_realloc(). If the area pointed to was moved, an os_free(ptr) is
 * done.
 * If allocating the new memory fails, this method will terminate the current process.
 *
 * \param ptr The pointer to the memory block to grow.
 * \param size The total new size.
 * \return The pointer to the new allocated memory.
 */
void* os_realloc(void* ptr, size_t size);

/**
 * Frees the memory space pointed to by ptr, which must have been returned by previous call to \c os_malloc().
 * Otherwise, or if \c os_free(ptr) has already been called before, undefined behaviour occurs. If \c ptr is \c NULL,
 * no operation is performed.
 *
 * \param ptr The pointer to the memory to free.
 */
void os_free(void* ptr);

/**
 * Sends the given signal to the calling process or thread.
 * \param signal The signal to send.
 * \return 0 on success, nonzero on failure.
 */
int32_t os_raise(int32_t signal);

/**
 * Terminates the current process.
 */
void os_abort();

#endif //HDB_OS_H
