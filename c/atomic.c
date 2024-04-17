
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*************************************************************************
> FileName: atomic.c
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Thu 07 Sep 2022 22:20:19 PM CST
************************************************************************/

#include "atomic.h"

#include <pthread.h>

static pthread_mutex_t gsAtomicLock = PTHREAD_MUTEX_INITIALIZER;

int c_atomic_int_get (const volatile int *atomic)
{
    int value;

    pthread_mutex_lock (&gsAtomicLock);
    value = *atomic;
    pthread_mutex_unlock (&gsAtomicLock);

    return value;
}

void c_atomic_int_set (volatile int *atomic, int value)
{
    pthread_mutex_lock (&gsAtomicLock);
    *atomic = value;
    pthread_mutex_unlock (&gsAtomicLock);
}

void c_atomic_int_inc (volatile int *atomic)
{
    pthread_mutex_lock (&gsAtomicLock);
    (*atomic)++;
    pthread_mutex_unlock (&gsAtomicLock);
}

bool c_atomic_int_dec_and_test (volatile int *atomic)
{
    bool isZero;

    pthread_mutex_lock (&gsAtomicLock);
    isZero = --(*atomic) == 0;
    pthread_mutex_unlock (&gsAtomicLock);

    return isZero;
}

bool c_atomic_int_compare_and_exchange (volatile int *atomic, int oldval, int newval)
{
    bool success;

    pthread_mutex_lock (&gsAtomicLock);

    if ((success = (*atomic == oldval)))
        *atomic = newval;

    pthread_mutex_unlock (&gsAtomicLock);

    return success;
}

bool c_atomic_int_compare_and_exchange_full (int *atomic, int oldval, int newval, int* preval)
{
    bool     success;

    pthread_mutex_lock (&gsAtomicLock);

    *preval = *atomic;

    if ((success = (*atomic == oldval))) {
        *atomic = newval;
    }

    pthread_mutex_unlock (&gsAtomicLock);

    return success;
}

int c_atomic_int_exchange (int *atomic, int newval)
{
    int *ptr = atomic;
    int oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *ptr;
    *ptr = newval;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}

int c_atomic_int_add (volatile int *atomic, int val)
{
    int oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *atomic;
    *atomic = oldval + val;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}

cuint c_atomic_int_and (volatile cuint *atomic, cuint val)
{
    cuint oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *atomic;
    *atomic = oldval & val;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}

cuint c_atomic_int_or (volatile cuint *atomic, cuint val)
{
    cuint oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *atomic;
    *atomic = oldval | val;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}

cuint c_atomic_int_xor (volatile cuint *atomic, cuint val)
{
    cuint oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *atomic;
    *atomic = oldval ^ val;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}

void* c_atomic_pointer_get (const volatile void* atomic)
{
    const void** ptr = (const void**) atomic;
    void* value;

    pthread_mutex_lock (&gsAtomicLock);
    value = (void*) *ptr;
    pthread_mutex_unlock (&gsAtomicLock);

    return value;
}


void c_atomic_pointer_set (volatile void* atomic, void* newval)
{
    void** ptr = (void**) atomic;

    pthread_mutex_lock (&gsAtomicLock);
    *ptr = newval;
    pthread_mutex_unlock (&gsAtomicLock);
}

bool c_atomic_pointer_compare_and_exchange (volatile void* atomic, void* oldval, void* newval)
{
    void** ptr = (void**) atomic;
    bool success;

    pthread_mutex_lock (&gsAtomicLock);
    if ((success = (*ptr == oldval))) {
        *ptr = newval;
    }
    pthread_mutex_unlock (&gsAtomicLock);

    return success;
}

bool c_atomic_pointer_compare_and_exchange_full (void* atomic, void* oldval, void* newval, void* preval)
{
    void** ptr = atomic;
    void** pre = preval;
    bool success;

    pthread_mutex_lock (&gsAtomicLock);

    *pre = *ptr;
    if ((success = (*ptr == oldval))) {
        *ptr = newval;
    }

    pthread_mutex_unlock (&gsAtomicLock);

    return success;
}

void* c_atomic_pointer_exchange (void* atomic, void* newval)
{
    void** ptr = atomic;
    void* oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *ptr;
    *ptr = newval;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}

cuint64 c_atomic_pointer_add (volatile void* atomic, cuint64 val)
{
    cuint64* ptr = (cuint64*) atomic;
    cuint64 oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *ptr;
    *ptr = oldval + val;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}

cuint64 c_atomic_pointer_and (volatile void* atomic, cuint64 val)
{
    cuint64* ptr = (cuint64*) atomic;
    cuint64 oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *ptr;
    *ptr = oldval & val;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}

cuint64 c_atomic_pointer_or (volatile void* atomic, cuint64 val)
{
    cuint64* ptr = (cuint64*) atomic;
    cuint64 oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *ptr;
    *ptr = oldval | val;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}

cuint64 c_atomic_pointer_xor (volatile void* atomic, cuint64 val)
{
    cuint64* ptr = (cuint64*) atomic;
    cuint64 oldval;

    pthread_mutex_lock (&gsAtomicLock);
    oldval = *ptr;
    *ptr = oldval ^ val;
    pthread_mutex_unlock (&gsAtomicLock);

    return oldval;
}


cint g_atomic_int_exchange_and_add (volatile cint *atomic, cint val)
{
    return c_atomic_int_add ((cint*) atomic, val);
}
