#ifndef ARROWCSS
#define ARROWCSS

#include "../src/CSSAST.h"
#include "../src/CSSParser.h"

typedef struct ArrowAllocator
{
    void *(*alloc)(size_t size, void *user_data);
    void (*free)(void *ptr, void *user_data);
} ArrowAllocator;

#endif