#ifndef LEXER_H
#define LEXER_H

#include "CSSAST.h"
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
    const char *input;

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

    // Flag to indicate if the lexer is peeking at the next token without consuming it
    bool lexerPeeking;

    // Flag to indicate if the lexer is parsing a large text/value blob including whitespaces
    bool expectsValue;

    // Flag to indicate if the lexer is parsing a selector blob for a ruleset
    bool expectsSelector;
};

/** @brief Initialise a lexer instance */
void LexerInit(struct Lexer *lexer, const char *input, size_t length, struct StringPool *string_pool);

/** @brief Fetches the next token from the lexer
 * @param lexer The lexer to get the next token from
 * @return A struct representing the next token
 */
struct Token LexerNextToken(struct Lexer *lexer);

inline char LexerPeek(struct Lexer *lexer)
{
    return *lexer->cursor;
}

#endif // LEXER_H