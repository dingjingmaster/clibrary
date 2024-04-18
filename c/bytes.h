
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-4-18.
//

#ifndef CLIBRARY_BYTES_H
#define CLIBRARY_BYTES_H

#if !defined (__CLIB_H_INSIDE__) && !defined (CLIB_COMPILATION)
#error "Only <clib.h> can be included directly."
#endif

#include <c/array.h>

C_BEGIN_EXTERN_C

CBytes*         c_bytes_new                     (const void* data, csize size);
CBytes*         c_bytes_new_take                (void* data, csize size);
CBytes*         c_bytes_new_static              (const void* data, csize size);
CBytes*         c_bytes_new_with_free_func      (const void* data, csize size, CDestroyNotify freeFunc, void* udata);
CBytes*         c_bytes_new_from_bytes          (CBytes* bytes, csize offset, csize length);
const void*     c_bytes_get_data                (CBytes* bytes, csize* size);
csize           c_bytes_get_size                (CBytes* bytes);
CBytes*         c_bytes_ref                     (CBytes* bytes);
void            c_bytes_unref                   (CBytes* bytes);
void*           c_bytes_unref_to_data           (CBytes* bytes, csize* size);
CByteArray*     c_bytes_unref_to_array          (CBytes* bytes);
cuint           c_bytes_hash                    (const void* bytes);
bool            c_bytes_equal                   (const void* bytes1, const void* bytes2);
cint            c_bytes_compare                 (const void* bytes1, const void* bytes2);
const void*     c_bytes_get_region              (CBytes* bytes, csize elementSize, csize offset, csize nElements);

C_END_EXTERN_C

#endif //CLIBRARY_BYTES_H
