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

    // Tracks the current file position in the generated CSS file (for source map generation)
    struct FilePosition genPosition;
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

    if (generator->config != NULL && generator->config->generateSourceMap)
    {
        // Update the current file position for source map generation
        if (c == '\n')
        {
            generator->genPosition.column = 0;
        }
        else
        {
            generator->genPosition.column++;
        }
    }
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

// Base64 alphabet for source map generation
static const char BASE64_ALPHABET[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static void GeneratorAppendVLQ(struct CSSGenerator *generator, int value)
{
    // If value is positive, shift left by 1
    // If value is negative, shift left by 1 and set the least significant bit to 1
    unsigned int vlq = (value < 0) ? ((-value) << 1) + 1 : (value << 1);

    do
    {
        // Get the least significant 5 bits using a bitwise AND with 0x1F which is 00011111
        unsigned int digit = vlq & 0x1F;

        // Shift right by 5 bits for the next iteration
        vlq >>= 5;

        if (vlq > 0)
        {
            // If there are more digits, set the continuation bit (6th bit) using bitwise OR with 0x20 which is 00100000
            digit |= 0x20;
        }

        // Append the corresponding base64 character to the source map buffer
        GeneratorAppendChar(generator, BASE64_ALPHABET[digit]);

    } while (vlq > 0);
}

// Struct to track the state of the source map generation
struct SourceMapState
{
    int previousGeneratedColumn;
    int previousSourceIndex;
    int previousSourceLine;
    int previousSourceColumn;
};

// Appends a mapping to the source map generator buffer
void AppendMapping(struct CSSGenerator *generator, struct SourceMapState *state, int generatedColumn, int sourceIndex, int sourceLine, int sourceColumn)
{
    // If source maps are not being generated, generator will be NULL, so we can return immediately (also check state for safety)
    if (generator == NULL || state == NULL)
    {
        return;
    }

    // Generated column (relative to previous generated column)
    GeneratorAppendVLQ(generator, generatedColumn - state->previousGeneratedColumn);
    state->previousGeneratedColumn = generatedColumn;

    // Source index (relative to previous source index)
    GeneratorAppendVLQ(generator, sourceIndex - state->previousSourceIndex);
    state->previousSourceIndex = sourceIndex;

    // Source line (relative to previous source line)
    GeneratorAppendVLQ(generator, sourceLine - state->previousSourceLine);
    state->previousSourceLine = sourceLine;

    // Source column (relative to previous source column)
    GeneratorAppendVLQ(generator, sourceColumn - state->previousSourceColumn);
    state->previousSourceColumn = sourceColumn;

    // Separate mappings with a comma
    GeneratorAppendChar(generator, ',');
}

// Recursive function to generate CSS from the AST nodes
void GenerateCSS(struct CSSGenerator *generator, struct CSSGenerator *sourceMapGenerator, struct ASTNode *node, struct SourceMapState *sourceMapState)
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
            GenerateCSS(generator, sourceMapGenerator, current->data.stylesheet.children, sourceMapState);
            break;
        case CSS_NODE_RULESET:

            // Append node's mapping to the source map
            AppendMapping(sourceMapGenerator, sourceMapState, generator->genPosition.column, 0, current->position.column, current->position.line);

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
            GenerateCSS(generator, sourceMapGenerator, current->data.ruleset.declarations, sourceMapState);
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

            AppendMapping(sourceMapGenerator, sourceMapState, generator->genPosition.column, 0, current->position.column, current->position.line);

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

            AppendMapping(sourceMapGenerator, sourceMapState, generator->genPosition.column, 0, current->position.column, current->position.line);

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
                GenerateCSS(generator, sourceMapGenerator, current->data.at_rule.block, sourceMapState);
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

struct ArrowCSSBuildResult *ArrowCSSBuildResultCreate(char *css, char *sourceMap)
{
    struct ArrowCSSBuildResult *result = malloc(sizeof(struct ArrowCSSBuildResult));
    result->css = css;
    result->sourceMap = sourceMap;
    return result;
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
struct ArrowCSSBuildResult *ArrowCSS_GenerateCSSFromAST(struct CSSAST *ast, struct CSSGeneratorConfig *config)
{
    if (ast == NULL)
    {
        return NULL;
    }

    struct CSSGenerator generator;

    CSSGeneratorInit(&generator, config);

    struct CSSGenerator sourceMapGenerator;

    struct SourceMapState sourceMapState = {0};

    // If source map generation is enabled, initialise the source map generator
    if (config->generateSourceMap)
    {
        CSSGeneratorInit(&sourceMapGenerator, NULL);
        // Start the JSON
        GeneratorAppend(&sourceMapGenerator, "{\"version\":3,\"mappings\":\"", 25);
    }

    // Recursive CSS generate function
    GenerateCSS(&generator, config->generateSourceMap ? &sourceMapGenerator : NULL, ast->root, &sourceMapState);

    char *cssOutput = CSSGeneratorFinish(&generator);
    char *sourceMapOutput = NULL;

    // If source map generation enabled, finish the source map
    if (config->generateSourceMap)
    {
        // Close the source map JSON object
        GeneratorAppend(&sourceMapGenerator, "\"}", 2);
        sourceMapOutput = CSSGeneratorFinish(&sourceMapGenerator);
    }

    return ArrowCSSBuildResultCreate(cssOutput, sourceMapOutput);
}