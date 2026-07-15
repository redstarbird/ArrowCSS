#include "Generator.h"

// Struct representing the CSS generator
struct CSSGenerator
{
    char *buffer;
    size_t length;
    size_t capacity;

    // Configuration options for the generator

    // Whether to minify the generated CSS
    bool minify;

    // The level of indentation for the generated CSS
    unsigned int indentLevel;

    // Whether to use tabs for indentation instead of spaces
    bool useTabs;
};

// Append a string to the generator's buffer, resizing if necessary
static void GeneratorAppend(struct CSSGenerator *generator, const char *str, size_t len)
{
    if (len == 0 || str == NULL)
    {
        return;
    }

    // Ensure there is enough capacity in the buffer
    if (generator->length + len >= generator->capacity)
    {
        generator->capacity = generator->length * 2 + len;
        generator->buffer = realloc(generator->buffer, generator->capacity);
    }

    // Append the string to the buffer
    memcpy(generator->buffer + generator->length, str, len);
    generator->length += len;
}

// Append a single character to the generator's buffer
static void GeneratorAppendChar(struct CSSGenerator *generator, char c)
{
    GeneratorAppend(generator, &c, 1);
}

// Append a string view to the generator's buffer
static void GeneratorAppendStringView(struct CSSGenerator *generator, struct StringView *view)
{
    GeneratorAppend(generator, view->data, view->length);
}

// Recursive function to generate CSS from the AST nodes
void GenerateCSS(struct CSSGenerator *generator, struct ASTNode *node)
{
    if (node == NULL || generator == NULL)
    {
        return;
    }

    struct ASTNode *current = node;
    while (current != NULL)
    {
        switch (current->type)
        {
        case CSS_NODE_STYLESHEET:
            GenerateCSS(generator, current->data.stylesheet.children);
            break;

        case CSS_NODE_RULESET:
            // Append selector, opening brace, recursively append declarations, and closing brace
            GeneratorAppendStringView(generator, &current->data.ruleset.selectors);
            GeneratorAppendChar(generator, '{');
            GenerateCSS(generator, current->data.ruleset.declarations);
            GeneratorAppendChar(generator, '}');
            break;

        case CSS_NODE_DECLARATION:
            // Add property value pair to the buffer
            GeneratorAppendStringView(generator, &current->data.decl.property);
            GeneratorAppendChar(generator, ':');
            GeneratorAppendStringView(generator, &current->data.decl.value);

            // If the declaration is marked as !important, append it
            if (current->data.decl.important)
            {
                GeneratorAppend(generator, " !important", 11);
            }

            GeneratorAppendChar(generator, ';');
            break;

        // Handle at-rule nodes
        case CSS_NODE_AT_RULE:

            // Ensure the at-rule name starts with '@'
            if (current->data.at_rule.name.length > 0 && current->data.at_rule.name.data[0] != '@')
            {
                GeneratorAppendChar(generator, '@');
            }

            GeneratorAppendStringView(generator, &current->data.at_rule.name);
            GeneratorAppendChar(generator, ' ');
            GeneratorAppendStringView(generator, &current->data.at_rule.params);

            // If the at-rule has a block, generate its contents
            if (current->data.at_rule.block != NULL)
            {
                GeneratorAppendChar(generator, '{');
                GenerateCSS(generator, current->data.at_rule.block);
                GeneratorAppendChar(generator, '}');
            }
            else
            {
                // If there's no block, end the at-rule with a semicolon
                GeneratorAppendChar(generator, ';');
            }
            break;

        default:
            // Report an error for unknown node types
            fprintf(stderr, "Error: Unknown AST node type encountered during CSS generation.\n");
            break;
        }

        current = current->next;
    }
}

char *CSSGeneratorFinish(struct CSSGenerator *generator)
{
    // Add the final null terminator
    GeneratorAppendChar(generator, '\0');

    return generator->buffer;
}

void CSSGeneratorInit(struct CSSGenerator *generator, struct CSSGeneratorConfig *config)
{
    // Initial buffer size of 2KB
    generator->buffer = malloc(2048);
    generator->length = 0;
    generator->capacity = 2048;
    generator->minify = config->minify;
    generator->indentLevel = config->indentLevel;
}

// Public function for generating CSS from the AST
char *ArrowCSS_GenerateCSSFromAST(struct CSSAST *ast, struct CSSGeneratorConfig *config)
{
    if (ast == NULL)
    {
        return NULL;
    }

    struct CSSGenerator generator;

    CSSGeneratorInit(&generator, config);

    GenerateCSS(&generator, ast->root);

    return CSSGeneratorFinish(&generator);
}