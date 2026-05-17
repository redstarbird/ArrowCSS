#include "../include/CSSParser.h"

// Move to the next function
static void advance(struct Parser *parser)
{
    parser->prevToken = parser->currentToken;
    parser->currentToken = LexerNextToken(parser->lexer);
}

// Checks if the current token is a specific type, if EOF then false is always returned
static bool check(Parser *parser, TokenType type)
{
    if (parser->currentToken.type == TOK_EOF)
        return false;
    return parser->currentToken.type == type;
}

static bool consume(Parser *parser, TokenType type, const char *error_message)
{
    // Check the current token is the expected token before consuming it
    if (check(parser, type))
    {
        advance(parser);
        return true;
    }

    // If not the expected token then the CSS is malformed
    printf("Syntax Error: %s\n", error_message);
    parser->ErrorDiscovered = true;
    return false;
}
