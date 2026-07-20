#include "CSSAST.h"

void ArrowCSS_DestroyAST(struct CSSAST *ast)
{
    if (ast->ownsStringPool)
    {
        DestroyStringPool(ast->stringPool);
    }

    if (ast->ownsArena)
    {
        DestroyArena(ast->arena);
    }
}