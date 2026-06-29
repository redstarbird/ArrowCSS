#include "Lexer.h"
#include <ctype.h>

void LexerInit(struct Lexer *lexer, const char *input, size_t length, struct StringPool *string_pool)
{
    lexer->input = input;
    lexer->length = length;
    lexer->cursor = 0;
    lexer->line = 1;
    lexer->column = 0;
    lexer->string_pool = string_pool;
}

inline char LexerPeek(struct Lexer *lexer)
{
    return *lexer->cursor;
}

static inline char LexerAdvance(struct Lexer *lexer)
{
    char c = *lexer->cursor;
    lexer->cursor++;
    lexer->column++;
    if (LexerPeek(lexer) == '\n')
    {
        lexer->line++;
        lexer->column = 0;
    }
    return c;
}

static inline bool LexerIsAtEnd(struct Lexer *lexer)
{
    return lexer->cursor >= lexer->input + lexer->length;
}

static void LexerSkipWhitespaceAndComments(struct Lexer *lexer)
{
    while (!LexerIsAtEnd(lexer))
    {
        char c = *lexer->cursor;

        // Skip whitespace characters
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            LexerAdvance(lexer);
        }
        // Skip comments (/* ... */)
        else if (c == '/' && *(lexer->cursor + 1) == '*')
        {
            LexerAdvance(lexer); // Skip '/'
            LexerAdvance(lexer); // Skip '*'

            // Look for the end of the comment
            while (!LexerIsAtEnd(lexer))
            {
                if (*lexer->cursor == '*' && *(lexer->cursor + 1) == '/')
                {
                    LexerAdvance(lexer); // Skip '*'
                    LexerAdvance(lexer); // Skip '/'
                    break;
                }

                LexerAdvance(lexer);
            }
        }
        // Whitespace and comments have been skipped, break out of the loop
        else
        {
            break;
        }
    }
}

struct Token LexerNextToken(struct Lexer *lexer)
{
    LexerSkipWhitespaceAndComments(lexer);

    // Create a default EOF token to return the end of input is reached
    struct Token token = {.type = TOK_EOF, .value = {NULL}};

    if (LexerIsAtEnd(lexer))
    {
        return token;
    }

    const char *start = lexer->cursor;
    char c = LexerPeek(lexer);

    // Specific handling for finding rule parameters
    if (lexer->state == LEXER_STATE_AT_RULE_PARAMS)
    {
        // If the current char is a { or ; then the paramters are over
        if (c == '{' || c == ';')
        {
            // Put the state back to normal
            lexer->state = LEXER_STATE_NORMAL;

            // Let the normal mode handle the { or ; character on the next loop
            return LexerNextToken(lexer);
        }

        while (!LexerIsAtEnd(lexer) && LexerPeek(lexer) != '{' && LexerPeek(lexer) != ';')
        {
            LexerAdvance(lexer);
        }

        // Get rid of any extra whitespace from the end of the parameter string
        const char *end = lexer->cursor;
        while (end > start && isspace(*(end - 1)))
        {
            end--;
        }

        // Return the token as an identifier
        token.type = TOK_PARAMS;

        // Only consume the token if the lexer is not peeking
        if (!lexer->lexerPeeking)
        {
            token.value = InternString(lexer->string_pool, start, end - start);
        }
        else
        {
            token.value = (struct StringView){0};
        }
        return token;
    }

    // Normal state

    c = LexerAdvance(lexer);

    switch (c)
    {
    case '{':
        token.type = TOK_LBRACE;
        return token;
    case '}':
        token.type = TOK_RBRACE;
        return token;
    case ':':
        token.type = TOK_COLON;
        return token;
    case ';':
        token.type = TOK_SEMICOLON;
        return token;
    }

    // Handle at-rules (e.g., @media, @keyframes)
    if (c == '@')
    {
        // Consume the at-rule name, until a whitespace or a special character is found
        while (!LexerIsAtEnd(lexer) && !isspace(LexerPeek(lexer)) && LexerPeek(lexer) != '{' && LexerPeek(lexer) != ';')
        {
            LexerAdvance(lexer);
        }

        // Create a token for the at-rule
        size_t length = lexer->cursor - start;
        token.type = TOK_AT_RULE;

        // Only consume the token if the lexer is not peeking
        if (!lexer->lexerPeeking)
        {
            token.value = InternString(lexer->string_pool, start, length);
        }
        else
        {
            token.value = (struct StringView){0};
        }

        // Change the state to at rule parameters
        lexer->state = LEXER_STATE_AT_RULE_PARAMS;

        return token;
    }

    // Handle identifiers (selectors, property names, values)
    while (!LexerIsAtEnd(lexer))
    {
        char next = LexerPeek(lexer);
        if (isspace(next) || next == '{' || next == '}' || next == ':' || next == ';')
        {
            break;
        }
        LexerAdvance(lexer);
    }

    size_t length = lexer->cursor - start;
    token.type = TOK_IDENTIFIER;
    token.value = InternString(lexer->string_pool, start, length);
    return token;
}
