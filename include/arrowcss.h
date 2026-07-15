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

struct CSSAST *ParseCSSToAST(char *fileContent, size_t length, struct ArrowCssParseOptions *options);

#endif