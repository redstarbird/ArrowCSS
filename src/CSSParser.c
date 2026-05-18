#include "CSSParser.h"

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

// Parses a declaration. This should have a sequence of Identifier -> Colon -> Identifier -> Semicolon
static struct ASTNode *ParseDeclaration(struct Parser *parser)
{
    struct ASTNode *node = PushStruct(parser->arena, struct ASTNode);
    node->type = CSS_NODE_DECLARATION;
    node->next = NULL;

    // Expect the property name (e.g, "color")
    consume(parser, TOK_IDENTIFIER, "Expected property name.");
    node->data.decl.property = parser->prevToken.value;

    // Expect the colon seperating the property and value
    consume(parser, TOK_COLON, "Expected : after property name.");

    // Expect the property value (e.g, "green")
    consume(parser, TOK_IDENTIFIER, "Expected property value.");
    node->data.decl.value = parser->prevToken.value;

    // Expect the semicolon at the end of the declaration
    consume(parser, TOK_SEMICOLON, "Expected : after property value");

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
        struct ASTNode *decl = ParseDeclaration(parser);

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
            // Parse at-rule (NOT YET IMPLEMENTED!)
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

struct CSSAST *ParseCSSToAST(char *fileContent, size_t length, struct MemoryArena *arena, struct StringPool *pool)
{
    if (arena == NULL)
    {
        arena = CreateArena(length, OOM_GROW_ARENA, NULL);
    }
    if (pool == NULL)
    {
        pool = CreateStringPool(2 << 19, length, OOM_GROW_ARENA, NULL);
    }

    struct Lexer lexer;
    LexerInit(&lexer, fileContent, length, pool);

    struct Parser parser;
    ParserInit(&parser, &lexer, arena);

    struct CSSAST *ast = ParseStylesheet(&parser);

    return ast;
}