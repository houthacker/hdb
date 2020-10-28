#include <stdlib.h> // malloc, abort
#include <errno.h> // errno

#include "os.h"

void* os_malloc(size_t size) {
    void* ptr = malloc(size);

    if (ptr == NULL && errno) {
        abort();
    }

    return ptr;
}

void os_free(void* ptr) {
    free(ptr);
}

void os_abort() {
    abort();
}