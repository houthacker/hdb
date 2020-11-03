#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    hdb_token_t current;
    hdb_token_t previous;
    bool had_error;
    bool panic_mode;
} hdb_parser_t;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,    // =
    PREC_OR,            // or
    PREC_AND,           // and
    PREC_EQUALITY,      // = != <>
    PREC_COMPARISON,    // < > <= =>
    PREC_TERM,          // + -
    PREC_FACTOR,        // * /
    PREC_UNARY,         // ! -
    PREC_CALL,          // . ()
    PREC_PRIMARY
} hdb_precedence_t;

typedef void (*hdb_parse_fn)();

typedef struct {
    hdb_parse_fn prefix;
    hdb_parse_fn infix;
    hdb_precedence_t precedence;
} hdb_parse_rule_t;

hdb_parser_t parser;

hdb_chunk_t* compiling_chunk;

uint8_t stack_high_water_mark;
uint8_t stack_size;

#define HDB_DECREASE_STACK_SIZE(amount) stack_size -= amount;

#define HDB_INCREASE_STACK_SIZE(amount) \
    stack_size += amount;                \
    stack_high_water_mark = stack_size > stack_high_water_mark ? stack_size : stack_high_water_mark;


static hdb_chunk_t* current_chunk(void) {
    return compiling_chunk;
}

void hdb_compiler_init(void) {
    hdb_scanner_create();
}

void hdb_compiler_free(void) {
    hdb_scanner_free();
}

static void error_at(hdb_token_t* token, const char* message) {
    if (parser.panic_mode) {
        return;
    }

    parser.panic_mode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Do nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}

static void error(const char* message) {
    error_at(&parser.previous, message);
}

static void error_at_current(const char* message) {
    error_at(&parser.current, message);
}

static void advance(void) {
    parser.previous = parser.current;

    for (;;) {
        parser.current = hdb_scanner_scan_token();
        if (parser.current.type != TOKEN_ERROR) { break; }

        error_at_current(parser.current.start);
    }
}

static void consume(hdb_token_type_t type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    error_at_current(message);
}

static void emit_byte(uint8_t byte) {
    hdb_chunk_write(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
}

static void emit_return(void) {
    emit_byte(OP_RETURN);
}

static void emit_constant(hdb_value_t value) {
    hdb_chunk_write_constant(current_chunk(), value, parser.current.line);

    // A constant will get pushed on the stack, using a single slot.
    HDB_INCREASE_STACK_SIZE(1);
}

static void end_compiler(void) {
    emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        hdb_dbg_disassemble_chunk(current_chunk(), "code");
    }
#endif
}

static void expression(void);
static void parse_precedence(hdb_precedence_t precedence);
static hdb_parse_rule_t* get_rule(hdb_token_type_t operator_type);

// infix
static void binary(void) {
    // Remember the operator
    hdb_token_type_t operator_type = parser.previous.type;

    // Compile the right-hand operand
    hdb_parse_rule_t* rule = get_rule(operator_type);
    parse_precedence((hdb_precedence_t)(rule->precedence + 1));

    // Emit the operator instruction.
    // Binary operators first pop() two values off the stack, and push() the result back on to it.
    switch(operator_type) {
        case TOKEN_PLUS:            emit_byte(OP_ADD); HDB_DECREASE_STACK_SIZE(2); HDB_INCREASE_STACK_SIZE(1); break;
        case TOKEN_MINUS:           emit_byte(OP_SUBTRACT); HDB_DECREASE_STACK_SIZE(2); HDB_INCREASE_STACK_SIZE(1); break;
        case TOKEN_ASTERISK:        emit_byte(OP_MULTIPLY); HDB_DECREASE_STACK_SIZE(2); HDB_INCREASE_STACK_SIZE(1); break;
        case TOKEN_FORWARD_SLASH:   emit_byte(OP_DIVIDE); HDB_DECREASE_STACK_SIZE(2); HDB_INCREASE_STACK_SIZE(1); break;
        default:
            return; // unreachable
    }
}

static void grouping(void) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(void) {
    double value = strtod(parser.previous.start, NULL);
    emit_constant(value);
}

static void unary(void) {
    hdb_token_type_t operator_type = parser.previous.type;

    // Compile the operand.
    parse_precedence(PREC_UNARY);

    // Emit the operator instruction.
    switch(operator_type) {
        case TOKEN_MINUS:           emit_byte(OP_NEGATE); break;
        default:
            return; // unreachable
    }
}

hdb_parse_rule_t rules[] = {
        [TOKEN_DOUBLE_QUOTE]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_PERCENT]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_AMPERSAND]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_BANG]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_LEFT_PAREN]                          = { grouping, NULL, PREC_NONE},
        [TOKEN_RIGHT_PAREN]                         = {NULL, NULL, PREC_NONE},
        [TOKEN_ASTERISK]                            = {NULL, binary, PREC_FACTOR},
        [TOKEN_PLUS]                                = {NULL, binary, PREC_TERM},
        [TOKEN_COMMA]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_MINUS]                               = {unary, binary, PREC_TERM},
        [TOKEN_PERIOD]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_BACKSLASH]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_FORWARD_SLASH]                       = {NULL, binary, PREC_FACTOR},
        [TOKEN_COLON]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_SEMICOLON]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_LESS_THAN]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_EQUALS]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_NOT_EQUAL]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_LESS_EQUAL]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_GREATER_EQUAL]                       = {NULL, NULL, PREC_NONE},
        [TOKEN_GREATER_THAN]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_QUESTION_MARK]                       = {NULL, NULL, PREC_NONE},
        [TOKEN_LEFT_BRACKET]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_RIGHT_BRACKET]                       = {NULL, NULL, PREC_NONE},
        [TOKEN_CIRCUMFLEX]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_VERTICAL_BAR]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_LEFT_BRACE]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_RIGHT_BRACE]                         = {NULL, NULL, PREC_NONE},
        [TOKEN_STRING]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_IDENTIFIER]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_ENCLOSED_IDENTIFIER]                 = {NULL, NULL, PREC_NONE},
        [TOKEN_NUMBER]                              = {number, NULL, PREC_NONE},
        [TOKEN_ERROR]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_EOF]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_ABSOLUTE]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_ACTION]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_ADD]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_AFTER]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_ALL]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_ALLOCATE]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_ALTER]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_AND]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_ANY]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_ARE]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_ARRAY]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_AS]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_ASC]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_ASSERTION]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_AT]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_AUTHORIZATION]                       = {NULL, NULL, PREC_NONE},
        [TOKEN_BEFORE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_BEGIN]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_BETWEEN]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_BINARY]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_BIT]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_BLOB]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_BOOLEAN]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_BOTH]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_BREADTH]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_BY]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_CALL]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_CASCADE]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_CASCADED]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_CASE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_CAST]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_CATALOG]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_CHAR]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_CHARACTER]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_CHECK]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_CLOB]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_CLOSE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_COLLATE]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_COLLATION]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_COLUMN]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_COMMIT]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_CONDITION]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_CONNECT]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_CONNECTION]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_CONSTRAINT]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_CONSTRAINTS]                         = {NULL, NULL, PREC_NONE},
        [TOKEN_CONSTRUCTOR]                         = {NULL, NULL, PREC_NONE},
        [TOKEN_CONTINUE]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_CORRESPONDING]                       = {NULL, NULL, PREC_NONE},
        [TOKEN_CREATE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_CROSS]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_CUBE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_CURRENT]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_CURRENT_DATE]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP]     = {NULL, NULL, PREC_NONE},
        [TOKEN_CURRENT_TRANSFORM_GROUP_FOR_TYPE]    = {NULL, NULL, PREC_NONE},
        [TOKEN_CURRENT_PATH]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_CURRENT_ROLE]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_CURRENT_TIME]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_CURRENT_TIMESTAMP]                   = {NULL, NULL, PREC_NONE},
        [TOKEN_CURRENT_USER]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_CURSOR]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_CYCLE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_DATA]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_DATE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_DAY]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_DEALLOCATE]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_DEC]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_DECIMAL]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_DECLARE]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_DEFAULT]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_DEFERRABLE]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_DEFERRED]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_DELETE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_DEPTH]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_DEREF]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_DESC]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_DESCRIBE]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_DESCRIPTOR]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_DETERMINISTIC]                       = {NULL, NULL, PREC_NONE},
        [TOKEN_DIAGNOSTICS]                         = {NULL, NULL, PREC_NONE},
        [TOKEN_DISCONNECT]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_DISTINCT]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_DO]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_DOMAIN]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_DOUBLE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_DROP]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_DYNAMIC]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_EACH]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_ELSE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_ELSEIF]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_END]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_END_EXEC]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_EQUALS_KEYWORD]                      = {NULL, NULL, PREC_NONE},
        [TOKEN_ESCAPE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_EXCEPT]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_EXCEPTION]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_EXEC]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_EXECUTE]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_EXISTS]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_EXIT]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_EXTERNAL]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_FALSE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_FETCH]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_FIRST]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_FLOAT]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_FOR]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_FOREIGN]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_FOUND]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_FROM]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_FREE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_FULL]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_FUNCTION]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_GENERAL]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_GET]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_GLOBAL]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_GO]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_GOTO]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_GRANT]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_GROUP]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_GROUPING]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_HANDLE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_HAVING]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_HOLD]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_HOUR]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_IDENTITY]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_IF]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_IMMEDIATE]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_IN]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_INDICATOR]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_INITIALLY]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_INNER]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_INOUT]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_INPUT]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_INSERT]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_INT]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_INTEGER]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_INTERSECT]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_INTERVAL]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_INTO]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_IS]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_ISOLATION]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_JOIN]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_KEY]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_LANGUAGE]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_LARGE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_LAST]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_LATERAL]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_LEADING]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_LEAVE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_LEFT]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_LEVEL]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_LIKE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_LOCAL]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_LOCALTIME]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_LOCALTIMESTAMP]                      = {NULL, NULL, PREC_NONE},
        [TOKEN_LOCATOR]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_LOOP]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_MAP]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_MATCH]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_METHOD]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_MINUTE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_MODIFIES]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_MODULE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_MONTH]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_NAMES]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_NATIONAL]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_NATURAL]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_NCHAR]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_NCLOB]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_NESTING]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_NEW]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_NEXT]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_NO]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_NONE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_NOT]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_NULL]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_NUMERIC]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_OBJECT]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_OF]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_OLD]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_ON]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_ONLY]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_OPEN]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_OPTION]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_OR]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_ORDER]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_ORDINALITY]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_OUT]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_OUTER]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_OUTPUT]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_OVERLAPS]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_PAD]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_PARAMETER]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_PARTIAL]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_PATH]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_PRECISION]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_PREPARE]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_PRESERVE]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_PRIMARY]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_PRIOR]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_PRIVILEGES]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_PROCEDURE]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_PUBLIC]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_READ]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_READS]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_REAL]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_RECURSIVE]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_REDO]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_REF]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_REFERENCES]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_REFERENCING]                         = {NULL, NULL, PREC_NONE},
        [TOKEN_RELATIVE]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_RELEASE]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_REPEAT]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_RESIGNAL]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_RESTRICT]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_RESULT]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_RETURN]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_RETURNS]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_REVOKE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_RIGHT]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_ROLE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_ROLLBACK]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_ROLLUP]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_ROUTINE]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_ROW]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_ROWS]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_SAVEPOINT]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_SCHEMA]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_SCROLL]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_SEARCH]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_SECOND]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_SECTION]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_SELECT]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_SESSION]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_SESSION_USER]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_SET]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_SETS]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_SIGNAL]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_SIMILAR]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_SIZE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_SMALLINT]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_SOME]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_SPACE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_SPECIFIC]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_SPECIFICTYPE]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_SQL]                                 = {NULL, NULL, PREC_NONE},
        [TOKEN_SQLEXCEPTION]                        = {NULL, NULL, PREC_NONE},
        [TOKEN_SQLSTATE]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_SQLWARNING]                          = {NULL, NULL, PREC_NONE},
        [TOKEN_START]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_STATE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_STATIC]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_SYSTEM_USER]                         = {NULL, NULL, PREC_NONE},
        [TOKEN_TABLE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_TEMPORARY]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_THEN]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_TIME]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_TIMESTAMP]                           = {NULL, NULL, PREC_NONE},
        [TOKEN_TIMEZONE_HOUR]                       = {NULL, NULL, PREC_NONE},
        [TOKEN_TIMEZONE_MINUTE]                     = {NULL, NULL, PREC_NONE},
        [TOKEN_TO]                                  = {NULL, NULL, PREC_NONE},
        [TOKEN_TRAILING]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_TRANSACTION]                         = {NULL, NULL, PREC_NONE},
        [TOKEN_TRANSLATION]                         = {NULL, NULL, PREC_NONE},
        [TOKEN_TREAT]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_TRIGGER]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_TRUE]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_UNDER]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_UNDO]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_UNION]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_UNIQUE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_UNKNOWN]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_UNNEST]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_UNTIL]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_UPDATE]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_USAGE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_USER]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_USING]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_VALUE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_VALUES]                              = {NULL, NULL, PREC_NONE},
        [TOKEN_VARCHAR]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_VARYING]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_VIEW]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_WHEN]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_WHENEVER]                            = {NULL, NULL, PREC_NONE},
        [TOKEN_WHERE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_WHILE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_WITH]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_WITHOUT]                             = {NULL, NULL, PREC_NONE},
        [TOKEN_WORK]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_WRITE]                               = {NULL, NULL, PREC_NONE},
        [TOKEN_YEAR]                                = {NULL, NULL, PREC_NONE},
        [TOKEN_ZONE]                                = {NULL, NULL, PREC_NONE},
};

static void parse_precedence(hdb_precedence_t precedence) {
    advance();
    hdb_parse_fn prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        error("Expect expression.");
        return;
    }

    prefix_rule();

    while(precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        hdb_parse_fn infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule();
    }
}

static hdb_parse_rule_t* get_rule(hdb_token_type_t operator_type) {
    return &rules[operator_type];
}

static void expression(void) {
    parse_precedence(PREC_ASSIGNMENT);
}

bool hdb_compiler_compile(const char* source, hdb_chunk_t* chunk) {
    stack_high_water_mark = stack_size = 0;

    hdb_scanner_init(source);
    compiling_chunk = chunk;
    advance();
    expression();

    // Add the high water mark of the stack to the chunk. This allows for
    // less array bounds tests when executing the code.
    compiling_chunk->stack_high_water_mark = stack_high_water_mark;
    consume(TOKEN_EOF, "Expect end of expression.");
    end_compiler();
    return !parser.had_error;
}
