//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_GLOBAL_H
#define clibrary_GLOBAL_H
#include "define.h"


#ifndef __cplusplus
// In C++ mode, we define below using QIntegerForSize template
CX_STATIC_ASSERT_X(sizeof(ptrdiff_t) == sizeof(size_t), "Weird ptrdiff_t and size_t definitions");
typedef ptrdiff_t cxptrdiff;
typedef ptrdiff_t cxsizetype;
typedef ptrdiff_t cxintptr;
typedef size_t cxuintptr;
#endif


#if defined(__cplusplus) && defined(CX_COMPILER_STATIC_ASSERT)
#  define CX_STATIC_ASSERT(Condition) static_assert(bool(Condition), #Condition)
#  define CX_STATIC_ASSERT_X(Condition, Message) static_assert(bool(Condition), Message)
#elif defined(CX_COMPILER_STATIC_ASSERT)
// C11 mode - using the _S version in case <assert.h> doesn't do the right thing
#  define CX_STATIC_ASSERT(Condition) _Static_assert(!!(Condition), #Condition)
#  define CX_STATIC_ASSERT_X(Condition, Message) _Static_assert(!!(Condition), Message)
#else
// C89 & C99 version
#  define CX_STATIC_ASSERT_PRIVATE_JOIN(A, B) CX_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B)
#  define CX_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B) A ## B
#  ifdef __COUNTER__
#  define CX_STATIC_ASSERT(Condition) \
typedef char CX_STATIC_ASSERT_PRIVATE_JOIN(cx_static_assert_result, __COUNTER__) [(Condition) ? 1 : -1];
#  else
#  define CX_STATIC_ASSERT(Condition) \
typedef char CX_STATIC_ASSERT_PRIVATE_JOIN(cx_static_assert_result, __LINE__) [(Condition) ? 1 : -1];
#  endif /* __COUNTER__ */
#  define CX_STATIC_ASSERT_X(Condition, Message) CX_STATIC_ASSERT(Condition)
#endif



#endif // clibrary_GLOBAL_H
