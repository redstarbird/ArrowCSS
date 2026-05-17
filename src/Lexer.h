#ifndef LEXER_H
#define LEXER_H

#include "../include/CSSAST.h"
#include <stddef.h>
#include <stdbool.h>
#include <StringPool.h>

typedef enum LexerState
{
    // The normal lexer state
    LEXER_STATE_NORMAL,
    // Looking for the parameters in an @ rule (e.g, @media, @keyframes)
    LEXER_STATE_AT_RULE_PARAMS

} LexerState;

struct Lexer
{
    // Pointer to the input CSS string
    char *input;

    // Length of the input string
    size_t length;

    // Current position in the input
    char *cursor;

    // Current line number (for error reporting)
    int line;

    // Current column number (for error reporting)
    int column;

    // String pool for token values
    struct StringPool *string_pool;

    // The current state of the lexer
    enum LexerState state;
};

/** @brief Initialise a lexer instance */
void LexerInit(struct Lexer *lexer, const char *input, size_t length, struct StringPool *string_pool);

/** @brief Fetches the next token from the lexer
 * @param lexer The lexer to get the next token from
 * @return A struct representing the next token
 */
struct Token LexerNextToken(struct Lexer *lexer);

#endif // LEXER_H