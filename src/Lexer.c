#include "Lexer.h"
#include <ctype.h>

void LexerInit(struct Lexer *lexer, const char *input, size_t length, struct StringPool *string_pool)
{
    lexer->input = input;
    lexer->length = length;
    lexer->cursor = (char *)input;
    lexer->line = 1;
    lexer->column = 0;
    lexer->string_pool = string_pool;
    lexer->lexerPeeking = false;
    lexer->expectsValue = false;
    lexer->expectsSelector = false;
}

extern inline char LexerPeek(struct Lexer *lexer);

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

#include <stdlib.h>

struct Token GetValueBlob(struct Lexer *lexer)
{
    // Reset expects value flag
    lexer->expectsValue = false;

    const char *start = lexer->cursor;

    // Store whether cursor is in a css string to avoid reading a ';' or '}' token from a string
    bool inQuotes = false;

    // Stores the character used for currently open quotes, " or '
    char quoteChar = '\0';

    while (*lexer->cursor != '\0')
    {
        char c = *lexer->cursor;

        // Handle quote state to avoid tokens in strings
        if (c == '"' || c == '\'')
        {
            if (!inQuotes)
            {
                inQuotes = true;
                quoteChar = c;
            }
            else if (c == quoteChar)
            {
                inQuotes = false;
            }
        }

        // Check for the end of the value blob
        if (!inQuotes && (c == ';' || c == '}'))
        {
            break;
        }

        // Advance cursor
        lexer->cursor++;
    }

    const char *end = lexer->cursor;

    while (end > start && isspace(*(end - 1)))
    {
        end--;
    }

    // Create token
    struct Token token;
    token.type = TOK_IDENTIFIER;

    // Only actually intern and return the string if the lexer is not peeking in a look-ahead operation
    if (!lexer->lexerPeeking)
    {
        token.value = InternString(lexer->string_pool, start, end - start);
    }
    else
    {
        token.value = (struct StringView){0};
    }

    // printf("Token blob: '%.*s', peeking: %i\n", (int)token.value.length, token.value.data, lexer->lexerPeeking);

    // Set the start pointer of the token to the beginning of the value blob
    token.start = (char *)start;

    return token;
}

struct Token GetRulesetSelector(struct Lexer *lexer)
{
    // Reset expects selector flag
    lexer->expectsSelector = false;

    const char *start = lexer->cursor;

    // Store whether cursor is in a css string to avoid reading a '{' token from a string
    bool inQuotes = false;

    // Stores the character used for currently open quotes, " or '
    char quoteChar = '\0';

    while (*lexer->cursor != '\0')
    {
        char c = *lexer->cursor;

        // Handle quote state to avoid tokens in strings
        if (c == '"' || c == '\'')
        {
            if (!inQuotes)
            {
                inQuotes = true;
                quoteChar = c;
            }
            else if (c == quoteChar)
            {
                inQuotes = false;
            }
        }

        // Check for the end of the selector blob
        if (!inQuotes && c == '{')
        {
            break;
        }

        // Advance cursor
        lexer->cursor++;
    }

    const char *end = lexer->cursor;

    // Trim whitespace at the end
    while (end > start && isspace(*(end - 1)))
    {
        end--;
    }

    // Create token
    struct Token token;
    token.type = TOK_IDENTIFIER;

    // Only actually intern and return the string if the lexer is not peeking in a look-ahead operation
    if (!lexer->lexerPeeking)
    {
        token.value = InternString(lexer->string_pool, start, end - start);
    }
    else
    {
        token.value = (struct StringView){0};
    }

    // Set the start pointer of the token to the beginning of the selector blob
    token.start = (char *)start;

    return token;
}

struct Token GetAtRuleParams(struct Lexer *lexer)
{
    // Reset expects at-rule parameters flag
    lexer->expectsAtRuleParams = false;

    struct Token token;
    token.type = TOK_PARAMS;

    const char *start = lexer->cursor;

    // Skip leading whitespace
    while (!LexerIsAtEnd(lexer) && isspace((unsigned char)LexerPeek(lexer)))
    {
        LexerAdvance(lexer);
    }

    // Store whether cursor is in a css string, avoid reading a '{' or ';' token from a string
    bool inQuotes = false;
    char quoteChar = '\0';

    // Read until the end of the at-rule parameters
    while (!LexerIsAtEnd(lexer))
    {
        char c = LexerPeek(lexer);

        // Handle quote state
        if (c == '"' || c == '\'')
        {
            if (!inQuotes)
            {
                inQuotes = true;
                quoteChar = c;
            }
            else if (c == quoteChar)
            {
                inQuotes = false;
            }
        }

        // Check for the end of the at-rule parameters
        if (!inQuotes && (c == '{' || c == ';'))
        {
            break;
        }

        LexerAdvance(lexer);
    }

    const char *end = lexer->cursor;

    // Trim trailing whitespace
    while (end > start && isspace((unsigned char)*(end - 1)))
    {
        end--;
    }

    // Only return and intern the string if the lexer is not peeking in a look-ahead operation
    if (!lexer->lexerPeeking)
    {
        token.value = InternString(lexer->string_pool, start, end - start);
    }
    else
    {
        token.value = (struct StringView){0};
    }

    // Set the start pointer of the token to the beginning of the at-rule parameters
    token.start = (char *)start;

    return token;
}

struct Token LexerNextToken(struct Lexer *lexer)
{
    LexerSkipWhitespaceAndComments(lexer);

    if (lexer->expectsValue)
    {
        return GetValueBlob(lexer);
    }

    if (lexer->expectsSelector)
    {
        return GetRulesetSelector(lexer);
    }

    if (lexer->expectsAtRuleParams)
    {
        return GetAtRuleParams(lexer);
    }

    // Create a default EOF token to return the end of input is reached
    struct Token token = {.type = TOK_EOF, .value = {NULL}, .start = lexer->cursor};

    if (LexerIsAtEnd(lexer))
    {
        return token;
    }

    const char *start = lexer->cursor;
    char c = LexerPeek(lexer);

    // Specific handling for finding rule parameters
    /*if (lexer->state == LEXER_STATE_AT_RULE_PARAMS)
    {
        // If the current char is a { or ; then the parameters are over
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
    }*/

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
