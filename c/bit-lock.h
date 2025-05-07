//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-7.
//

#ifndef clibrary_BIT_LOCK_H
#define clibrary_BIT_LOCK_H

#if !defined (__CLIB_H_INSIDE__) && !defined (CLIB_COMPILATION)
#error "Only <clib.h> can be included directly."
#endif
#include <c/macros.h>

C_BEGIN_EXTERN_C

void        c_bit_lock                      (volatile cint* address, cint lockBit);
bool        c_bit_trylock                   (volatile cint* address, cint lockBit);
void        c_bit_unlock                    (volatile cint* address, cint lockBit);
void        c_pointer_bit_lock              (volatile void* address, cint lockBit);
void        c_pointer_bit_lock_and_get      (void* address, cuint lockBit, cuintptr* outPtr);
bool        c_pointer_bit_trylock           (volatile void* address, cint lockBit);
void        c_pointer_bit_unlock            (volatile void* address, cint lockBit);
void*       c_pointer_bit_lock_mask_ptr     (void* ptr, cuint lockBit, bool set, cuintptr preserveMask, void* preservePtr);
void        c_pointer_bit_unlock_and_set    (void* address, cuint lockBit, void* ptr, cuintptr preserveMask);

C_END_EXTERN_C

#endif // clibrary_BIT_LOCK_H
