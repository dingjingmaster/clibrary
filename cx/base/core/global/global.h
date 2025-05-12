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
#include <cstdio>


#include "define.h"


#ifndef __cplusplus
// In C++ mode, we define below using QIntegerForSize template
CX_STATIC_ASSERT_X(sizeof(ptrdiff_t) == sizeof(size_t), "Weird ptrdiff_t and size_t definitions");
typedef ptrdiff_t cxptrdiff;
typedef ptrdiff_t cxsizetype;
typedef ptrdiff_t cxintptr;
typedef size_t cxuintptr;
#endif


#if defined(__cplusplus) && C_SUPPORTED_CXX11
# define CX_STATIC_ASSERT(Condition) static_assert(bool(Condition), #Condition)
# define CX_STATIC_ASSERT_X(Condition, Message) static_assert(bool(Condition), Message)
#elif C_SUPPORTED_CXX11
// C11 mode - using the _S version in case <assert.h> doesn't do the right thing
# define CX_STATIC_ASSERT(Condition) _Static_assert(!!(Condition), #Condition)
# define CX_STATIC_ASSERT_X(Condition, Message) _Static_assert(!!(Condition), Message)
#else
// C89 & C99 version
# define CX_STATIC_ASSERT_PRIVATE_JOIN(A, B) CX_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B)
# define CX_STATIC_ASSERT_PRIVATE_JOIN_IMPL(A, B) A ## B
# ifdef __COUNTER__
# define CX_STATIC_ASSERT(Condition) \
typedef char CX_STATIC_ASSERT_PRIVATE_JOIN(cx_static_assert_result, __COUNTER__) [(Condition) ? 1 : -1];
# else
# define CX_STATIC_ASSERT(Condition) \
typedef char CX_STATIC_ASSERT_PRIVATE_JOIN(cx_static_assert_result, __LINE__) [(Condition) ? 1 : -1];
# endif /* __COUNTER__ */
# define CX_STATIC_ASSERT_X(Condition, Message) CX_STATIC_ASSERT(Condition)
#endif




#ifdef __cplusplus
// A tag to help mark stuff deprecated (cf. QStringViewLiteral)
namespace CXPrivate
{
    enum class Deprecated_t {};
    constexpr C_DECL_UNUSED Deprecated_t Deprecated = {};
}
#endif


namespace CXPrivate
{
    template <class T>
    struct AlignOfHelper
    {
        char c;
        T type;

        AlignOfHelper();
        ~AlignOfHelper();
    };

    template <class T>
    struct AlignOf_Default
    {
        enum { Value = sizeof(AlignOfHelper<T>) - sizeof(T) };
    };

    template <class T> struct AlignOf : AlignOf_Default<T> { };
    template <class T> struct AlignOf<T &> : AlignOf<T> {};
    template <class T> struct AlignOf<T &&> : AlignOf<T> {};
    template <size_t N, class T> struct AlignOf<T[N]> : AlignOf<T> {};

#if defined(CX_PROCESSOR_X86_32) && !defined(CX_OS_WIN)
    template <class T> struct AlignOf_WorkaroundForI386Abi { enum { Value = sizeof(T) }; };

    // x86 ABI weirdness
    // Alignment of naked type is 8, but inside struct has alignment 4.
    template <> struct AlignOf<double>  : AlignOf_WorkaroundForI386Abi<double> {};
    template <> struct AlignOf<cint64>  : AlignOf_WorkaroundForI386Abi<cint64> {};
    template <> struct AlignOf<cuint64> : AlignOf_WorkaroundForI386Abi<cuint64> {};
#ifdef CX_CC_CLANG
    // GCC and Clang seem to disagree wrt to alignment of arrays
    template <size_t N> struct AlignOf<double[N]>   : AlignOf_Default<double> {};
    template <size_t N> struct AlignOf<cint64[N]>   : AlignOf_Default<cint64> {};
    template <size_t N> struct AlignOf<cuint64[N]>  : AlignOf_Default<cuint64> {};
#endif
#endif
} // namespace CXPrivate


template <int> struct CXIntegerForSize;
template <>    struct CXIntegerForSize<1> { typedef cuint8  Unsigned; typedef cint8  Signed; };
template <>    struct CXIntegerForSize<2> { typedef cuint16 Unsigned; typedef cint16 Signed; };
template <>    struct CXIntegerForSize<4> { typedef cuint32 Unsigned; typedef cint32 Signed; };
template <>    struct CXIntegerForSize<8> { typedef cuint64 Unsigned; typedef cint64 Signed; };
#if defined(CX_CC_GNU) && defined(__SIZEOF_INT128__)
template <>    struct CXIntegerForSize<16> { __extension__ typedef unsigned __int128 Unsigned; __extension__ typedef __int128 Signed; };
#endif
template <class T> struct CXIntegerForSizeof: CXIntegerForSize<sizeof(T)> { };
typedef CXIntegerForSize<C_CPU_WORDSIZE>::Signed cxregisterint;
typedef CXIntegerForSize<C_CPU_WORDSIZE>::Unsigned cxregisteruint;
typedef CXIntegerForSizeof<void*>::Unsigned cxuintptr;
typedef CXIntegerForSizeof<void*>::Signed cxptrdiff;
typedef cxptrdiff cxintptr;
using cxsizetype = CXIntegerForSizeof<std::size_t>::Signed;


#if !defined(CX_ASSERT)
# if defined(CX_NO_DEBUG) && !defined(CX_FORCE_ASSERTS)
#   define CX_ASSERT(cond) static_cast<void>(false && (cond))
# else
#    define CX_ASSERT(cond) ((cond) ? static_cast<void>(0) : cx_assert(#cond, __FILE__, __LINE__))
# endif
#endif

using csizetype = CXIntegerForSizeof<std::size_t>::Signed;

#endif // clibrary_GLOBAL_H
