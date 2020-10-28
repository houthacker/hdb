/**
 * Contains data structures and operations around the HDB Heap.
 *
 * \since 0.0.1
 * \author houthacker
 */
#ifndef HDB_MEMORY_H
#define HDB_MEMORY_H

#include "common.h"

#define HDB_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define HDB_GROW_ARRAY(type, pointer, newCount) \
    (type*)hdb_reallocate(pointer, sizeof(type) * (newCount))

#define HDB_FREE_ARRAY(type, pointer) \
    hdb_reallocate(pointer, 0)

#define HDB_HEAP_INITIAL_MIN_SIZE 8 * 1024 * 1024 // 8MB
#define HDB_HEAP_PAGE_SIZE sizeof(hdb_memory_block_t)
#define HDB_HEAP_INCREASE_SIZE 8 * 1024 * 1024 // 8MB
#define HDB_HEAP_RETURN_BARRIER 8 * 1024 * 1024 // 8MB

#define HDB_MEMORY_PTR(ptr)  \
    ((void*)(ptr) + sizeof(hdb_memory_block_t))

#define HDB_BLOCK_PTR(ptr) \
    ((void*)(ptr) - sizeof(hdb_memory_block_t))

/**
 * A header which stores size information about the adjacent blob of data.
 */
typedef struct hdb_memory_block {

    /**
     * The size of the data, including the size of this struct itself.
     */
    size_t size;

    /**
     * Pointer to the next block of memory.
     */
    struct hdb_memory_block *next;

    /**
     * Pointer to the previous block of memory.
     */
    struct hdb_memory_block *prev;
} hdb_memory_block_t;

/**
 * Represents an unmodifiable view of the \c hdb_memory_block_t struct.
 */
typedef struct hdb_memory_block_view {

    /**
     * The size of the data, including the size of this struct itself.
     */
    const size_t size;

    /**
     * Pointer to the next block of memory.
     */
    struct hdb_memory_block_view *const next;

    /**
     * Pointer to the previous block of memory.
     */
    struct hdb_memory_block_view *const prev;
} hdb_memory_block_view_t;

/**
 * This structure describes the heap as it is implemented in hdb.
 */
typedef struct hdb_heap {

    /**
     * The minimum amount of available bytes of memory in this heap.
     */
    const size_t min_size;

    /**
     * The maximum amount of available bytes of memory in this heap.
     */
    const size_t max_size;

    /**
     * The current size in bytes of memory in this heap.
     */
    size_t current_size;

    /**
     * The total amount of memory which is currently free.
     */
    size_t current_free;

    /**
     * A pointer to the head of the list of free blocks of memory.
     */
    hdb_memory_block_t* free_blocks;

    /**
     * A pointer to the memory to be freed when the heap is destroyed.
     */
    void* free_ptr;
} hdb_heap_t;

/**
 * An unmodifiable view of the \c hdb_heap_t struct.
 */
typedef struct hdb_heap_view {

    /**
     * The minimum amount of available bytes of memory in this heap.
     */
    const size_t min_size;

    /**
     * The maximum amount of available bytes of memory in this heap.
     */
    const size_t max_size;

    /**
     * The current size in bytes of memory in this heap.
     */
    const size_t current_size;

    /**
     * The total amount of memory which is currently free.
     */
    const size_t current_free;

    /**
     * A pointer to the head of the list of free blocks of memory.
     */
    hdb_memory_block_view_t* free_blocks;
} hdb_heap_view_t;

/**
 * Initializes the heap for the HDB Virtual Machine. All memory required during the runtime of the HDB Virtual Machine
 * is retrieved from this heap. The initial size of the heap is \c min_size. If the heap has been initialized before,
 * this method just returns a pointer to that heap.
 * If an error occurs during initialization, this method returns \c NULL and sets \c errno accordingly.
 *
 * \param min_size The minimum size of the heap in bytes, must be >= (HDB_HEAP_PAGE_SIZE * 3).
 * \param max_size The maximum size of the heap in bytes, must be larger than \c min_size.
 * \return: A non-modifiable view of the heap.
 */
hdb_heap_view_t* hdb_heap_init(size_t min_size, size_t max_size);

/**
 * \return A non-modifiable view of the current heap.
 */
hdb_heap_view_t* hdb_heap(void);

/**
 * Merges contiguous free blocks into a single block.
 */
void hdb_heap_compact(void);

/**
 * Returns memory claimed by the current heap to the underlying OS. This causes all pointers still in use to become
 * invalid. Any request for new memory using \c hdb_malloc will return \c NULL.
 */
void hdb_heap_free(void);

/**
 * Allocates a new block of uninitialized memory of at least size bytes, aligned to \c HDB_HEAP_PAGE_SIZE bytes.
 * This alignment is to prevent overhead when memory blocks would otherwise have a size less than the related header.
 * If the requested amount of bytes cannot be allocated, \c hdb_malloc returns \c NULL and \c errno is set accordingly.
 *
 * \c hdb_malloc() also returns \c NULL when \c size is \c 0. To differentiate between a possible memory allocation
 * failure and this valid case, check the value of \c errno if \c NULL is returned.
 *
 * \param size The minimum amount of bytes to allocate.
 * \return A pointer to the newly allocated memory, or \c NULL if \c size is 0 or on failure.
 */
void* hdb_malloc(size_t size);

/**
 * Frees the memory space pointed to by ptr, which must have been returned by previous call to \c hdb_malloc().
 * Otherwise, or if \c hdb_free(ptr) has already been called before, undefined behaviour occurs. If \c ptr is \c NULL,
 * no operation is performed.
 *
 * \param ptr The pointer to the memory to free.
 */
void hdb_free(void* ptr);

/**
 * Allocates \c new_size bytes of memory preserving and returns a pointer to it. The data in
 * \c ptr is preserved. If new memory must be allocated but it fails, this method returns \c NULL and
 * \c errno is set accordingly.
 * If a \c newSize of 0 is provided, the given pointer is freed.
 *
 * \param ptr A pointer to a blob of data previously retrieved by \c hdb_malloc().
 * \param new_size The requested new data size in bytes.
 * \return A pointer to the new data, or \c NULL if no memory has been allocated.
 */
void* hdb_reallocate(void* ptr, size_t new_size);

#endif //HDB_MEMORY_H
