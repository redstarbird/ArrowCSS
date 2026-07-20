#ifndef ARROWCSS
#define ARROWCSS

#include "../src/CSSAST.h"
#include "../src/Generator.h"

typedef struct ArrowCssParseOptions
{
    // For error reporting
    const char *filename;
    // Should source maps be generated
    bool generateSourceMap;
    // Supress any console warnings
    bool silent;

    // Memory arena
    struct MemoryArena *arena;
    // String pool
    struct StringPool *pool;

} ArrowCssParseOptions;

// Contains the generated CSS and source map (if requested)
typedef struct ArrowCSSBuildResult
{
    // The generated CSS string
    char *css;
    // The generated source map string (if requested)
    char *sourceMap;
} ArrowCSSBuildResult;

struct CSSAST *ParseCSSToAST(char *fileContent, size_t length, struct ArrowCssParseOptions *options);

/// @brief Frees the memory of the provided AST, this can only be done for the string pool and/or memory arena if they are not provided by the user.
/// @param ast The ast to be destroyed
void ArrowCSS_DestroyAST(struct CSSAST *ast);

#endif