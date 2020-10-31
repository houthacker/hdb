#include <string.h>

#include "common.h"
#include "os.h"
#include "scanner.h"

/**
 * Scanner structure.
 */
typedef struct {

    /**
     * Pointer to the start of the full source string.
     */
    const char* first;

    /**
     * The pointer to the start of the current token within the source string.
     */
    const char* start;

    /**
     * The pointer to the current character within the current token in the source string.
     */
    const char* current;

    /**
     * The current line number within the source string.
     */
    int32_t line;
} hdb_scanner_t;

hdb_scanner_t* scanner;

void hdb_scanner_create(void) {
    if (scanner) {
        return;
    }

    scanner = os_malloc(sizeof(hdb_scanner_t));
    scanner->first = NULL;
    scanner->start = NULL;
    scanner->current = NULL;
    scanner->line = -1;
}

void hdb_scanner_init(const char* source) {
    if (scanner) {
        scanner->first = source;
        scanner->start = source;
        scanner->current = source;
        scanner->line = 1;
    }
}

void hdb_scanner_free(void) {
    if (scanner) {
        os_free(scanner);
        scanner = NULL;
    }
}

static bool at_end(void) {
    return *scanner->current == '\0';
}

static char advance(void) {
    scanner->current++;
    return scanner->current[-1];
}

static char peek(void) {
    return *scanner->current;
}

static char peek_next(void) {
    if (at_end()) { return '\0'; }
    return scanner->current[1];
}

static char peek_prev(void) {
    return scanner->current[-1];
}

static bool match(char expected) {
    if(at_end()) { return false; }
    if (*scanner->current != expected) { return false; }

    scanner->current++;
    return true;
}

static hdb_token_t make_token(hdb_token_type_t type) {
    hdb_token_t token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int32_t)(scanner->current - scanner->start);
    token.line = scanner->line;

    return token;
}

static hdb_token_t error_token(const char* message) {
    hdb_token_t token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int32_t)strlen(message);
    token.line = scanner->line;

    return token;
}

static void skip_whitespace(void) {
    for (;;) {
        char c = peek();
        switch(c) {
            case ' ':
            case '\r':
            case '\t':
                advance(); break;

            case '\n':
                scanner->line++;
                advance();
                break;

            case '/':
                if (peek_next() == '/') {
                    // single line comment
                    while (peek() != '\n' && !at_end()) { advance(); }
                } else {
                    return;
                }
            default:
                return;
        }
    }
}

static hdb_token_type_t check_keyword(int32_t start, int32_t length, const char* rest, hdb_token_type_t type) {
    if (scanner->current - scanner->start == start + length
        && memcmp(scanner->start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static hdb_token_type_t identifier_type(void) {
    int32_t  len = scanner->current - scanner->start;
    switch(scanner->start[0]) {
        case 'a':
            // absolute, action, add, after, all, allocate, alter, and, any, are,
            //    array, as, asc, assertion, at, authorization,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'b': return check_keyword(2, 6, "solute", TOKEN_ABSOLUTE);
                    case 'c': return check_keyword(2, 4, "tion", TOKEN_ACTION);
                    case 'd': return check_keyword(2, 1, "d", TOKEN_ADD);
                    case 'f': return check_keyword(2, 3, "ter", TOKEN_AFTER);
                    case 'l':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'l':
                                    if (len == 3) {
                                        return TOKEN_ALL;
                                    } else if (len > 3) {
                                        switch(scanner->start[3]) {
                                            case 'o': return check_keyword(4, 4, "cate", TOKEN_ALLOCATE);
                                        }
                                    }
                                case 't': return check_keyword(3, 2, "er", TOKEN_ALTER);
                            }

                        }
                    case 'n':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'd': if (len == 3) { return TOKEN_AND; }
                                case 'y': if (len == 3) { return TOKEN_ANY; }
                            }
                        }
                    case 'r':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'e': if (len == 3) { return TOKEN_ARE; }
                                case 'r': return check_keyword(3, 2, "ay", TOKEN_ARRAY);
                            }
                        }
                    case 's':
                        if (len == 2) {
                            return TOKEN_AS;
                        } else {
                            switch(scanner->start[2]) {
                                case 'c': if (len == 3) { return TOKEN_ASC; }
                                case 's': return check_keyword(3, 6, "ertion", TOKEN_ASSERTION);
                            }
                        }
                    case 't':
                        if (len == 2) { return TOKEN_AT; }
                    case 'u': return check_keyword(2, 11, "thorization", TOKEN_AUTHORIZATION);
                }
            }

        case 'b':
            // before, begin, between, binary, bit, blob, boolean, both,
            //    breadth, by,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'e':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'f': return check_keyword(3, 3, "ore", TOKEN_BEFORE);
                                case 'g': return check_keyword(3, 2, "in", TOKEN_BEGIN);
                                case 't': return check_keyword(3, 4, "ween", TOKEN_BETWEEN);
                            }
                        }
                    case 'i':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'n': return check_keyword(3, 3, "ary", TOKEN_BINARY);
                                case 't': if (len == 3) { return TOKEN_BIT; }
                            }
                        }
                    case 'l': return check_keyword(2, 2, "ob", TOKEN_BLOB);
                    case 'o':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'o': return check_keyword(3, 4, "lean", TOKEN_BOOLEAN);
                                case 't': return check_keyword(3, 1, "h", TOKEN_BOTH);
                            }
                        }
                    case 'r': return check_keyword(2, 5, "eadth", TOKEN_BREADTH);
                    case 'y': if (len == 2) { return TOKEN_BY; }

                }
            }

        case 'c':
            // call, cascade, cascaded, case, cast, catalog, char, character,
            //    check, clob, close, collate, collation, column, commit,
            //    condition, connect, connection, constraint, constraints,
            //    constructor, continue, corresponding, create, cross, cube,
            //    current, current_date, current_default_transform_group,
            //    current_path, current_role, current_time, current_timestamp,
            //    current_transform_group_for_type, current_user, cursor, cycle,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'a':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'l': return check_keyword(3, 1, "l", TOKEN_CALL);
                                case 's':
                                    if (len > 3) {
                                        switch(scanner->start[3]) {
                                            case 'c':
                                                if (len == 7) {
                                                    return check_keyword(4, 3, "ade", TOKEN_CASCADE);
                                                } else if (len == 8) {
                                                    return check_keyword(4, 4, "aded", TOKEN_CASCADED);
                                                }
                                            case 'e': if (len == 4) { return TOKEN_CASE; }
                                            case 't': if (len == 4) { return TOKEN_CAST; }
                                        }
                                    }
                                case 't': return check_keyword(3, 4, "alog", TOKEN_CATALOG);
                            }
                        }
                    case 'h':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'a':
                                    if (len == 4) {
                                        return check_keyword(3, 1, "r", TOKEN_CHAR);
                                    } else if (len == 9) {
                                        return check_keyword(3, 6, "racter", TOKEN_CHARACTER);
                                    }
                                case 'e': return check_keyword(3, 2, "ck", TOKEN_CHECK);
                            }
                        }
                    case 'l':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'o':
                                    if (len == 4) {
                                        return check_keyword(3, 1, "b", TOKEN_CLOB);
                                    } else if (len == 5) {
                                        return check_keyword(3, 2, "se", TOKEN_CLOSE);
                                    }
                            }
                        }
                    case 'o':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'l':
                                    if (len > 3) {
                                        switch(scanner->start[3]) {
                                            case 'l':
                                                if (len == 7) {
                                                    return check_keyword(4, 3, "ate", TOKEN_COLLATE);
                                                }
                                                return check_keyword(4, 5, "ation", TOKEN_COLLATION);
                                            case 'u': return check_keyword(4, 2, "mn", TOKEN_COLUMN);
                                        }
                                    }
                                case 'm': return check_keyword(3, 3, "mit", TOKEN_COMMIT);
                                case 'n':
                                    if (len > 3) {
                                        switch (scanner->start[3]) {
                                            case 'd': return check_keyword(4, 5, "ition", TOKEN_CONDITION);
                                            case 'n':
                                                if (len == 7) {
                                                    return check_keyword(4, 3, "ect", TOKEN_CONNECT);
                                                }
                                                return check_keyword(4, 6, "ection", TOKEN_CONNECTION);
                                            case 's':
                                                if (len == 10) {
                                                    return check_keyword(4, 6, "traint", TOKEN_CONSTRAINT);
                                                } else if (len == 11) {
                                                    if (scanner->start[6] == 'u') {
                                                        return check_keyword(4, 7, "tructor", TOKEN_CONSTRUCTOR);
                                                    }
                                                    return check_keyword(4, 7, "traints", TOKEN_CONSTRAINTS);
                                                }
                                            case 't': return check_keyword(4, 4, "inue", TOKEN_CONTINUE);
                                        }
                                    }
                                case 'r': return check_keyword(3, 10, "responding", TOKEN_CORRESPONDING);
                            }
                        }
                    case 'r':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'e': return check_keyword(3, 3, "ate", TOKEN_CREATE);
                                case 'o': return check_keyword(3, 2, "ss", TOKEN_CROSS);
                            }
                        }
                    case 'u':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'b': return check_keyword(3, 1, "e", TOKEN_CUBE);
                                case 'r':
                                    if (len == 6) {
                                        return check_keyword(3, 3, "sor", TOKEN_CURSOR);
                                    } else if (len == 7) {
                                        return check_keyword(3, 4, "rent", TOKEN_CURRENT);
                                    } else if (len > 7 && scanner->start[7] == '_') {
                                        switch(scanner->start[8]) {
                                            case 'd':
                                                if (len == 12) {
                                                    return check_keyword(9, 3, "ate",TOKEN_CURRENT_DATE);
                                                }
                                                return check_keyword(9, 22, "efault_transform_group",
                                                                     TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP);
                                            case 'p': return check_keyword(9, 3, "ath", TOKEN_CURRENT_PATH);
                                            case 'r': return check_keyword(9, 3, "ole", TOKEN_CURRENT_ROLE);
                                            case 't':
                                                if (len == 12) {
                                                    return check_keyword(9, 3, "ime", TOKEN_CURRENT_TIME);
                                                } else if (len == 17) {
                                                    return check_keyword(9, 8, "imestamp", TOKEN_CURRENT_TIMESTAMP);
                                                }
                                                return check_keyword(9, 23, "ransform_group_for_type",
                                                                     TOKEN_CURRENT_TRANSFORM_GROUP_FOR_TYPE);
                                            case 'u': return check_keyword(9, 3, "ser", TOKEN_CURRENT_USER);
                                        }
                                    }
                            }
                        }
                    case 'y': return check_keyword(2, 3, "cle", TOKEN_CYCLE);
                }
            }

        case 'd':
            // data, date, day, deallocate, dec, decimal, declare, default,
            //    deferrable, deferred, delete, depth, deref, desc,
            //    describe, descriptor, deterministic,
            //    diagnostics, disconnect, distinct, do, domain, double,
            //    drop, dynamic,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'a':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 't':
                                    if (len == 4) {
                                        switch (scanner->start[3]) {
                                            case 'a': return TOKEN_DATA;
                                            case 'e': return TOKEN_DATE;
                                        }
                                    }
                                case 'y':
                                    if (len == 3) { return TOKEN_DAY; }
                            }
                        }
                    case 'e':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'a': return check_keyword(3, 7, "llocate", TOKEN_DEALLOCATE);
                                case 'c':
                                    if (len == 3) {
                                        return TOKEN_DEC;
                                    } else {
                                        switch(scanner->start[3]) {
                                            case 'i': return check_keyword(4, 3, "mal", TOKEN_DECIMAL);
                                            case 'l': return check_keyword(4, 3, "are", TOKEN_DECLARE);
                                        }
                                    }
                                case 'f':
                                    if (len == 7) {
                                        return check_keyword(3, 4, "ault", TOKEN_DEFAULT);
                                    } else if (len == 8) {
                                        return check_keyword(3, 5, "erred", TOKEN_DEFERRED);
                                    }
                                    return check_keyword(3, 7, "errable", TOKEN_DEFERRABLE);
                                case 'l': return check_keyword(3, 3, "ete", TOKEN_DELETE);
                                case 'p': return check_keyword(3, 2, "th", TOKEN_DEPTH);
                                case 'r': return check_keyword(3, 2, "ef", TOKEN_DEREF);
                                case 's':
                                    if (len == 4) {
                                        return check_keyword(3, 1, "c", TOKEN_DESC);
                                    } else if (len == 8) {
                                        return check_keyword(3, 5, "cribe", TOKEN_DESCRIBE);
                                    }
                                    return check_keyword(3, 7, "criptor", TOKEN_DESCRIPTOR);
                                case 't': return check_keyword(3, 10, "erministic", TOKEN_DETERMINISTIC);
                            }
                        }
                    case 'i':
                        if (len == 8) {
                            return check_keyword(2, 6, "stinct", TOKEN_DISTINCT);
                        } else if (len == 10) {
                            return check_keyword(2, 8, "sconnect", TOKEN_DISCONNECT);
                        }

                        return check_keyword(2, 9, "agnostics", TOKEN_DIAGNOSTICS);
                    case 'o':
                        if (len == 2) {
                            return TOKEN_DO;
                        } else {
                            switch (scanner->start[2]) {
                                case 'm': return check_keyword(3, 3, "ain", TOKEN_DOMAIN);
                                case 'u': return check_keyword(3, 3, "ble", TOKEN_DOUBLE);
                            }
                        }
                    case 'r': return check_keyword(2, 2, "op", TOKEN_DROP);
                    case 'y': return check_keyword(2, 5, "namic", TOKEN_DYNAMIC);
                }
            }

        case 'e':
            // each, else, elseif, end, end_exec, equals_keyword, escape, except,
            //    exception, exec, execute, exists, exit, external,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'a': return check_keyword(2, 2, "ch", TOKEN_EACH);
                    case 'l':
                        if (len == 4) {
                            return check_keyword(2, 2, "se", TOKEN_ELSE);
                        }

                        return check_keyword(2, 4, "seif", TOKEN_ELSEIF);
                    case 'n':
                        if (len == 3) {
                            return check_keyword(2, 1, "d", TOKEN_END);
                        }

                        return check_keyword(2, 6, "d_exec", TOKEN_END_EXEC);
                    case 'q': return check_keyword(2, 4, "uals", TOKEN_EQUALS_KEYWORD);
                    case 's': return check_keyword(2, 4, "cape", TOKEN_ESCAPE);
                    case 'x':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'c':
                                    if (len == 6) {
                                        return check_keyword(3, 3, "ept", TOKEN_EXCEPT);
                                    }

                                    return check_keyword(3, 6, "eption", TOKEN_EXCEPTION);
                                case 'e':
                                    if (len == 4) {
                                        return check_keyword(3, 1, "c", TOKEN_EXEC);
                                    }
                                    return check_keyword(3, 4, "cute", TOKEN_EXECUTE);
                                case 'i':
                                    if (len == 4) {
                                        return check_keyword(3, 1, "t", TOKEN_EXIT);
                                    }

                                    return check_keyword(3, 3, "sts", TOKEN_EXISTS);
                                case 't': return check_keyword(3, 5, "ernal", TOKEN_EXTERNAL);
                            }
                        }

                }
            }

        case 'f':
            // false, fetch, first, float, for, foreign, found, from, free,
            //    full, function,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'a': return check_keyword(2, 3, "lse", TOKEN_FALSE);
                    case 'e': return check_keyword(2, 3, "tch", TOKEN_FETCH);
                    case 'i': return check_keyword(2, 3, "rst", TOKEN_FIRST);
                    case 'l': return check_keyword(2, 3, "oat", TOKEN_FLOAT);
                    case 'o':
                        if (len == 3) {
                            return check_keyword(2, 1, "r", TOKEN_FOR);
                        } else if (len == 5) {
                            return check_keyword(2, 3, "und", TOKEN_FOUND);
                        }

                        return check_keyword(2, 5, "reign", TOKEN_FOREIGN);
                    case 'r':
                        if (len == 4) {
                            switch(scanner->start[2]) {
                                case 'e': return check_keyword(3, 1, "e", TOKEN_FREE);
                                case 'o': return check_keyword(3, 1, "m", TOKEN_FROM);
                            }
                        }
                    case 'u':
                        if (len == 4) {
                            return check_keyword(2, 2, "ll", TOKEN_FULL);
                        }
                        return check_keyword(2, 6, "nction", TOKEN_FUNCTION);
                }
            }

        case 'g':
            // general, get, global, go, goto, grant, group, grouping,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'e':
                        if (len == 3) {
                            return check_keyword(2, 1, "t", TOKEN_GET);
                        }
                        return check_keyword(2, 5, "neral", TOKEN_GENERAL);
                    case 'l': return check_keyword(2, 4, "obal", TOKEN_GLOBAL);
                    case 'o':
                        if (len == 2) {
                            return TOKEN_GO;
                        }
                        return check_keyword(2, 2, "to", TOKEN_GOTO);
                    case 'r':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'a': return check_keyword(3, 2, "nt", TOKEN_GRANT);
                                case 'o':
                                    if (len == 5) {
                                        return check_keyword(3, 2, "up", TOKEN_GROUP);
                                    }

                                    return check_keyword(3, 5, "uping", TOKEN_GROUPING);
                            }
                        }
                }
            }

        case 'h':
            // handle, having, hold, hour,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'a':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'n': return check_keyword(3, 3, "dle", TOKEN_HANDLE);
                                case 'v': return check_keyword(3, 3, "ing", TOKEN_HAVING);
                            }
                        }
                    case 'o':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'l':  return check_keyword(3, 1, "d", TOKEN_HOLD);
                                case 'u': return check_keyword(3, 1, "r", TOKEN_HOUR);
                            }
                        }
                }
            }

        case 'i':
            // identity, if, immediate, in, indicator,
            //    initially, inner, inout, input, insert, int, integer,
            //    intersect, interval, into, is, isolation,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'd': return check_keyword(2, 6, "entity", TOKEN_IDENTITY);
                    case 'f': if (len == 2) { return TOKEN_IF; }
                    case 'm': return check_keyword(2, 7, "mediate", TOKEN_IMMEDIATE);
                    case 'n':
                        if (len == 2) {
                            return TOKEN_IN;
                        } else {
                            switch(scanner->start[2]) {
                                case 'd': return check_keyword(3, 6, "icator", TOKEN_INDICATOR);
                                case 'i': return check_keyword(3, 6, "tially", TOKEN_INITIALLY);
                                case 'n': return check_keyword(3, 2, "er", TOKEN_INNER);
                                case 'o': return check_keyword(3, 2, "ut", TOKEN_INOUT);
                                case 'p': return check_keyword(3, 2, "ut", TOKEN_INPUT);
                                case 's': return check_keyword(3, 3, "ert", TOKEN_INSERT);
                                case 't':
                                    if (len == 3) {
                                        return TOKEN_INT;
                                    } else if (len > 3) {
                                        switch(scanner->start[3]) {
                                            case 'e':
                                                if (len > 4) {
                                                    switch (scanner->start[4]) {
                                                        case 'g': return check_keyword(5, 2, "er", TOKEN_INTEGER);
                                                        case 'r':
                                                            if (len == 8) {
                                                                return check_keyword(5, 3, "val", TOKEN_INTERVAL);
                                                            }

                                                            return check_keyword(5, 4, "sect", TOKEN_INTERSECT);
                                                    }
                                                }
                                            case 'o': if (len == 4) { return TOKEN_INTO; }
                                        }
                                    }

                            }
                        }
                    case 's':
                        if (len == 2) {
                            return TOKEN_IS;
                        }

                        return check_keyword(2, 7, "olation", TOKEN_ISOLATION);
                }
            }

        case 'j': return check_keyword(1, 3, "oin", TOKEN_JOIN);

        case 'k': return check_keyword(1, 2, "ey", TOKEN_KEY);

        case 'l':
            // language, large, last, lateral, leading, leave, left,
            //    level, like, local, localtime, localtimestamp, locator, loop,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'a':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'n': return check_keyword(3, 5, "guage", TOKEN_LANGUAGE);
                                case 'r': return check_keyword(3, 2, "ge", TOKEN_LARGE);
                                case 's': return check_keyword(3, 1, "t", TOKEN_LAST);
                                case 't': return check_keyword(3, 4, "eral", TOKEN_LATERAL);
                            }
                        }
                    case 'e':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'a':
                                    if (len == 5) {
                                        return check_keyword(3, 2, "ve", TOKEN_LEAVE);
                                    }

                                    return check_keyword(3, 4, "ding", TOKEN_LEADING);
                                case 'f': return check_keyword(3, 1, "t", TOKEN_LEFT);
                                case 'v': return check_keyword(3, 2, "el", TOKEN_LEVEL);
                            }
                        }
                    case 'i': return check_keyword(2, 2, "ke", TOKEN_LIKE);
                    case 'o':
                        if (len > 2) {
                            switch(scanner->start[2]) {
                                case 'c':
                                    if (len == 5) {
                                        return check_keyword(3, 2, "al", TOKEN_LOCAL);
                                    } else if (len == 7) {
                                        return check_keyword(3, 4, "ator", TOKEN_LOCATOR);
                                    } else if (len == 9) {
                                        return check_keyword(3, 6, "altime", TOKEN_LOCALTIME);
                                    }

                                    return check_keyword(3, 11, "altimestamp", TOKEN_LOCALTIMESTAMP);
                                case 'o': return check_keyword(3, 1, "p", TOKEN_LOOP);
                            }
                        }
                }
            }

        case 'm':
            // map, match, method, minute, modifies, module, month,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'a':
                        if (len == 3) {
                            return check_keyword(2, 1, "p", TOKEN_MAP);
                        }

                        return check_keyword(2, 3, "tch", TOKEN_MATCH);
                    case 'e': return check_keyword(2, 4, "thod", TOKEN_METHOD);
                    case 'i': return check_keyword(2, 4, "nute", TOKEN_MINUTE);
                    case 'o':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'd':
                                    if (len > 3) {
                                        switch(scanner->start[3]) {
                                            case 'i': return check_keyword(4, 4, "fies", TOKEN_MODIFIES);
                                            case 'u': return check_keyword(4, 2, "le", TOKEN_MODULE);
                                        }
                                    }
                                case 'n': return check_keyword(3, 2, "th", TOKEN_MONTH);
                            }
                        }
                }
            }

        case 'n':
            // names, national, natural, nchar, nclob, nesting, new, next,
            //    no, none, not, null, numeric,
            if (len > 1) {
                switch (scanner->start[1]) {
                    case 'a':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'm': return check_keyword(3, 2, "es", TOKEN_NAMES);
                                case 't':
                                    if (len == 7) {
                                        return check_keyword(3, 4, "ural", TOKEN_NATURAL);
                                    }
                                    return check_keyword(3, 5, "ional", TOKEN_NATIONAL);
                            }
                        }
                    case 'c':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'h': return check_keyword(3, 2, "ar", TOKEN_NCHAR);
                                case 'l': return check_keyword(3, 2, "ob", TOKEN_NCLOB);
                            }
                        }
                    case 'e':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 's': return check_keyword(3, 4, "ting", TOKEN_NESTING);
                                case 'w': if (len == 3) { return TOKEN_NEW; }
                                case 'x': return check_keyword(3, 1, "t", TOKEN_NEXT);
                            }
                        }
                    case 'o':
                        if (len == 2) {
                            return TOKEN_NO;
                        } else if (len == 3) {
                            return check_keyword(2, 1, "t", TOKEN_NOT);
                        } else if (len == 4) {
                            return check_keyword(2, 2, "ne", TOKEN_NONE);
                        }
                    case 'u':
                        if (len == 4) {
                            return check_keyword(2, 2, "ll", TOKEN_NULL);
                        } else if (len == 7) {
                            return check_keyword(2, 5, "meric", TOKEN_NUMERIC);
                        }
                }
            }

        case 'o':
            // object, of, old, on, only, open, option,
            //    or, order, ordinality, out, outer, output, overlaps,
            if (len > 1) {
                switch(scanner->start[1]) {
                    case 'b': return check_keyword(2, 4, "ject", TOKEN_OBJECT);
                    case 'f': if (len == 2) { return TOKEN_OF; }
                    case 'l': return check_keyword(2, 1, "d", TOKEN_OLD);
                    case 'n':
                        if (len == 2) {
                            return TOKEN_ON;
                        }
                        return check_keyword(2, 2, "ly", TOKEN_ONLY);
                    case 'p':
                        if (len == 4) {
                            return check_keyword(2, 2, "en", TOKEN_OPEN);
                        }
                        return check_keyword(2, 4, "tion", TOKEN_OPTION);
                    case 'r':
                        if (len == 2) {
                            return TOKEN_OR;
                        } else if (len == 5) {
                            return check_keyword(2, 3, "der", TOKEN_ORDER);
                        }
                        return check_keyword(2, 8, "dinality", TOKEN_ORDINALITY);
                    case 'u':
                        if (len == 3) {
                            return check_keyword(2, 1, "t", TOKEN_OUT);
                        } else if (len == 5) {
                            return check_keyword(2, 3, "ter", TOKEN_OUTER);
                        }
                        return check_keyword(2, 4, "tput", TOKEN_OUTPUT);
                    case 'v': return check_keyword(2, 6, "erlaps", TOKEN_OVERLAPS);
                }
            }

        case 'p':
            // pad, parameter, partial, path, precision,
            //    prepare, preserve, primary, prior, privileges, procedure, public,
            if (len > 1) {
                switch (scanner->start[1]) {
                    case 'a':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'd': if (len == 3) { return TOKEN_PAD; }
                                case 'r':
                                    if (len > 3) {
                                        switch (scanner->start[3]) {
                                            case 'a': return check_keyword(4, 5, "meter", TOKEN_PARAMETER);
                                            case 't': return check_keyword(4, 3, "ial", TOKEN_PARTIAL);
                                        }
                                    }
                                case 't': return check_keyword(3, 1, "h", TOKEN_PATH);
                            }
                        }
                    case 'r':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'e':
                                    if (len > 3) {
                                        switch (scanner->start[3]) {
                                            case 'c': return check_keyword(4, 5, "ision", TOKEN_PRECISION);
                                            case 'p': return check_keyword(4, 3, "are", TOKEN_PREPARE);
                                            case 's': return check_keyword(4, 4, "erve", TOKEN_PRESERVE);
                                        }
                                    }
                                case 'i':
                                    if (len > 3) {
                                        switch(scanner->start[3]) {
                                            case 'm': return check_keyword(4, 3, "ary", TOKEN_PRIMARY);
                                            case 'o': return check_keyword(4, 1, "r", TOKEN_PRIOR);
                                            case 'v': return check_keyword(4, 6, "ileges", TOKEN_PRIVILEGES);
                                        }
                                    }
                                case 'o': return check_keyword(3, 6, "cedure", TOKEN_PROCEDURE);
                            }
                        }
                    case 'u': return check_keyword(2, 4, "blic", TOKEN_PUBLIC);
                }
            }

        case 'r':
            // read, reads, real, recursive, redo, ref, references, referencing,
            //    relative, release, repeat, resignal, restrict, result, return,
            //    returns, revoke, right, role, rollback, rollup, routine,
            //    row, rows,
            if (len > 1) {
                switch (scanner->start[1]) {
                    case 'e':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'a':
                                    if (len > 3) {
                                        switch (scanner->start[3]) {
                                            case 'd':
                                                if (len == 4) {
                                                    return TOKEN_READ;
                                                } else if (len == 5) {
                                                    return check_keyword(4, 1, "s", TOKEN_READS);
                                                }
                                            case 'l': if (len == 4) { return TOKEN_REAL; }
                                        }
                                    }
                                case 'c': return check_keyword(3, 6, "ursive", TOKEN_RECURSIVE);
                                case 'd': return check_keyword(3, 1, "o", TOKEN_REDO);
                                case 'f':
                                    if (len == 3) {
                                        return TOKEN_REF;
                                    } else if (len == 10) {
                                        return check_keyword(3, 7, "erences", TOKEN_REFERENCES);
                                    }
                                    return check_keyword(3, 8, "erencing", TOKEN_REFERENCING);
                                case 'l':
                                    if (len == 7) {
                                        return check_keyword(3, 4, "ease", TOKEN_RELEASE);
                                    }
                                    return check_keyword(3, 5, "ative", TOKEN_RELATIVE);
                                case 'p': return check_keyword(3, 3, "eat", TOKEN_REPEAT);
                                case 's':
                                    if (len > 3) {
                                        switch (scanner->start[3]) {
                                            case 'i': return check_keyword(4, 4, "gnal", TOKEN_RESIGNAL);
                                            case 't': return check_keyword(4, 4, "rict", TOKEN_RESTRICT);
                                            case 'u': return check_keyword(4, 2, "lt", TOKEN_RESULT);
                                        }
                                    }
                                case 't':
                                    if (len == 6) {
                                        return check_keyword(3, 3, "urn", TOKEN_RETURN);
                                    }
                                    return check_keyword(3, 4, "urns", TOKEN_RETURNS);
                                case 'v': return check_keyword(3, 3, "oke", TOKEN_REVOKE);
                            }
                        }
                    case 'i': return check_keyword(2, 3, "ght", TOKEN_RIGHT);
                    case 'o':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'l':
                                    if (len == 4) {
                                        return check_keyword(3, 1, "e", TOKEN_ROLE);
                                    } else if (len == 6) {
                                        return check_keyword(3, 3, "lup", TOKEN_ROLLUP);
                                    }
                                    return check_keyword(3, 5, "lback", TOKEN_ROLLBACK);
                                case 'u': return check_keyword(3, 4, "tine", TOKEN_ROUTINE);
                                case 'w':
                                    if (len == 3) {
                                        return TOKEN_ROW;
                                    }
                                    return check_keyword(3, 1, "s", TOKEN_ROWS);
                            }
                        }
                }
            }

        case 's':
            // savepoint, schema, scroll, search, second, section, select,
            //    session, session_user, set, sets, signal, similar, size,
            //    smallint, some, space, specific, specifictype, sql, sqlexception,
            //    sqlstate, sqlwarning, start, state, static, system_user,
            if (len > 1) {
                switch (scanner->start[1]) {
                    case 'a': return check_keyword(2, 7, "vepoint", TOKEN_SAVEPOINT);
                    case 'c':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'h': return check_keyword(3, 3, "ema", TOKEN_SCHEMA);
                                case 'r': return check_keyword(3, 3, "oll", TOKEN_SCROLL);
                            }
                        }
                    case 'e':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'a': return check_keyword(3, 3, "rch", TOKEN_SEARCH);
                                case 'c':
                                    if (len == 6) {
                                        return check_keyword(3, 3, "ond", TOKEN_SECOND);
                                    }
                                    return check_keyword(3, 4, "tion", TOKEN_SECTION);
                                case 'l': return check_keyword(3, 3, "ect", TOKEN_SELECT);
                                case 's':
                                    if (len == 7) {
                                        return check_keyword(3, 4, "sion", TOKEN_SESSION);
                                    }
                                    return check_keyword(3, 9, "sion_user", TOKEN_SESSION_USER);
                                case 't':
                                    if (len == 3) {
                                        return TOKEN_SET;
                                    }
                                    return check_keyword(3, 1, "s", TOKEN_SETS);
                            }
                        }
                    case 'i':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'g': return check_keyword(3, 3, "nal", TOKEN_SIGNAL);
                                case 'm': return check_keyword(3, 4, "ilar", TOKEN_SIMILAR);
                                case 'z': return check_keyword(3, 1, "e", TOKEN_SIZE);
                            }
                        }
                    case 'm': return check_keyword(2, 6, "allint", TOKEN_SMALLINT);
                    case 'o': return check_keyword(2, 2, "me", TOKEN_SOME);
                    case 'p':
                        if (len == 5) {
                            return check_keyword(2, 3, "ace", TOKEN_SPACE);
                        } else if (len == 8) {
                            return check_keyword(2, 6, "ecific", TOKEN_SPECIFIC);
                        }
                        return check_keyword(2, 10, "ecifictype", TOKEN_SPECIFICTYPE);
                    case 'q':
                        if (len == 3) {
                            return check_keyword(2, 1, "l", TOKEN_SQL);
                        } else if (len == 8) {
                            return check_keyword(2, 6, "lstate", TOKEN_SQLSTATE);
                        } else if (len == 10) {
                            return check_keyword(2, 8, "lwarning", TOKEN_SQLWARNING);
                        }
                        return check_keyword(2, 10, "lexception", TOKEN_SQLEXCEPTION);
                    case 't':
                        if (len > 3 && scanner->start[2] == 'a') {
                            switch (scanner->start[3]) {
                                case 'r': return check_keyword(4, 1, "t", TOKEN_START);
                                case 't':
                                    if (len == 5) {
                                        return check_keyword(4, 1, "e", TOKEN_STATE);
                                    }
                                    return check_keyword(4, 2, "ic", TOKEN_STATIC);
                            }
                        }
                    case 'y': return check_keyword(2, 9, "stem_user", TOKEN_SYSTEM_USER);
                }
            }

        case 't':
            // table, temporary, then, time, timestamp,
            //    timezone_hour, timezone_minute, to, trailing, transaction,
            //    translation, treat, trigger, true,
            if (len > 1) {
                switch (scanner->start[1]) {
                    case 'a': return check_keyword(2, 3, "ble", TOKEN_TABLE);
                    case 'e': return check_keyword(2, 7, "mporary", TOKEN_TEMPORARY);
                    case 'h': return check_keyword(2, 2, "en", TOKEN_THEN);
                    case 'i':
                        if (len == 4) {
                            return check_keyword(2, 2, "me", TOKEN_TIME);
                        } else if (len == 9) {
                            return check_keyword(2, 7, "mestamp", TOKEN_TIMESTAMP);
                        } else if (len == 13) {
                            return check_keyword(2, 11, "mezone_hour", TOKEN_TIMEZONE_HOUR);
                        }

                        return check_keyword(2, 13, "mezone_minute", TOKEN_TIMEZONE_MINUTE);
                    case 'o': if (len == 2) { return TOKEN_TO; }
                    case 'r':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'a':
                                    if (len == 8) {
                                        return check_keyword(3, 5, "iling", TOKEN_TRAILING);
                                    } else if (len == 11) {
                                        if (scanner->start[5] == 'a') {
                                            return check_keyword(3, 8, "nsaction", TOKEN_TRANSACTION);
                                        }
                                        return check_keyword(3, 8, "nslation", TOKEN_TRANSLATION);
                                    }
                                case 'e': return check_keyword(3, 2, "at", TOKEN_TREAT);
                                case 'i': return check_keyword(3, 4, "gger", TOKEN_TRIGGER);
                                case 'u': return check_keyword(3, 1, "e", TOKEN_TRUE);
                            }
                        }
                }
            }

        case 'u':
            // under, undo, union, unique, unknown, unnest, until, update,
            //    usage, user, using,
            if (len > 1) {
                switch (scanner->start[1]) {
                    case 'n':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'd':
                                    if (len > 3) {
                                        switch (scanner->start[3]) {
                                            case 'e': return check_keyword(4, 1, "r", TOKEN_UNDER);
                                            case 'o': if (len == 4) { return TOKEN_UNDO; }
                                        }
                                    }
                                case 'i':
                                    if (len == 5) {
                                        return check_keyword(3, 2, "on", TOKEN_UNION);
                                    }
                                    return check_keyword(3, 3, "que", TOKEN_UNIQUE);
                                case 'k': return check_keyword(3, 4, "nown", TOKEN_UNKNOWN);
                                case 'n': return check_keyword(3, 3, "est", TOKEN_UNNEST);
                                case 't': if (len == 5) {
                                    return check_keyword(3, 2, "il", TOKEN_UNTIL);
                                }
                            }
                        }
                    case 'p': return check_keyword(2, 4, "date", TOKEN_UPDATE);
                    case 's':
                        if (len == 4) {
                            return check_keyword(2, 2, "er", TOKEN_USER);
                        } else if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'a': return check_keyword(3, 2, "ge", TOKEN_USAGE);
                                case 'i': return check_keyword(3, 2, "ng", TOKEN_USING);
                            }
                        }
                }
            }

        case 'v':
            // value, values, varchar, varying, view,
            if (len > 1) {
                switch (scanner->start[1]) {
                    case 'a':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'l':
                                    if (len == 5) {
                                        return check_keyword(3, 2, "ue", TOKEN_VALUE);
                                    }

                                    return check_keyword(3, 3, "ues", TOKEN_VALUES);
                                case 'r':
                                    if (len > 3) {
                                        switch (scanner->start[3]) {
                                            case 'c': return check_keyword(4, 3, "har", TOKEN_VARCHAR);
                                            case 'y': return check_keyword(4, 3, "ing", TOKEN_VARYING);
                                        }
                                    }
                            }
                        }
                    case 'i': return check_keyword(2, 2, "ew", TOKEN_VIEW);
                }
            }

        case 'w':
            // when, whenever, where, while, with, without, work, write,
            if (len > 1) {
                switch (scanner->start[1]) {
                    case 'h':
                        if (len > 2) {
                            switch (scanner->start[2]) {
                                case 'e':
                                    if (len == 4) {
                                        return check_keyword(3, 1, "n", TOKEN_WHEN);
                                    } else if (len == 5) {
                                        return check_keyword(3, 2, "re", TOKEN_WHERE);
                                    }
                                    return check_keyword(3, 5, "never", TOKEN_WHENEVER);
                                case 'i': return check_keyword(3, 2, "le", TOKEN_WHILE);
                            }
                        }
                    case 'i':
                        if (len == 4) {
                            return check_keyword(2, 2, "th", TOKEN_WITH);
                        }
                        return check_keyword(2, 5, "thout", TOKEN_WITHOUT);
                    case 'o': return check_keyword(2, 2, "rk", TOKEN_WORK);
                    case 'r': return check_keyword(2, 3, "ite", TOKEN_WRITE);
                }
            }

        case 'y':
            // year
            return check_keyword(1, 3, "ear", TOKEN_YEAR);

        case 'z':
            // zone
            return check_keyword(1, 3, "one", TOKEN_ZONE);
    }

    return TOKEN_IDENTIFIER;
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_identifier_start(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static bool is_identifier_body(char c) {
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
        || c == '_'
        || (c >= '0' && c <= '9');
}

static hdb_token_t string(void) {
    while ((peek() != '\'' || peek_prev() == '\\') && !at_end()) {
        if (peek() == '\n') {
            scanner->line++;
        }
        advance();
    }

    if (at_end()) {
        return error_token("Unterminated string.");
    }

    // The closing quote
    advance();
    return make_token(TOKEN_STRING);
}

static hdb_token_t identifier(void) {
    while (!at_end() && is_identifier_body(peek())) {
        advance();
    }

    return make_token(identifier_type());
}

static hdb_token_t number(void) {
    while(is_digit(peek())) { advance(); }

    // Look for a fractional part
    if (peek() == '.' && is_digit(peek_next())) {
        // Consume the period.
        advance();
    }

    while (is_digit(peek())) { advance(); }

    return make_token(TOKEN_NUMBER);
}

static hdb_token_t enclosed_identifier(char enclosing) {
    if (is_identifier_start(peek())) {
        advance();
    } else {
        return error_token("Invalid identifier start character.");
    }

    while (!at_end() && peek() != enclosing && is_identifier_body(peek())) {
        advance();
    }

    if (peek() != enclosing) {
        return error_token("Unterminated identifier.");
    }

    // Closing character
    advance();

    // Enclosed identifiers are never keywords, so identifier_type() is not required here.
    return make_token(TOKEN_ENCLOSED_IDENTIFIER);
}

hdb_token_t hdb_scanner_scan_token(void) {
    skip_whitespace();

    scanner->start = scanner->current;

    if (at_end()) { return make_token(TOKEN_EOF); }

    char c = advance();
    if (is_identifier_start(c)) { return identifier(); }
    if (is_digit(c)) { return number(); }

    switch (c) {
        case '(': return make_token(TOKEN_LEFT_PAREN);
        case ')': return make_token(TOKEN_RIGHT_PAREN);
        case '{': return make_token(TOKEN_LEFT_BRACE);
        case '}': return make_token(TOKEN_RIGHT_BRACE);
        case '[': return make_token(TOKEN_LEFT_BRACKET);
        case ']': return make_token(TOKEN_RIGHT_BRACKET);
        case ';': return make_token(TOKEN_SEMICOLON);
        case ',': return make_token(TOKEN_COMMA);
        case '-': return make_token(TOKEN_MINUS);
        case '+': return make_token(TOKEN_PLUS);
        case '/': return make_token(TOKEN_FORWARD_SLASH);
        case '*': return make_token(TOKEN_ASTERISK);
        case '=': return make_token(TOKEN_EQUALS);
        case '%': return make_token(TOKEN_PERCENT);
        case '&': return make_token(TOKEN_AMPERSAND);
        case ':': return make_token(TOKEN_COLON);
        case '?': return make_token(TOKEN_QUESTION_MARK);
        case '^': return make_token(TOKEN_CIRCUMFLEX);
        case '|': return make_token(TOKEN_VERTICAL_BAR);
        case '\\': return make_token(TOKEN_BACKSLASH);
        case '.': return make_token(TOKEN_PERIOD);

        case '!':
            return make_token(match('=') ? TOKEN_NOT_EQUAL : TOKEN_BANG);
        case '<':
            return make_token(match('=') ? TOKEN_LESS_EQUAL : match('>') ? TOKEN_NOT_EQUAL : TOKEN_LESS_THAN);
        case '>':
            return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER_THAN);

        case '\'':
            if (match('\'')) { return make_token(TOKEN_DOUBLE_QUOTE); }
            return string();

        case '`':
        case '"':
            return enclosed_identifier(c);
        default:
            return error_token("Unexpected character.");
    }
}
