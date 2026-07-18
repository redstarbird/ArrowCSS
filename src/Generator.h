#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdlib.h>
#include <stdbool.h>
#include "CSSAST.h"
#include "StringPool.h"
#include "../include/arrowcss.h"

// Configuration for the CSS generator
struct CSSGeneratorConfig
{
    // Whether to minify the generated CSS
    bool minify;
    // The number of spaces to use for indentation in the generated CSS
    unsigned int indentLevel;
    // Whether to use tabs for indentation instead of spaces
    bool useTabs;

    // Whether to generate source maps for the generated CSS
    bool generateSourceMap;
};

/** Generates CSS from a CSS AST based on the provided configuration. */
struct ArrowCSSBuildResult *ArrowCSS_GenerateCSSFromAST(struct CSSAST *ast, struct CSSGeneratorConfig *config);

#endif