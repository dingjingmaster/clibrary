/*************************************************************************
> FileName: test_macros.c
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Thu 08 Sep 2022 09:20:21 AM CST
 ************************************************************************/
#include "../c/log.h"
#include "../c/macros.h"
#include <stdio.h>

int main (C_UNUSED int argc, C_UNUSED char* argv[])
{
    printf ("%s %s -- %s\n", C_LINE, C_STRLOC, C_STRFUNC);

    C_DEBUG_INFO(1131313131313131"asdadas"aaaa);

    char* ptr = NULL;
    c_malloc(ptr, 10);
    c_free(ptr);

    c_malloc_type(ptr, char, 10);
//    c_free(ptr);

    c_free_with_func (ptr, free);
    c_free_with_func (ptr, free);

    return 0;
}
