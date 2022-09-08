/*************************************************************************
> FileName: test_macros.c
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Thu 08 Sep 2022 09:20:21 AM CST
 ************************************************************************/
#include "../core/macros.h"
#include <stdio.h>

int main (C_UNUSED int argc, char* argv[])
{
    printf ("%s %s -- %s\n", C_LINE, C_STRLOC, C_STRFUNC);

    return 0;
}
