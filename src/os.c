#include <stdlib.h> // malloc, abort
#include <errno.h> // errno
#include <signal.h> // raise

#include "os.h"

void* os_malloc(size_t size) {
    void* ptr = malloc(size);

    if (ptr == NULL && errno) {
        abort();
    }

    return ptr;
}

void* os_realloc(void* ptr, size_t size) {
    void* result = realloc(ptr, size);

    if (result == NULL && errno) {
        abort();
    }

    return result;
}

void os_free(void* ptr) {
    free(ptr);
}

int32_t os_raise(int32_t signal) {
    return raise(signal);
}

void os_abort() {
    abort();
}