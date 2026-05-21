#ifndef PARSER_H
#define PARSER_H
#include "CSSAST.h"
#include "../src/Lexer.h"

typedef struct
{
    // For error reporting
    const char *filename;
    // Should source maps be generated
    bool generateSourceMap;
    // Supress any console warnings
    bool silent;

    // Memory arena
    struct MemoryArena *arena;
    // String pool
    struct StringPool *pool;

} ArrowCssParseOptions;

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

struct CSSAST *ParseCSSToAST(char *fileContent, size_t length, struct MemoryArena *arena, struct StringPool *pool);

#endif