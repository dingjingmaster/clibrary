
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


/**
 * @brief 原子操作，获取值
 */
int     c_atomic_int_get                        (const volatile int *atomic);

/**
 * @brief 原子操作，设置值
 */
void    c_atomic_int_set                        (volatile int* atomic, int newVal);

/**
 * @brief 原子操作，值+1
 */
void    c_atomic_int_inc                        (volatile int* atomic);

/**
 * @brief 原子操作，值-1，并测试值-1之后是否为0
 * @return 值是否为0
 */
bool    c_atomic_int_dec_and_test               (volatile int* atomic);

/**
 * @brief 原子操作，比较值为 oldValue，则把值设置为 newVal
 * @return 值是否为 oldVal
 */
bool    c_atomic_int_compare_and_exchange       (volatile int* atomic, int oldVal, int newVal);

/**
 * @brief 原子操作，先备份 atomic 的值，保存到 preVal 变量中；再确定 atomic 的值是否为 oldVal，是的话则替换为 newVal
 * @return
 */
bool    c_atomic_int_compare_and_exchange_full  (int* atomic, int oldVal, int newVal, int* preVal);

/**
 * @brief 原子操作，值设置为newVal，并返回原来的值
 */
int     c_atomic_int_exchange                   (int* atomic, int newVal);

/**
 * @brief 原子操作，atomic = atomic + val，并返回相加后的结果
 */
int     c_atomic_int_add                        (volatile int* atomic, int val);

/**
 * @brief 原子操作，atomic = atomic & val，并返回按位与后的结果
 */
cuint   c_atomic_int_and                        (volatile cuint* atomic, cuint val);

/**
 * @brief 原子操作，atomic = atomic | val，并返回按位或的结果
 */
cuint   c_atomic_int_or                         (volatile cuint* atomic, cuint val);

/**
 * @brief 原子操作，atomic = atomic ^ val，并返回按位异或的结果
 */
cuint   c_atomic_int_xor                        (volatile cuint *atomic, cuint val);

/**
 * @brief 原子操作
 */
int     c_atomic_int_exchange_and_add           (volatile int *atomic, int val);

/**
 * @brief 原子操作，atomic 为指针，返回其对应的指针值
 */
void*   c_atomic_pointer_get                    (const volatile void* atomic);

/**
 * @brief 原子操作，atomic 为指针，设置新的指针值
 */
void    c_atomic_pointer_set                    (volatile void* atomic, void* newVal);

/**
 * @brief 原子操作，交换
 */
bool    c_atomic_pointer_compare_and_exchange   (volatile void* atomic, void* oldVal, void* newVal);

/**
 * @brief 原子操作，交换
 */
bool    c_atomic_pointer_compare_and_exchange_full (void* atomic, void* oldVal, void* newVal, void* preVal);

/**
 * @brief 原子操作，
 */
void*   c_atomic_pointer_exchange               (void* atomic, void* newVal);

/**
 * @brief 原子操作，
 */
culong  c_atomic_pointer_add                    (volatile void* atomic, culong val);

/**
 * @brief 原子操作
 */
culong  c_atomic_pointer_and                    (volatile void* atomic, culong val);

/**
 * @brief 原子操作
 */
culong  c_atomic_pointer_or                     (volatile void* atomic, culong val);

/**
 * @brief 原子操作
 */
culong  c_atomic_pointer_xor                    (volatile void* atomic, culong val);

/**
 * @brief 引用初始化，不加锁
 */
void    c_ref_count_init                        (crefcount* rc);

/**
 * @brief 引用+1，不加锁
 */
void    c_ref_count_inc                         (crefcount* rc);

/**
 * @brief 引用-1, 不加锁
 */
bool    c_ref_count_dec                         (crefcount* rc);

/**
 * @brief 引用比较是否相等，不加锁
 */
bool    c_ref_count_compare                     (crefcount* rc, cint val);

/**
 * @brief 原子操作，初始化 arc
 */
void    c_atomic_ref_count_init                 (catomicrefcount* arc);

/**
 * @brief 原子操作，+1
 */
void    c_atomic_ref_count_inc                  (catomicrefcount* arc);

/**
 * @brief 原子操作，-1
 */
bool    c_atomic_ref_count_dec                  (catomicrefcount* arc);

/**
 * @brief 原子操作，比较是否相等
 */
bool    c_atomic_ref_count_compare              (catomicrefcount* arc, cint val);

C_END_EXTERN_C

#endif
