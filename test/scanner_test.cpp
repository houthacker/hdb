#include "gtest/gtest.h"

extern "C" {
#include <scanner.h>

    typedef struct {
        const char* value;
        hdb_token_type_t type;
    } pair_t;

}

class HdbScannerFixture : public ::testing::Test {
protected:

    virtual void SetUp() {
        hdb_scanner_create();
    }

    virtual void TearDown() {
        hdb_scanner_free();
    }
};

using HdbScannerFixtureDeathTest = HdbScannerFixture;

TEST_F(HdbScannerFixture, scan_single_token) {
    hdb_scanner_init("select");

    hdb_token_t t1 = hdb_scanner_scan_token();
    hdb_token_t t2 = hdb_scanner_scan_token();

    EXPECT_EQ(t1.line, 1);
    EXPECT_EQ(t1.length, 6);
    EXPECT_NE(t1.start, nullptr);
    EXPECT_EQ(t1.type, TOKEN_SELECT);

    EXPECT_EQ(t2.line, 1);
    EXPECT_EQ(t2.length, 0);
    EXPECT_NE(t2.start, nullptr);
    EXPECT_EQ(t2.type, TOKEN_EOF);
}

TEST_F(HdbScannerFixture, test_user_identifier) {
    hdb_scanner_init("_alias");

    hdb_token_t token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_IDENTIFIER);
}

TEST_F(HdbScannerFixture, test_all_token_types) {

    const pair_t all[] = {
            {"''",                                              TOKEN_DOUBLE_QUOTE},
            {"%",                                               TOKEN_PERCENT},
            {"&",                                               TOKEN_AMPERSAND},
            {"(",                                               TOKEN_LEFT_PAREN},
            {")",                                               TOKEN_RIGHT_PAREN},
            {"*",                                               TOKEN_ASTERISK},
            {"+",                                               TOKEN_PLUS},
            {",",                                               TOKEN_COMMA},
            {"-",                                               TOKEN_MINUS},
            {".",                                               TOKEN_PERIOD},
            {"\\",                                              TOKEN_BACKSLASH},
            {"/",                                               TOKEN_FORWARD_SLASH},
            {":",                                               TOKEN_COLON},
            {";",                                               TOKEN_SEMICOLON},
            {"!",                                               TOKEN_BANG},
            {"!=",                                              TOKEN_NOT_EQUAL},
            {"<",                                               TOKEN_LESS_THAN},
            {"=",                                               TOKEN_EQUALS},
            {"<>",                                              TOKEN_NOT_EQUAL},
            {"<=",                                              TOKEN_LESS_EQUAL},
            {">=",                                              TOKEN_GREATER_EQUAL},
            {">",                                               TOKEN_GREATER_THAN},
            {"?",                                               TOKEN_QUESTION_MARK},
            {"[",                                               TOKEN_LEFT_BRACKET},
            {"]",                                               TOKEN_RIGHT_BRACKET},
            {"^",                                               TOKEN_CIRCUMFLEX},
            {"|",                                               TOKEN_VERTICAL_BAR},
            {"{",                                               TOKEN_LEFT_BRACE},
            {"}",                                               TOKEN_RIGHT_BRACE},
            {"'string'",                                        TOKEN_STRING},
            {"'string\n with newlines'",                        TOKEN_STRING},
            {"'string\n with newlines \\' and single quotes.'", TOKEN_STRING},
            {"identifier",                                      TOKEN_IDENTIFIER},
            {"`identifier`",                                    TOKEN_ENCLOSED_IDENTIFIER},
            {"\"identifier\"",                                  TOKEN_ENCLOSED_IDENTIFIER},
            {"1",                                               TOKEN_NUMBER},
            {"9223372036854775807",                             TOKEN_NUMBER},
            {"1.337",                                           TOKEN_NUMBER},
            {"absolute",                                        TOKEN_ABSOLUTE},
            {"action",                                          TOKEN_ACTION},
            {"add",                                             TOKEN_ADD},
            {"after",                                           TOKEN_AFTER},
            {"all",                                             TOKEN_ALL},
            {"allocate",                                        TOKEN_ALLOCATE},
            {"alter",                                           TOKEN_ALTER},
            {"and",                                             TOKEN_AND},
            {"any",                                             TOKEN_ANY},
            {"are",                                             TOKEN_ARE},
            {"array",                                           TOKEN_ARRAY},
            {"as",                                              TOKEN_AS},
            {"asc",                                             TOKEN_ASC},
            {"assertion",                                       TOKEN_ASSERTION},
            {"at",                                              TOKEN_AT},
            {"authorization",                                   TOKEN_AUTHORIZATION},
            {"before",                                          TOKEN_BEFORE},
            {"begin",                                           TOKEN_BEGIN},
            {"between",                                         TOKEN_BETWEEN},
            {"binary",                                          TOKEN_BINARY},
            {"bit",                                             TOKEN_BIT},
            {"blob",                                            TOKEN_BLOB},
            {"boolean",                                         TOKEN_BOOLEAN},
            {"both",                                            TOKEN_BOTH},
            {"breadth",                                         TOKEN_BREADTH},
            {"by",                                              TOKEN_BY},
            {"call",                                            TOKEN_CALL},
            {"cascade",                                         TOKEN_CASCADE},
            {"cascaded",                                        TOKEN_CASCADED},
            {"case",                                            TOKEN_CASE},
            {"cast",                                            TOKEN_CAST},
            {"catalog",                                         TOKEN_CATALOG},
            {"char",                                            TOKEN_CHAR},
            {"character",                                       TOKEN_CHARACTER},
            {"check",                                           TOKEN_CHECK},
            {"clob",                                            TOKEN_CLOB},
            {"close",                                           TOKEN_CLOSE},
            {"collate",                                         TOKEN_COLLATE},
            {"collation",                                       TOKEN_COLLATION},
            {"column",                                          TOKEN_COLUMN},
            {"commit",                                          TOKEN_COMMIT},
            {"condition",                                       TOKEN_CONDITION},
            {"connect",                                         TOKEN_CONNECT},
            {"connection",                                      TOKEN_CONNECTION},
            {"constraint",                                      TOKEN_CONSTRAINT},
            {"constraints",                                     TOKEN_CONSTRAINTS},
            {"constructor",                                     TOKEN_CONSTRUCTOR},
            {"continue",                                        TOKEN_CONTINUE},
            {"corresponding",                                   TOKEN_CORRESPONDING},
            {"create",                                          TOKEN_CREATE},
            {"cross",                                           TOKEN_CROSS},
            {"cube",                                            TOKEN_CUBE},
            {"current",                                         TOKEN_CURRENT},
            {"current_date",                                    TOKEN_CURRENT_DATE},
            {"current_default_transform_group",                 TOKEN_CURRENT_DEFAULT_TRANSFORM_GROUP},
            {"current_transform_group_for_type",                TOKEN_CURRENT_TRANSFORM_GROUP_FOR_TYPE},
            {"current_path",                                    TOKEN_CURRENT_PATH},
            {"current_role",                                    TOKEN_CURRENT_ROLE},
            {"current_time",                                    TOKEN_CURRENT_TIME},
            {"current_timestamp",                               TOKEN_CURRENT_TIMESTAMP},
            {"current_user",                                    TOKEN_CURRENT_USER},
            {"cursor",                                          TOKEN_CURSOR},
            {"cycle",                                           TOKEN_CYCLE},
            {"data",                                            TOKEN_DATA},
            {"date",                                            TOKEN_DATE},
            {"day",                                             TOKEN_DAY},
            {"deallocate",                                      TOKEN_DEALLOCATE},
            {"dec",                                             TOKEN_DEC},
            {"decimal",                                         TOKEN_DECIMAL},
            {"declare",                                         TOKEN_DECLARE},
            {"default",                                         TOKEN_DEFAULT},
            {"deferrable",                                      TOKEN_DEFERRABLE},
            {"deferred",                                        TOKEN_DEFERRED},
            {"delete",                                          TOKEN_DELETE},
            {"depth",                                           TOKEN_DEPTH},
            {"deref",                                           TOKEN_DEREF},
            {"desc",                                            TOKEN_DESC},
            {"describe",                                        TOKEN_DESCRIBE},
            {"descriptor",                                      TOKEN_DESCRIPTOR},
            {"deterministic",                                   TOKEN_DETERMINISTIC},
            {"diagnostics",                                     TOKEN_DIAGNOSTICS},
            {"disconnect",                                      TOKEN_DISCONNECT},
            {"distinct",                                        TOKEN_DISTINCT},
            {"do",                                              TOKEN_DO},
            {"domain",                                          TOKEN_DOMAIN},
            {"double",                                          TOKEN_DOUBLE},
            {"drop",                                            TOKEN_DROP},
            {"dynamic",                                         TOKEN_DYNAMIC},
            {"each",                                            TOKEN_EACH},
            {"else",                                            TOKEN_ELSE},
            {"elseif",                                          TOKEN_ELSEIF},
            {"end",                                             TOKEN_END},
            {"end_exec",                                        TOKEN_END_EXEC},
            {"equals",                                          TOKEN_EQUALS_KEYWORD},
            {"escape",                                          TOKEN_ESCAPE},
            {"except",                                          TOKEN_EXCEPT},
            {"exception",                                       TOKEN_EXCEPTION},
            {"exec",                                            TOKEN_EXEC},
            {"execute",                                         TOKEN_EXECUTE},
            {"exists",                                          TOKEN_EXISTS},
            {"exit",                                            TOKEN_EXIT},
            {"external",                                        TOKEN_EXTERNAL},
            {"false",                                           TOKEN_FALSE},
            {"fetch",                                           TOKEN_FETCH},
            {"first",                                           TOKEN_FIRST},
            {"float",                                           TOKEN_FLOAT},
            {"for",                                             TOKEN_FOR},
            {"foreign",                                         TOKEN_FOREIGN},
            {"found",                                           TOKEN_FOUND},
            {"from",                                            TOKEN_FROM},
            {"free",                                            TOKEN_FREE},
            {"full",                                            TOKEN_FULL},
            {"function",                                        TOKEN_FUNCTION},
            {"general",                                         TOKEN_GENERAL},
            {"get",                                             TOKEN_GET},
            {"global",                                          TOKEN_GLOBAL},
            {"go",                                              TOKEN_GO},
            {"goto",                                            TOKEN_GOTO},
            {"grant",                                           TOKEN_GRANT},
            {"group",                                           TOKEN_GROUP},
            {"grouping",                                        TOKEN_GROUPING},
            {"handle",                                          TOKEN_HANDLE},
            {"having",                                          TOKEN_HAVING},
            {"hold",                                            TOKEN_HOLD},
            {"hour",                                            TOKEN_HOUR},
            {"identity",                                        TOKEN_IDENTITY},
            {"if",                                              TOKEN_IF},
            {"immediate",                                       TOKEN_IMMEDIATE},
            {"in",                                              TOKEN_IN},
            {"indicator",                                       TOKEN_INDICATOR},
            {"initially",                                       TOKEN_INITIALLY},
            {"inner",                                           TOKEN_INNER},
            {"inout",                                           TOKEN_INOUT},
            {"input",                                           TOKEN_INPUT},
            {"insert",                                          TOKEN_INSERT},
            {"int",                                             TOKEN_INT},
            {"integer",                                         TOKEN_INTEGER},
            {"intersect",                                       TOKEN_INTERSECT},
            {"interval",                                        TOKEN_INTERVAL},
            {"into",                                            TOKEN_INTO},
            {"is",                                              TOKEN_IS},
            {"isolation",                                       TOKEN_ISOLATION},
            {"join",                                            TOKEN_JOIN},
            {"key",                                             TOKEN_KEY},
            {"language",                                        TOKEN_LANGUAGE},
            {"large",                                           TOKEN_LARGE},
            {"last",                                            TOKEN_LAST},
            {"lateral",                                         TOKEN_LATERAL},
            {"leading",                                         TOKEN_LEADING},
            {"leave",                                           TOKEN_LEAVE},
            {"left",                                            TOKEN_LEFT},
            {"level",                                           TOKEN_LEVEL},
            {"like",                                            TOKEN_LIKE},
            {"local",                                           TOKEN_LOCAL},
            {"localtime",                                       TOKEN_LOCALTIME},
            {"localtimestamp",                                  TOKEN_LOCALTIMESTAMP},
            {"locator",                                         TOKEN_LOCATOR},
            {"loop",                                            TOKEN_LOOP},
            {"map",                                             TOKEN_MAP},
            {"match",                                           TOKEN_MATCH},
            {"method",                                          TOKEN_METHOD},
            {"minute",                                          TOKEN_MINUTE},
            {"modifies",                                        TOKEN_MODIFIES},
            {"module",                                          TOKEN_MODULE},
            {"month",                                           TOKEN_MONTH},
            {"names",                                           TOKEN_NAMES},
            {"national",                                        TOKEN_NATIONAL},
            {"natural",                                         TOKEN_NATURAL},
            {"nchar",                                           TOKEN_NCHAR},
            {"nclob",                                           TOKEN_NCLOB},
            {"nesting",                                         TOKEN_NESTING},
            {"new",                                             TOKEN_NEW},
            {"next",                                            TOKEN_NEXT},
            {"no",                                              TOKEN_NO},
            {"none",                                            TOKEN_NONE},
            {"not",                                             TOKEN_NOT},
            {"null",                                            TOKEN_NULL},
            {"numeric",                                         TOKEN_NUMERIC},
            {"object",                                          TOKEN_OBJECT},
            {"of",                                              TOKEN_OF},
            {"old",                                             TOKEN_OLD},
            {"on",                                              TOKEN_ON},
            {"only",                                            TOKEN_ONLY},
            {"open",                                            TOKEN_OPEN},
            {"option",                                          TOKEN_OPTION},
            {"or",                                              TOKEN_OR},
            {"order",                                           TOKEN_ORDER},
            {"ordinality",                                      TOKEN_ORDINALITY},
            {"out",                                             TOKEN_OUT},
            {"outer",                                           TOKEN_OUTER},
            {"output",                                          TOKEN_OUTPUT},
            {"overlaps",                                        TOKEN_OVERLAPS},
            {"pad",                                             TOKEN_PAD},
            {"parameter",                                       TOKEN_PARAMETER},
            {"partial",                                         TOKEN_PARTIAL},
            {"path",                                            TOKEN_PATH},
            {"precision",                                       TOKEN_PRECISION},
            {"prepare",                                         TOKEN_PREPARE},
            {"preserve",                                        TOKEN_PRESERVE},
            {"primary",                                         TOKEN_PRIMARY},
            {"prior",                                           TOKEN_PRIOR},
            {"privileges",                                      TOKEN_PRIVILEGES},
            {"procedure",                                       TOKEN_PROCEDURE},
            {"public",                                          TOKEN_PUBLIC},
            {"read",                                            TOKEN_READ},
            {"reads",                                           TOKEN_READS},
            {"real",                                            TOKEN_REAL},
            {"recursive",                                       TOKEN_RECURSIVE},
            {"redo",                                            TOKEN_REDO},
            {"ref",                                             TOKEN_REF},
            {"references",                                      TOKEN_REFERENCES},
            {"referencing",                                     TOKEN_REFERENCING},
            {"relative",                                        TOKEN_RELATIVE},
            {"release",                                         TOKEN_RELEASE},
            {"repeat",                                          TOKEN_REPEAT},
            {"resignal",                                        TOKEN_RESIGNAL},
            {"restrict",                                        TOKEN_RESTRICT},
            {"result",                                          TOKEN_RESULT},
            {"return",                                          TOKEN_RETURN},
            {"returns",                                         TOKEN_RETURNS},
            {"revoke",                                          TOKEN_REVOKE},
            {"right",                                           TOKEN_RIGHT},
            {"role",                                            TOKEN_ROLE},
            {"rollback",                                        TOKEN_ROLLBACK},
            {"rollup",                                          TOKEN_ROLLUP},
            {"routine",                                         TOKEN_ROUTINE},
            {"row",                                             TOKEN_ROW},
            {"rows",                                            TOKEN_ROWS},
            {"savepoint",                                       TOKEN_SAVEPOINT},
            {"schema",                                          TOKEN_SCHEMA},
            {"scroll",                                          TOKEN_SCROLL},
            {"search",                                          TOKEN_SEARCH},
            {"second",                                          TOKEN_SECOND},
            {"section",                                         TOKEN_SECTION},
            {"select",                                          TOKEN_SELECT},
            {"session",                                         TOKEN_SESSION},
            {"session_user",                                    TOKEN_SESSION_USER},
            {"set",                                             TOKEN_SET},
            {"sets",                                            TOKEN_SETS},
            {"signal",                                          TOKEN_SIGNAL},
            {"similar",                                         TOKEN_SIMILAR},
            {"size",                                            TOKEN_SIZE},
            {"smallint",                                        TOKEN_SMALLINT},
            {"some",                                            TOKEN_SOME},
            {"space",                                           TOKEN_SPACE},
            {"specific",                                        TOKEN_SPECIFIC},
            {"specifictype",                                    TOKEN_SPECIFICTYPE},
            {"sql",                                             TOKEN_SQL},
            {"sqlexception",                                    TOKEN_SQLEXCEPTION},
            {"sqlstate",                                        TOKEN_SQLSTATE},
            {"sqlwarning",                                      TOKEN_SQLWARNING},
            {"start",                                           TOKEN_START},
            {"state",                                           TOKEN_STATE},
            {"static",                                          TOKEN_STATIC},
            {"system_user",                                     TOKEN_SYSTEM_USER},
            {"table",                                           TOKEN_TABLE},
            {"temporary",                                       TOKEN_TEMPORARY},
            {"then",                                            TOKEN_THEN},
            {"time",                                            TOKEN_TIME},
            {"timestamp",                                       TOKEN_TIMESTAMP},
            {"timezone_hour",                                   TOKEN_TIMEZONE_HOUR},
            {"timezone_minute",                                 TOKEN_TIMEZONE_MINUTE},
            {"to",                                              TOKEN_TO},
            {"trailing",                                        TOKEN_TRAILING},
            {"transaction",                                     TOKEN_TRANSACTION},
            {"translation",                                     TOKEN_TRANSLATION},
            {"treat",                                           TOKEN_TREAT},
            {"trigger",                                         TOKEN_TRIGGER},
            {"true",                                            TOKEN_TRUE},
            {"under",                                           TOKEN_UNDER},
            {"undo",                                            TOKEN_UNDO},
            {"union",                                           TOKEN_UNION},
            {"unique",                                          TOKEN_UNIQUE},
            {"unknown",                                         TOKEN_UNKNOWN},
            {"unnest",                                          TOKEN_UNNEST},
            {"until",                                           TOKEN_UNTIL},
            {"update",                                          TOKEN_UPDATE},
            {"usage",                                           TOKEN_USAGE},
            {"user",                                            TOKEN_USER},
            {"using",                                           TOKEN_USING},
            {"value",                                           TOKEN_VALUE},
            {"values",                                          TOKEN_VALUES},
            {"varchar",                                         TOKEN_VARCHAR},
            {"varying",                                         TOKEN_VARYING},
            {"view",                                            TOKEN_VIEW},
            {"when",                                            TOKEN_WHEN},
            {"whenever",                                        TOKEN_WHENEVER},
            {"where",                                           TOKEN_WHERE},
            {"while",                                           TOKEN_WHILE},
            {"with",                                            TOKEN_WITH},
            {"without",                                         TOKEN_WITHOUT},
            {"work",                                            TOKEN_WORK},
            {"write",                                           TOKEN_WRITE},
            {"year",                                            TOKEN_YEAR},
            {"zone",                                            TOKEN_ZONE}
    };

    for (auto pair : all) {
        hdb_scanner_init(pair.value);
        hdb_token_t token = hdb_scanner_scan_token();
        EXPECT_EQ(token.type, pair.type);
    }
}

TEST_F(HdbScannerFixture, test_unsupported_token) {
    hdb_scanner_init("$unsupported");

    hdb_token_t  token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_ERROR);
}

TEST_F(HdbScannerFixture, test_unexpected_character) {
    hdb_scanner_init("\xF0\x9F\x98\x80"); // Unicode 1F600

    hdb_token_t token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_ERROR);
}

TEST_F(HdbScannerFixture, test_unterminated_string) {
    hdb_scanner_init("'unterminated");

    hdb_token_t token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_ERROR);
}

TEST_F(HdbScannerFixture, test_start_with_escaped_quote) {
    hdb_scanner_init("'\\'an_escaped_quote'");

    hdb_token_t token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_STRING);
}

TEST_F(HdbScannerFixture, test_invalid_enclosed_identifier) {
    hdb_scanner_init("`$foo`");

    hdb_token_t token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_ERROR);
}

TEST_F(HdbScannerFixture, test_unterminated_enclosed_identifier) {
    hdb_scanner_init("`my_table");

    hdb_token_t token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_ERROR);
}

TEST_F(HdbScannerFixture, test_scan_whitespace) {
    hdb_scanner_init(" \r\t");

    hdb_token_t token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_EOF);
}

TEST_F(HdbScannerFixture, test_scan_newline) {
    hdb_scanner_init("\n\n\n");

    hdb_token_t token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_EOF);
    EXPECT_EQ(token.line, 4);
}

TEST_F(HdbScannerFixture, test_single_line_comment) {
    hdb_scanner_init("// This is a single line comment");

    hdb_token_t token = hdb_scanner_scan_token();
    EXPECT_EQ(token.type, TOKEN_EOF);
    EXPECT_EQ(token.line, 1);
}

TEST_F(HdbScannerFixtureDeathTest, create_scanner_twice_no_error) {
    hdb_scanner_create();
}

TEST_F(HdbScannerFixtureDeathTest, get_token_without_init) {
    EXPECT_EXIT(hdb_scanner_scan_token(),
                testing::KilledBySignal(SIGSEGV), "");
}