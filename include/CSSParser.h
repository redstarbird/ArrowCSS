#include "../include/CSSAST.h"
#include "Lexer.h"

typedef struct Parser
{
    // The lexer feeding tokens to the parser
    struct Lexer *lexer;
    // The token currently being looked at
    struct Token currentToken;
    // The previous token
    struct Token prevToken;
    // Arena to contain the ASTNode structs
    struct MemoryArena *arena;
    // Tracks whether the CSS has syntax errors
    bool ErrorDiscovered;
} Parser;

/** @brief Initialise a parser instance */
void ParserInit(struct Parser *parser, struct Lexer *lexer, struct MemoryArena *arena);

struct CSSAST *ParseCSSToAST(struct Parser *parser);