#include "Generator.h"

// Struct representing the CSS generator
struct CSSGenerator
{
    char *buffer;
    size_t length;
    size_t capacity;

    // Tracks the current indentation depth for pretty-printing
    int currentIndentDepth;

    // User provided configuration for the generator
    struct CSSGeneratorConfig *config;
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

// Append indentation based on the current depth and configuration
static void GeneratorAppendIndentation(struct CSSGenerator *generator)
{
    // If ident level is 0 consider it as minified in regards to indentation
    if (generator->config->minify || generator->config->indentLevel == 0)
    {
        return;
    }

    // Append indentation
    for (int i = 0; i < generator->currentIndentDepth; i++)
    {
        // Add number of tabs based on indent depth
        if (generator->config->useTabs)
        {
            GeneratorAppendChar(generator, '\t');
        }
        // Or add spaces based on indent level and indent depth
        else
        {
            for (unsigned int j = 0; j < generator->config->indentLevel; j++)
            {

                GeneratorAppendChar(generator, ' ');
            }
        }
    }
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

            // Append indentation for the ruleset, needed for nested rulesets
            GeneratorAppendIndentation(generator);

            GeneratorAppendStringView(generator, &current->data.ruleset.selectors);

            if (generator->config->minify)
            {
                GeneratorAppendChar(generator, '{');
            }
            else
            {
                GeneratorAppend(generator, " {\n", 3);
            }

            generator->currentIndentDepth++;
            GenerateCSS(generator, current->data.ruleset.declarations);
            generator->currentIndentDepth--;

            GeneratorAppendIndentation(generator);

            GeneratorAppendChar(generator, '}');

            // Add line break after closing brace if not minifying
            if (!generator->config->minify)
            {
                GeneratorAppendChar(generator, '\n');

                // Add extra line break between rulesets for better readability
                if (current->next != NULL)
                {
                    GeneratorAppendChar(generator, '\n');
                }
            }

            break;

        case CSS_NODE_DECLARATION:
            // Add property value pair to the buffer

            GeneratorAppendIndentation(generator);

            GeneratorAppendStringView(generator, &current->data.decl.property);
            GeneratorAppendChar(generator, ':');

            // Add a space after the colon if not minifying
            if (!generator->config->minify)
            {
                GeneratorAppendChar(generator, ' ');
            }

            GeneratorAppendStringView(generator, &current->data.decl.value);

            // If the declaration is marked as !important, append it
            if (current->data.decl.important)
            {
                GeneratorAppend(generator, " !important", 11);
            }

            GeneratorAppendChar(generator, ';');

            // Add line break after declaration if not minifying
            if (!generator->config->minify)
            {
                GeneratorAppendChar(generator, '\n');
            }
            break;

        // Handle at-rule nodes
        case CSS_NODE_AT_RULE:

            // Apply indentation for the name and parameters of the at-rule
            GeneratorAppendIndentation(generator);

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
                if (generator->config->minify)
                {
                    GeneratorAppendChar(generator, '{');
                }
                else
                {
                    GeneratorAppend(generator, " {\n", 3);
                }

                // Apply indentation for block and recursively generate its contents
                generator->currentIndentDepth++;
                GenerateCSS(generator, current->data.at_rule.block);
                generator->currentIndentDepth--;

                // End brace for the at-rule block with proper indentation
                GeneratorAppendIndentation(generator);
                GeneratorAppendChar(generator, '}');

                // Extra line break after closing brace if not minifying
                if (!generator->config->minify)
                {
                    GeneratorAppendChar(generator, '\n');

                    // Extra line break between at-rules for better readability
                    if (current->next != NULL)
                    {
                        GeneratorAppendChar(generator, '\n');
                    }
                }
            }
            else
            {
                // If there's no block, end the at-rule with a semicolon
                GeneratorAppendChar(generator, ';');

                // Extra line break after semicolon if not minifying
                if (!generator->config->minify)
                {
                    GeneratorAppendChar(generator, '\n');
                }
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
    generator->config = config;
    generator->currentIndentDepth = 0;
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