#include "testing.h"
#include "../src/Lexer.h"
#include "../src/CSSParser.h"

void test_parser_detects_missing_semicolon()
{
    // Missing semicolon
    const char *bad_css = ".btn { color: red }";
    struct Lexer lexer;
    struct Parser parser;
    struct MemoryArena *arena = CreateArena(1024, OOM_ABORT, NULL);
    struct StringPool *pool = CreateStringPool(64, 2048, OOM_ABORT, NULL);

    // Init parser and lexer
    LexerInit(&lexer, bad_css, strlen(bad_css), pool);
    ParserInit(&parser, &lexer, arena);

    struct CSSAST *ast = ParseStylesheet(&parser);

    ASSERT_TRUE(parser.ErrorDiscovered == true, "Parser should flag missing semicolon as an error");

    // Cleanup
    DestroyArena(arena);
    DestroyStringPool(pool);
}

int main(void)
{
    test_parser_detects_missing_semicolon();

    return 0;
}