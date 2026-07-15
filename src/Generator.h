#include <stdlib.h>
#include <stdbool.h>
#include "CSSAST.h"
#include "StringPool.h"

// Configuration for the CSS generator
struct CSSGeneratorConfig
{
    // Whether to minify the generated CSS
    bool minify;
    // The level of indentation for the generated CSS
    int indentLevel;
    // Whether to use tabs for indentation instead of spaces
    bool useTabs;
};

/** Generates CSS from a CSS AST based on the provided configuration. */
char *ArrowCSS_GenerateCSSFromAST(struct CSSAST *ast, struct CSSGeneratorConfig *config);