//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-7.
//

#include "bit-lock.h"

#include "slist.h"
#include "atomic.h"
#include "thread.h"


#define CONTENTION_CLASSES 11
static cint c_bit_lock_contended[CONTENTION_CLASSES];

static inline cuint bit_lock_contended_class (void* address)
{
    return ((csize) address) % C_N_ELEMENTS (c_bit_lock_contended);
}

static inline void* pointer_bit_lock_mask_ptr (void* ptr, cuint lock_bit, bool set, cuintptr preserve_mask, void* preserve_ptr)
{
    cuintptr x_ptr;
    cuintptr x_preserve_ptr;
    cuintptr lock_mask;

    x_ptr = (cuintptr) ptr;

    if (preserve_mask != 0) {
        x_preserve_ptr = (cuintptr) preserve_ptr;
        x_ptr = (x_preserve_ptr & preserve_mask) | (x_ptr & ~preserve_mask);
    }

    if (lock_bit == C_MAX_UINT) {
        return (void*) x_ptr;
    }

    lock_mask = (cuintptr) (1u << lock_bit);
    if (set) {
        return (void*) (x_ptr | lock_mask);
    }

    return (void*) (x_ptr & ~lock_mask);
}

typedef struct
{
    const cint *address;
    cint ref_count;
    CCond wait_queue;
} WaitAddress;

static void c_futex_wake (const cint *address);
static void c_futex_wait (const cint* address, cint value);
static const cint* c_futex_int_address (const void *address);
static WaitAddress* c_futex_find_address (const cint *address);

static CMutex c_futex_mutex;
static CSList* c_futex_address_list = NULL;

void c_bit_lock(volatile cint *address, cint lockBit)
{
    cint* address_nonvolatile = (cint*) address;

    cuint mask = 1u << lockBit;
    cuint v;

retry:
    v = c_atomic_int_or (address_nonvolatile, mask);
    if (v & mask) {
        cuint class = bit_lock_contended_class (address_nonvolatile);
        c_atomic_int_add (&c_bit_lock_contended[class], +1);
        c_futex_wait (address_nonvolatile, v);
        c_atomic_int_add (&c_bit_lock_contended[class], -1);

        goto retry;
    }
}

bool c_bit_trylock(volatile cint *address, cint lockBit)
{
    cint *address_nonvolatile = (cint*) address;
    cuint mask = 1u << lockBit;
    cuint v = c_atomic_int_or (address_nonvolatile, mask);

    return ~v & mask;
}

void c_bit_unlock(volatile cint *address, cint lockBit)
{
    cint *address_nonvolatile = (cint*) address;
    cuint mask = 1u << lockBit;

    c_atomic_int_and (address_nonvolatile, ~mask);
    cuint class = bit_lock_contended_class (address_nonvolatile);

    if (c_atomic_int_get (&c_bit_lock_contended[class])) {
        c_futex_wake (address_nonvolatile);
    }
}

void c_pointer_bit_lock(volatile void *address, cint lockBit)
{
    c_pointer_bit_lock_and_get ((void**) address, (cuint) lockBit, NULL);
}

void c_pointer_bit_lock_and_get(void *address, cuint lockBit, cuintptr *outPtr)
{
    cuint class = bit_lock_contended_class (address);
    cuintptr mask;
    cuintptr v;

    c_return_if_fail (lockBit < 32);

    mask = 1u << lockBit;

retry:
    v = c_atomic_pointer_or ((void**) address, mask);
    if (v & mask) {
        c_atomic_int_add (&c_bit_lock_contended[class], +1);
        c_futex_wait (c_futex_int_address (address), (cuint) v);
        c_atomic_int_add (&c_bit_lock_contended[class], -1);
        goto retry;
    }

    if (outPtr) {
        *outPtr = (v | mask);
    }
}

bool c_pointer_bit_trylock(volatile void *address, cint lockBit)
{
    c_return_val_if_fail (lockBit < 32, false);

    void *address_nonvolatile = (void *) address;
    void** pointer_address = address_nonvolatile;
    csize mask = 1u << lockBit;
    cuintptr v;

    c_return_val_if_fail (lockBit < 32, false);

    v = c_atomic_pointer_or (pointer_address, mask);

    return (~(csize) v & mask) != 0;
}

void c_pointer_bit_unlock(volatile void *address, cint lockBit)
{
    void *address_nonvolatile = (void *) address;

    c_return_if_fail (lockBit < 32);

    void** pointer_address = address_nonvolatile;
    csize mask = 1u << lockBit;

    c_atomic_pointer_and (pointer_address, ~mask);
    cuint class = bit_lock_contended_class (address_nonvolatile);

    if (c_atomic_int_get (&c_bit_lock_contended[class])) {
        c_futex_wake (c_futex_int_address (address_nonvolatile));
    }
}

void *c_pointer_bit_lock_mask_ptr(void *ptr, cuint lockBit, bool set, cuintptr preserveMask, void *preservePtr)
{
    c_return_val_if_fail (lockBit < 32u || lockBit == C_MAX_UINT, ptr);

    return pointer_bit_lock_mask_ptr (ptr, lockBit, set, preserveMask, preservePtr);
}

void c_pointer_bit_unlock_and_set(void *address, cuint lockBit, void *ptr, cuintptr preserveMask)
{
    void** pointer_address = address;
    cuint class = bit_lock_contended_class (address);
    void* ptr2;

    c_return_if_fail (lockBit < 32u);

    if (preserveMask != 0) {
        void* old_ptr = c_atomic_pointer_get ((void**) address);
again:
        ptr2 = pointer_bit_lock_mask_ptr (ptr, lockBit, false, preserveMask, old_ptr);
        if (!c_atomic_pointer_compare_and_exchange_full (pointer_address, old_ptr, ptr2, &old_ptr)) {
            goto again;
        }
    }
    else {
        ptr2 = pointer_bit_lock_mask_ptr (ptr, lockBit, false, 0, NULL);
        c_atomic_pointer_set (pointer_address, ptr2);
    }

    if (c_atomic_int_get (&c_bit_lock_contended[class]) > 0) {
        c_futex_wake (c_futex_int_address (address));
    }

    c_return_if_fail (ptr == pointer_bit_lock_mask_ptr (ptr, lockBit, false, 0, NULL));
}

static WaitAddress* c_futex_find_address (const cint *address)
{
    CSList *node;
    for (node = c_futex_address_list; node; node = node->next) {
        WaitAddress *waiter = node->data;
        if (waiter->address == address) {
            return waiter;
        }
    }

    return NULL;
}

static void c_futex_wait (const cint* address, cint value)
{
    c_mutex_lock (&c_futex_mutex);
    if C_LIKELY (c_atomic_int_get (address) == value) {
        WaitAddress *waiter;
        if ((waiter = c_futex_find_address (address)) == NULL) {
            waiter = c_malloc0(sizeof(WaitAddress));
            waiter->address = address;
            c_cond_init (&waiter->wait_queue);
            waiter->ref_count = 0;
            c_futex_address_list = c_slist_prepend (c_futex_address_list, waiter);
        }

        waiter->ref_count++;
        c_cond_wait (&waiter->wait_queue, &c_futex_mutex);

        if (!--waiter->ref_count) {
            c_futex_address_list = c_slist_remove (c_futex_address_list, waiter);
            c_cond_clear (&waiter->wait_queue);
            c_free0(waiter);
        }
    }
    c_mutex_unlock (&c_futex_mutex);
}

static void c_futex_wake (const cint *address)
{
    WaitAddress *waiter;

    c_mutex_lock (&c_futex_mutex);
    if ((waiter = c_futex_find_address (address))) {
        c_cond_signal (&waiter->wait_queue);
    }
    c_mutex_unlock (&c_futex_mutex);
}

static const cint* c_futex_int_address (const void *address)
{
    const cint *int_address = address;

    /* this implementation makes these (reasonable) assumptions: */
    C_STATIC_ASSERT (C_BYTE_ORDER == C_LITTLE_ENDIAN
        || (C_BYTE_ORDER == C_BIG_ENDIAN && sizeof (int) == 4 && (sizeof (void*) == 4 || sizeof (void*) == 8)));

#if C_BYTE_ORDER == C_BIG_ENDIAN && SIZEOF_VOID_P == 8
    int_address++;
#endif

    return int_address;
}