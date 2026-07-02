#include "CSSParser.h"

// Forward declaration of ParseAtRule so it can be recursively called by ParseRuleset
struct ASTNode *ParseAtRule(struct Parser *parser);

// Saves lexer state
void LexerSaveState(struct Lexer *lexer, struct Lexer *saveTo)
{
    // Struct data copy
    *saveTo = *lexer;
}

// Restores lexer state
void LexerRestoreState(struct Lexer *lexer, struct Lexer *restoreFrom)
{
    // Struct data copy
    *lexer = *restoreFrom;
}

// Move to the next function
static void advance(struct Parser *parser)
{
    parser->prevToken = parser->currentToken;
    parser->currentToken = LexerNextToken(parser->lexer);
}

// Checks if the current token is a specific type, if EOF then false is always returned
static bool check(Parser *parser, TokenType type)
{

    return parser->currentToken.type == type;
}

#include <stdlib.h>

static bool consume(Parser *parser, TokenType type, const char *error_message)
{
    printf("\n\n\n\ncursor: %s\n", parser->lexer->cursor);
    printf("token: %.*s\n\n", (int)parser->currentToken.value.length, parser->currentToken.value.data);
    // Check the current token is the expected token before consuming it
    if (check(parser, type))
    {
        advance(parser);
        return true;
    }

    // If not the expected token then the CSS is malformed
    printf("Syntax Error: %s\n", error_message);
    parser->ErrorDiscovered = true;

    // exit(1);

    // Forcibly advance the lexer forward to prevent an infinite loop in the case of a syntax error
    if (parser->currentToken.type != TOK_EOF)
    {
        advance(parser);
    }

    return false;
}

static bool isNextConstructADeclaration(struct Parser *parser)
{

    // Save the current lexer state
    struct Lexer savedLexer;
    LexerSaveState(parser->lexer, &savedLexer);

    // Set the lexer to be peeking so extra tokens are not consumed here to reduce unecessary CPU cycles
    parser->lexer->lexerPeeking = true;

    // Check if the next construct is a declaration
    bool isDeclaration = false;
    bool colonFound = false;

    while (true)
    {
        struct Token token = LexerNextToken(parser->lexer);

        // If we hit the end of the file, then there is no declaration
        if (token.type == TOK_EOF)
        {
            isDeclaration = false;
            break;
        }

        // If a colon is found, the next construct could be a ruleset or a declaration so keep looking
        if (token.type == TOK_COLON)
        {
            colonFound = true;
        }

        // If we find a left brace, then the next construct is a ruleset
        // e.g, ".btn { color: red; }"
        if (token.type == TOK_LBRACE)
        {
            isDeclaration = false;
            break;
        }

        // If we find a semicolon first, then the next construct is a declaration
        // e.g, "color: red;"
        if (token.type == TOK_SEMICOLON)
        {
            isDeclaration = true;
            break;
        }

        if (token.type == TOK_RBRACE)
        {
            // If we find a '{' before a ':' then the construct is a ruleset otherwise it is a declaration without a semicolon
            // e.g, ".btn { color: red; }" vs "color: red" at the end of a ruleset block
            isDeclaration = colonFound;
            break;
        }
    }

    // Restore the lexer state
    LexerRestoreState(parser->lexer, &savedLexer);

    return isDeclaration;
}

// Parses a declaration. This should have a sequence of Identifier -> Colon -> Identifier -> Semicolon
static struct ASTNode *ParseDeclaration(struct Parser *parser)
{
    struct ASTNode *node = PushStruct(parser->arena, struct ASTNode);
    node->type = CSS_NODE_DECLARATION;
    node->next = NULL;

    // Expect the property name (e.g, "color")
    consume(parser, TOK_IDENTIFIER, "Expected property name.");
    node->data.decl.property = parser->prevToken.value;

    // Tell the lexer to store the next token as a blob including whitespace
    parser->lexer->expectsValue = true;

    // Expect the colon seperating the property and value
    consume(parser, TOK_COLON, "Expected : after property name.");

    // advance(parser);

    // Expect the property value (e.g, "green")
    consume(parser, TOK_IDENTIFIER, "Expected property value.");
    node->data.decl.value = parser->prevToken.value;

    // Expect the semicolon at the end of the declaration, or a '}'
    if (check(parser, TOK_SEMICOLON))
    {
        consume(parser, TOK_SEMICOLON, "Expected : after property value");
    }
    else if (!check(parser, TOK_LBRACE) && !check(parser, TOK_EOF))
    {
        printf("Syntax Error: Expected ';' after property value\n");

        parser->ErrorDiscovered = true;

        if (parser->currentToken.type != TOK_EOF)
        {
            advance(parser);
        }
    }

    return node;
}

static struct ASTNode *ParseRuleset(struct Parser *parser)
{
    struct ASTNode *node = PushStruct(parser->arena, struct ASTNode);
    node->type = CSS_NODE_RULESET;
    node->next = NULL;

    // Find the CSS selector
    consume(parser, TOK_IDENTIFIER, "Expected CSS selector.");
    node->data.ruleset.selectors = parser->prevToken.value;

    // Open the block containing the declarations
    consume(parser, TOK_LBRACE, "Expected '{' before ruleset block.");

    // Parse declarations until a } character is hit

    struct ASTNode *firstDecl = NULL;
    struct ASTNode *currentDecl = NULL;

    while (!check(parser, TOK_RBRACE) && !check(parser, TOK_EOF))
    {
        struct ASTNode *decl = NULL;

        // Parse a declaration inside the ruleset, e.g., "color: red;". This is the most common case!!
        if (isNextConstructADeclaration(parser))
        {
            decl = ParseDeclaration(parser);
        }
        // Parse an at-rule inside the ruleset, e.g., @media or @keyframes
        else if (check(parser, TOK_AT_RULE))
        {
            decl = ParseAtRule(parser);
        }
        // Parse a nested ruleset inside the ruleset, e.g., ".btn { color: red; }"
        else
        {
            decl = ParseRuleset(parser);
        }

        // Add the node to the linked list of declarations inside the ruleset
        if (firstDecl == NULL)
        {
            firstDecl = decl;
            currentDecl = decl;
        }
        else
        {
            currentDecl->next = decl;
            currentDecl = decl;
        }
    }
    node->data.ruleset.declarations = firstDecl;

    // Close the block
    consume(parser, TOK_RBRACE, "Expected } after ruleset block.");

    return node;
}

struct ASTNode *ParseAtRule(struct Parser *parser)
{
    struct ASTNode *node = PushStruct(parser->arena, struct ASTNode);
    node->type = CSS_NODE_AT_RULE;
    node->next = NULL;

    // Consume the name of the at-rule (e.g, @media or @import)
    consume(parser, TOK_AT_RULE, "Expected at-rule name");
    node->data.at_rule.name = parser->prevToken.value;

    // Consume the at-rule parameters if they exist
    if (check(parser, TOK_PARAMS))
    {
        consume(parser, TOK_PARAMS, "Expected at-rule parameters");
        node->data.at_rule.params = parser->prevToken.value;
    }
    else
    {
        node->data.at_rule.params = (struct StringView){0};
    }

    // If the next token is a semicolon, the at-rule is a statement such as "@import url(...);"
    if (check(parser, TOK_SEMICOLON))
    {
        consume(parser, TOK_SEMICOLON, "Expected ';' after at-rule statement");
        // Simple statement has no children
        node->data.at_rule.block = NULL;
    }
    // If next token is a '{' then the at-rule contains a block
    else if (check(parser, TOK_LBRACE))
    {
        struct ASTNode *firstNested = NULL;
        struct ASTNode *currentNested = NULL;

        // Keep parsing all of the rulesets/at-rules in the block until there are none left
        while (!check(parser, TOK_RBRACE) && !check(parser, TOK_EOF))
        {

            struct ASTNode *nestedNode = NULL;

            // Decide what to parse based on the whether the next construct is an at-rule, a ruleset or a declaration

            // Handle nested at-rules, e.g., @media inside @media
            if (check(parser, TOK_AT_RULE))
            {
                nestedNode = ParseAtRule(parser);
            }
            // Handle nested declarations, e.g., "color: red;" inside @media
            else if (isNextConstructADeclaration(parser))
            {
                nestedNode = ParseDeclaration(parser);
            }
            // Handle nested rulesets, e.g., ".btn { color: red; }" inside @media
            else
            {
                nestedNode = ParseRuleset(parser);
            }

            if (firstNested == NULL)
            {
                firstNested = nestedNode;
                currentNested = nestedNode;
            }
            else
            {
                currentNested->next = nestedNode;
                currentNested = nestedNode;
            }
        }
        node->data.at_rule.block = firstNested;

        // Ensure at-rule ends with '{' for correct syntax
        consume(parser, TOK_RBRACE, "Expected '}' after at-rule block");
    }
    // If there is no ';' or '{' after the at-rule, it is a syntax error
    else
    {
        printf("Syntax error: Expected '{' or ';' after at-rule statement\n");
        parser->ErrorDiscovered = true;
    }

    return node;
}

struct CSSAST *ParseStylesheet(struct Parser *parser)
{
    // Get the first token after the stylesheet
    advance(parser);

    struct CSSAST *ast = PushStruct(parser->arena, struct CSSAST);
    ast->arena = parser->arena;
    ast->stringPool = parser->lexer->string_pool;

    struct ASTNode *root = PushStruct(parser->arena, struct ASTNode);
    root->type = CSS_NODE_STYLESHEET;

    struct ASTNode *firstRule = NULL;
    struct ASTNode *currentRule = NULL;

    // Loop until the end of the file to find all of the rulesets
    while (!check(parser, TOK_EOF))
    {
        struct ASTNode *rule = NULL;

        // Decide what to parse based on the current token
        if (check(parser, TOK_AT_RULE))
        {
            rule = ParseAtRule(parser);
        }
        else
        {
            rule = ParseRuleset(parser);
        }

        // Add ruleset to the linked list
        if (firstRule == NULL)
        {
            firstRule = rule;
            currentRule = rule;
        }
        else
        {
            currentRule->next = rule;
            currentRule = rule;
        }
    }
    root->data.stylesheet.children = firstRule;
    ast->root = root;

    return ast;
}

void ParserInit(struct Parser *parser, struct Lexer *lexer, struct MemoryArena *arena)
{
    parser->ErrorDiscovered = false;
    parser->lexer = lexer;
    parser->arena = arena;
}

struct CSSAST *ParseCSSToAST(char *fileContent, size_t length, struct ArrowCssParseOptions *options)
{
    struct MemoryArena *arena = NULL;
    struct StringPool *pool = NULL;

    if (options != NULL)
    {
        arena = options->arena;
        pool = options->pool;
    }

    // If no arena was provided, create one
    if (arena == NULL)
    {
        arena = CreateArena(length, OOM_GROW_ARENA, NULL);
    }
    // If no arena was provided, create one
    if (pool == NULL)
    {
        pool = CreateStringPool(2 << 14, length + sizeof(struct StringView) * (2 << 14), OOM_GROW_ARENA, NULL);
    }

    struct Lexer lexer;
    LexerInit(&lexer, fileContent, length, pool);

    struct Parser parser;
    ParserInit(&parser, &lexer, arena);

    struct CSSAST *ast = ParseStylesheet(&parser);

    ast->ownsArena = (options == NULL || options->arena == NULL);
    ast->ownsStringPool = (options == NULL || options->pool == NULL);

    return ast;
}