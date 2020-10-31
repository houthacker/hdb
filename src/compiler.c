#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

void hdb_compiler_init(void) {
    hdb_scanner_create();
}

void hdb_compiler_free(void) {
    hdb_scanner_free();
}

void hdb_compiler_compile(const char* source) {
    hdb_scanner_init(source);

    // temporary until scanner can provide tokens
    int32_t line = -1;
    for (;;) {
        hdb_token_t token = hdb_scanner_scan_token();
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }

        printf("%2d '%.*s'\n", token.type, token.length, token.start);
        if (token.type == TOKEN_EOF) break;
    }
}
