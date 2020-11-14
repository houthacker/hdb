#include <errno.h>
#include <unistd.h> // for sbrk()
#include <string.h> // for memcpy()

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#include <sys/sysinfo.h> // sysinfo()
#endif

#include "os.h"
#include "memory.h"

hdb_heap_t* heap;

static void free_list_remove(hdb_memory_block_t* block) {
    if (!heap || !block) {
        return;
    }

    if (!block->prev) {
        if (block->next) {
            heap->free_blocks = block->next;
        } else {
            heap->free_blocks = NULL;
        }
    } else {
        block->prev->next = block->next;
    }
    if (block->next) {
        block->next->prev = block->prev;
    }

    heap->current_free -= block->size;
}

static void free_list_add(hdb_memory_block_t* block) {
    if (!heap || !block) {
        return;
    }

    block->prev = NULL;
    block->next = NULL;
    if (!heap->free_blocks || heap->free_blocks > block) {
        if (heap->free_blocks) {
            heap->free_blocks->prev = block;
        }
        block->next = heap->free_blocks;
        heap->free_blocks = block;
    } else {
        hdb_memory_block_t* current = heap->free_blocks;
        while (current->next && current->next < block) {
            current = current->next;
        }

        block->next = current->next;
        block->prev = current;
        current->next = block;
    }

    heap->current_free += block->size;
}

/*
 * Aligns the given value to the nearest power of two.
 */
static size_t align_pow2(size_t value) {
#ifdef HDB_TESTING_NOALIGN
    return value;
#else

    if (value && !(value & (value - 1))) {
        return value;
    }

    uint8_t count = 0;
    while (value) {
        value >>= 1;
        count++;
    }

    return 1 << count;
#endif
}

static size_t initial_heap_size(size_t requested) {
#ifdef __APPLE__
    int32_t mib[] = {CTL_HW, HW_MEMSIZE};
    int64_t total_ram;
    size_t length = sizeof(total_ram);

    int32_t result = sysctl(mib, 2, &total_ram, &length, NULL, 0);
#else
    struct sysinfo sys_info;
    int32_t result = sysinfo(&sys_info);

    int64_t total_ram = sys_info.totalram;
#endif

    size_t aligned_requested = requested < HDB_HEAP_INITIAL_MIN_SIZE
            ? HDB_HEAP_INITIAL_MIN_SIZE
            : align_pow2(requested);

    if (result == 0) {
        size_t sixty_fourth = align_pow2(total_ram / 64);

        return sixty_fourth > aligned_requested ? sixty_fourth : aligned_requested;
    }

    return aligned_requested;
}

/*
 * Splits the given block by creating a new block after size bytes, and resizes the given block
 * accordingly. The newly created block is returned. This method assumes the given block contains enough space
 * to facilitate a split of split_size.
 *
 * example:
 * size = 5
 * sizeof(header) = 8
 * min_block_size = align(size, align(sizeof(header))) + sizeof(header) = 8 + 8 = 16
 * min_splittable_size = min_block_size + sizeof(header) + MIN_BLOCK_BYTES = 16 + 8 + 8 = 32
 * ptr->size = 25
 * before split:
 *  -------------------------------------------------------------------------------------------------
 *  |       HEADER       |  |                                 DATA                                  |
 *  -------------------------------------------------------------------------------------------------
 *  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
 *
 * split size = ptr->size - min_block_size = 26 - 16 = 10
 * after split:
 *  -------------------------------------------------------------------------------------------------
 *  |       HEADER       |  |       DATA        |  |       HEADER       |  |         DATA           |
 *  -------------------------------------------------------------------------------------------------
 *  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
 * header(0) =  0 -  7  (8)
 * data(0)   =  8 - 15  (8)
 * header(1) = 16 - 23  (8)
 * data(1)   = 24 - 32  (9)
 */
static hdb_memory_block_t* split(hdb_memory_block_t* block, size_t split_size) {
    // Create a new block starting at split_size
    hdb_memory_block_t* split = (hdb_memory_block_t*)(block + (block->size - split_size));

    // Initialize the new block. Get size by subtracting required size from block size.
    split->size = split_size;
    split->next = NULL;
    split->prev = NULL;

    block->size -= split->size;

    return split;
}

static void merge_if_continuous(hdb_memory_block_t* left, hdb_memory_block_t* right) {
    if (left + left->size == right) { // merge only continuous memory regions
        left->size += right->size;
        left->next = right->next;

        merge_if_continuous(left, left->next); // recursion
    }
}

hdb_heap_view_t* hdb_heap_init(size_t min_size, size_t max_size) {
    if (max_size < min_size || min_size == 0) {
        errno = EINVAL;
        return NULL;
    }

    const size_t actual_min_size = initial_heap_size(min_size);
    const size_t actual_max_size = max_size > actual_min_size ? align_pow2(max_size) : actual_min_size;
    hdb_heap_t init = {
            .min_size = actual_min_size,
            .max_size = actual_max_size,
            .current_size = 0,
            .current_free = 0,
            .free_blocks = NULL
    };

    heap = (hdb_heap_t *)os_malloc(sizeof(hdb_heap_t));
    if (heap == NULL) {
        return NULL;
    }
    memcpy(heap, &init, sizeof(hdb_heap_t));

    void* memory = os_malloc(heap->min_size);
    if (memory == NULL) {
        return NULL;
    }

    // Initialize heap memory
    hdb_memory_block_t* block = (hdb_memory_block_t*)memory;
    block->next = NULL;
    block->prev = NULL;
    block->size = heap->min_size;

    heap->current_size = heap->min_size;
    heap->current_free = heap->min_size;
    heap->free_blocks = block;
    heap->free_ptr = block;

    return (hdb_heap_view_t*)heap;
}

hdb_heap_view_t* hdb_heap(void) {
    return (hdb_heap_view_t*)heap;
}

void hdb_heap_compact() {
    hdb_memory_block_t* current_block = heap->free_blocks;
    while (current_block) {
        hdb_memory_block_t* next_block = current_block->next;

        if (next_block) {
            merge_if_continuous(current_block, next_block);
            current_block = current_block->next;
        } else {
            current_block = next_block;
        }
    }
}

void hdb_heap_free() {
    if (heap) {
        os_free(heap->free_ptr);
        os_free(heap);
        heap = NULL;
    }
}

void* hdb_malloc(size_t size) {
    if (!heap || size == 0) {
        return NULL;
    }

    const size_t block_size = align_pow2(size + HDB_HEAP_PAGE_SIZE);

    // To avoid cases where aligned_size < sizeof(hdb_memory_block_t).
    const size_t min_splittable_size = block_size + align_pow2(HDB_HEAP_PAGE_SIZE * 2);

    void* mem_pointer;
    hdb_memory_block_t *ptr;
    ptr = heap->free_blocks;

    while (ptr) {
        if (ptr->size >= block_size) {
            free_list_remove(ptr);
            mem_pointer = HDB_MEMORY_PTR(ptr);

            if (ptr->size >= min_splittable_size) {

                // Split off extraneous bytes.
                free_list_add(split(ptr, ptr->size - block_size));
            }

            // Block is larger than what we need, but cannot split, because another hdb_memory_block_t and
            // at least one byte user memory don't fit in the extraneous bytes.
            // This keeps the extraneous amount of bytes to a minimum.
            return mem_pointer;
        }

        ptr = ptr->next;
    }

    // Try to grow heap, but never over configured limit.
    if (heap->current_size < heap->max_size) {
        size_t increase_size = HDB_HEAP_INCREASE_SIZE;

        // Never allocate over heap->max_size.
        if (heap->current_size + increase_size > heap->max_size) {
            increase_size = heap->max_size - heap->current_size;
        }

        if (increase_size > 0) {
            hdb_memory_block_t *new_block = (hdb_memory_block_t *) sbrk(increase_size);
            if (new_block) {
                new_block->next = NULL;
                new_block->prev = NULL;
                new_block->size = increase_size;
                free_list_add(new_block);
            }

            heap->current_size += increase_size;
            return hdb_malloc(size); // recursion
        }
    }

    os_abort();
}

void hdb_free(void* ptr) {
    if (heap && ptr) {
        hdb_memory_block_t* block = HDB_BLOCK_PTR(ptr);

        free_list_add(block);
    }
}

void* hdb_reallocate(void* ptr, size_t new_size) {
    if (!heap) {
        return NULL;
    } else if (!ptr) {
        return hdb_malloc(new_size);
    }

    if (new_size == 0) {
        hdb_free(ptr);
        return NULL;
    }

    hdb_memory_block_t* block = HDB_BLOCK_PTR(ptr);
    if (block->size >= new_size) {
        return ptr;
    }

    void* new_block = hdb_malloc(new_size);
    if (new_block) {
        return memcpy(new_block, ptr, block->size);
    }

    return NULL;
}