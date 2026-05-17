/** @file CSSAST.h
 * @brief Header file defining the structures for the CSS Abstract Syntax Tree (AST)
 */
#ifndef CSSAST_H
#define CSSAST_H

#include <stddef.h>
#include <stdbool.h>
#include <MemoryArena.h>
#include <StringPool.h>

/** @brief Enumeration of token types in the CSS parser */
typedef enum
{
    TOK_IDENTIFIER, // .btn, color, red
    TOK_LBRACE,     // {
    TOK_RBRACE,     // }
    TOK_COLON,      // :
    TOK_SEMICOLON,  // ;
    TOK_AT_RULE,    // @media, @keyframes
    TOK_EOF
} TokenType;

/** @brief Structure representing a token in the CSS parser */
typedef struct Token
{
    // Token type
    TokenType type;
    // String view of the token (e.g., "color", "red")
    struct StringView value;
} Token;

/** @brief The Semantic types of AST Nodes */
typedef enum CssNodeType
{
    // The root of the AST, representing the entire stylesheet
    CSS_NODE_STYLESHEET,
    // A ruleset, representing a selector and its declarations (e.g., .btn)
    CSS_NODE_RULESET,
    // A declaration, representing a property-value pair (e.g., color: red)
    CSS_NODE_DECLARATION, // color: red;
                          // An at-rule, representing constructs like @media or @keyframes
    CSS_NODE_AT_RULE
} CssNodeType;

/** @brief Structure representing a node in the CSS Abstract Syntax Tree (AST) */
struct ASTNode
{
    /** @brief Read-only: Type of the AST node (e.g., CSS_NODE_RULESET) */
    enum CssNodeType type;
    /** @brief Pointer to the next sibling node (For linked list of rules/declarations) */
    struct ASTNode *next;

    /** @brief Union holding node-specific data based on the node type */
    union
    {
        // Active if type == CSS_NODE_STYLESHEET
        struct
        {
            // Linked list of child nodes (rulesets, at-rules)
            struct ASTNode *children;
        } stylesheet;

        // Active if type == CSS_NODE_RULESET
        struct
        {
            // StringView for the selector (e.g., ".btn", "h1, h2")
            const struct StringView *selectors;
            // Linked list of CSS_NODE_DECLARATION
            struct ASTNode *declarations;
        } ruleset;

        // Active if type == CSS_NODE_DECLARATION
        struct
        {
            // StringView for the property name (e.g., "color")
            const struct StringView *property;
            // StringView for the property value (e.g., "red", "16px")
            const struct StringView *value;
            // Flag indicating if the declaration is marked as !important
            bool important;
        } decl;

        // Active if type == CSS_NODE_AT_RULE
        struct
        {
            // StringView for the at-rule name (e.g., "media", "keyframes")
            const struct StringView name;
            // StringView for the at-rule parameters (e.g., "(max-width: 600px)")
            const struct StringView *params;
            // Linked list of child nodes (e.g., rulesets inside @media)
            struct ASTNode *block;
        } at_rule;
    } data;
};

/** @brief Structure representing the entire CSS Abstract Syntax Tree (AST) */
typedef struct
{
    /** @brief Internal, read-only: String pool for managing string literals */
    struct StringPool *stringPool;
    /** @brief Internal, read-only: Memory arena for allocating AST nodes */
    struct MemoryArena *arena;
    /** @brief Read-only: Root node of the AST */
    struct ASTNode *root;
} CSSAST;

#endif // CSSAST_H