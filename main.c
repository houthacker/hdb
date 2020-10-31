#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "os.h"

static void repl() {
    char line[1024];
    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        hdb_vm_interpret(line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)os_malloc(file_size + 1);
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char* source = readFile(path);
    hdb_interpret_result_t result = hdb_vm_interpret(source);
    os_free(source);

    if (result == INTERPRET_COMPILE_ERROR) { exit(65); }
    if (result == INTERPRET_RUNTIME_ERROR) { exit(70); }
}

int main(int argc, const char* argv[]) {
    hdb_vm_init(256, 512);

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: hdb [path]\n");
        exit(64);
    }
    return 0;
}
