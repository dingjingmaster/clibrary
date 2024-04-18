
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*************************************************************************
> FileName: atomic.h
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Thu 06 Sep 2022 21:29:27 AM CST
 ************************************************************************/
#ifndef _ATOMIC_H
#define _ATOMIC_H

#if !defined (__CLIB_H_INSIDE__) && !defined (CLIB_COMPILATION)
#error "Only <clib.h> can be included directly."
#endif
#include <c/macros.h>


C_BEGIN_EXTERN_C


int     c_atomic_int_get                        (const volatile int *atomic);
void    c_atomic_int_set                        (volatile int* atomic, int newval);
void    c_atomic_int_inc                        (volatile int* atomic);
bool    c_atomic_int_dec_and_test               (volatile int* atomic);
bool    c_atomic_int_compare_and_exchange       (volatile int* atomic, int oldval, int newval);
bool    c_atomic_int_compare_and_exchange_full  (int* atomic, int oldval, int newval, int* preval);
int     c_atomic_int_exchange                   (int* atomic, int newval);
int     c_atomic_int_add                        (volatile int* atomic, int val);
cuint   c_atomic_int_and                        (volatile cuint* atomic, cuint val);
cuint   c_atomic_int_or                         (volatile cuint* atomic, cuint val);
cuint   c_atomic_int_xor                        (volatile cuint *atomic, cuint val);
void*   c_atomic_pointer_get                    (const volatile void* atomic);
void    c_atomic_pointer_set                    (volatile void* atomic, void* newval);
bool    c_atomic_pointer_compare_and_exchange   (volatile void* atomic, void* oldval, void* newval);
bool    c_atomic_pointer_compare_and_exchange_full (void* atomic, void* oldval, void* newval, void* preval);
void*   c_atomic_pointer_exchange               (void* atomic, void* newval);
culong  c_atomic_pointer_add                    (volatile void* atomic, culong val);
culong  c_atomic_pointer_and                    (volatile void* atomic, culong val);
culong  c_atomic_pointer_or                     (volatile void* atomic, culong val);
culong  c_atomic_pointer_xor                    (volatile void* atomic, culong val);
int     c_atomic_int_exchange_and_add           (volatile int *atomic, int val);

void    c_ref_count_init                        (crefcount* rc);
void    c_ref_count_inc                         (crefcount* rc);
bool    c_ref_count_dec                         (crefcount* rc);
bool    c_ref_count_compare                     (crefcount* rc, cint val);
void    c_atomic_ref_count_init                 (catomicrefcount* arc);
void    c_atomic_ref_count_inc                  (catomicrefcount* arc);
bool    c_atomic_ref_count_dec                  (catomicrefcount* arc);
bool    c_atomic_ref_count_compare              (catomicrefcount* arc, cint val);

C_END_EXTERN_C

#endif
