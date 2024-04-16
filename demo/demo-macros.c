
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
