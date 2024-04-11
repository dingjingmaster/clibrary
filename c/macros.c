//
// Created by dingjing on 24-4-9.
//

#include "macros.h"

void* c_malloc0 (cuint64 size)
{
    void* ptr = NULL;

    c_malloc(ptr, size);

    return ptr;
}
