#include "testing.h"
#include "../src/Lexer.h"
#include <string.h>

void test_lexer_punctuation()
{
    const char *css = "\r{ } \n\n\r\r: ;\r\n";
    struct Lexer lexer;

    struct StringPool *pool = CreateStringPool(4096, MB(4), OOM_ABORT, NULL);

    LexerInit(&lexer, (char *)css, strlen(css), pool);

    struct Token t1 = LexerNextToken(&lexer);
    ASSERT_TRUE(t1.type == TOK_LBRACE, "Expected '{'");

    struct Token t2 = LexerNextToken(&lexer);
    ASSERT_TRUE(t2.type == TOK_RBRACE, "Expected '}'");

    struct Token t3 = LexerNextToken(&lexer);
    ASSERT_TRUE(t3.type == TOK_COLON, "Expected ':'");

    DestroyStringPool(pool);

    printf("[PASS] test_lexer_punctuation\n");
}

void test_lexer_at_rule()
{
    const char *css = "@media";
    struct Lexer lexer;
    LexerInit(&lexer, (char *)css, strlen(css), NULL);

    struct Token t = LexerNextToken(&lexer);
    ASSERT_TRUE(t.type == TOK_AT_RULE, "Expected TOK_AT_RULE");

    printf("[PASS] test_lexer_at_rule\n");
}

int main(void)
{
    test_lexer_punctuation();

    test_lexer_at_rule();

    return 0;
}