//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_FLAGS_H
#define clibrary_FLAGS_H
#include <initializer_list>

#include "global.h"
#include "3thrd/macros/macros.h"
#include "cx/base/core/global/type-info.h"


CX_BEGIN_NAMESPACE

class CXDataStream;

class CXFlag
{
    int i;
public:
    C_DECL_CONSTEXPR inline CXFlag(int value) noexcept : i(value) {}
    C_DECL_CONSTEXPR inline operator int() const noexcept { return i; }

    C_DECL_CONSTEXPR inline CXFlag(cuint value) noexcept : i(static_cast<cint>(value)) {}
    C_DECL_CONSTEXPR inline CXFlag(cshort value) noexcept : i(static_cast<cint>(value)) {}
    C_DECL_CONSTEXPR inline CXFlag(cushort value) noexcept : i(static_cast<cint>(static_cast<cuint>(value))) {}
    C_DECL_CONSTEXPR inline operator cuint() const noexcept { return static_cast<cuint>(i); }
};
CX_DECLARE_TYPEINFO(CXFlag, CX_PRIMITIVE_TYPE);

class CXIncompatibleFlag
{
    int i;
public:
    C_DECL_CONSTEXPR inline explicit CXIncompatibleFlag(int i) noexcept;
    C_DECL_CONSTEXPR inline operator int() const noexcept { return i; }
};
CX_DECLARE_TYPEINFO(CXIncompatibleFlag, CX_PRIMITIVE_TYPE);

C_DECL_CONSTEXPR inline CXIncompatibleFlag::CXIncompatibleFlag(int value) noexcept : i(value) {}

#ifndef CX_NO_TYPESAFE_FLAGS

template<typename Enum>
class CXFlags
{
    CX_STATIC_ASSERT_X((sizeof(Enum) <= sizeof(cint)), "CXFlags uses an int as storage, so an enum with underlying long long will overflow.");
    CX_STATIC_ASSERT_X((std::is_enum<Enum>::value), "CXFlags is only usable on enumeration types.");

    struct Private;
    typedef int (Private::*Zero);
    template <typename E> friend CXDataStream &operator>>(CXDataStream&, CXFlags<E> &);
    template <typename E> friend CXDataStream &operator<<(CXDataStream&, CXFlags<E>);
public:
    typedef typename std::conditional<
            std::is_unsigned<typename std::underlying_type<Enum>::type>::value,
            unsigned int,
            signed int
        >::type Int;
    typedef Enum enum_type;
    // compiler-generated copy/move ctor/assignment operators are fine!
    C_DECL_CONSTEXPR inline CXFlags() noexcept : i(0) {}
    C_DECL_CONSTEXPR inline CXFlags(Enum flags) noexcept : i(Int(flags)) {}
    C_DECL_CONSTEXPR inline CXFlags(CXFlag flag) noexcept : i(flag) {}
    C_DECL_CONSTEXPR inline CXFlags(std::initializer_list<Enum> flags) noexcept
        : i(initializer_list_helper(flags.begin(), flags.end())) {}

    C_DECL_RELAXED_CONSTEXPR inline CXFlags &operator&=(cint mask) noexcept { i &= mask; return *this; }
    C_DECL_RELAXED_CONSTEXPR inline CXFlags &operator&=(cuint mask) noexcept { i &= mask; return *this; }
    C_DECL_RELAXED_CONSTEXPR inline CXFlags &operator&=(Enum mask) noexcept { i &= Int(mask); return *this; }
    C_DECL_RELAXED_CONSTEXPR inline CXFlags &operator|=(CXFlags other) noexcept { i |= other.i; return *this; }
    C_DECL_RELAXED_CONSTEXPR inline CXFlags &operator|=(Enum other) noexcept { i |= Int(other); return *this; }
    C_DECL_RELAXED_CONSTEXPR inline CXFlags &operator^=(CXFlags other) noexcept { i ^= other.i; return *this; }
    C_DECL_RELAXED_CONSTEXPR inline CXFlags &operator^=(Enum other) noexcept { i ^= Int(other); return *this; }

    C_DECL_CONSTEXPR inline operator Int() const noexcept { return i; }

    C_DECL_CONSTEXPR inline CXFlags operator|(CXFlags other) const noexcept { return CXFlags(CXFlag(i | other.i)); }
    C_DECL_CONSTEXPR inline CXFlags operator|(Enum other) const noexcept { return CXFlags(CXFlag(i | Int(other))); }
    C_DECL_CONSTEXPR inline CXFlags operator^(CXFlags other) const noexcept { return CXFlags(CXFlag(i ^ other.i)); }
    C_DECL_CONSTEXPR inline CXFlags operator^(Enum other) const noexcept { return CXFlags(CXFlag(i ^ Int(other))); }
    C_DECL_CONSTEXPR inline CXFlags operator&(cint mask) const noexcept { return CXFlags(CXFlag(i & mask)); }
    C_DECL_CONSTEXPR inline CXFlags operator&(cuint mask) const noexcept { return CXFlags(CXFlag(i & mask)); }
    C_DECL_CONSTEXPR inline CXFlags operator&(Enum other) const noexcept { return CXFlags(CXFlag(i & Int(other))); }
    C_DECL_CONSTEXPR inline CXFlags operator~() const noexcept { return CXFlags(CXFlag(~i)); }

    C_DECL_CONSTEXPR inline bool operator!() const noexcept { return !i; }

    C_DECL_CONSTEXPR inline bool testFlag(Enum flag) const noexcept
    { return (i & Int(flag)) == Int(flag) && (Int(flag) != 0 || i == Int(flag) ); }
    C_DECL_RELAXED_CONSTEXPR inline CXFlags &setFlag(Enum flag, bool on = true) noexcept
    {
        return on ? (*this |= flag) : (*this &= ~Int(flag));
    }

private:
    C_DECL_CONSTEXPR static inline Int initializer_list_helper(typename std::initializer_list<Enum>::const_iterator it,
                                                               typename std::initializer_list<Enum>::const_iterator end)
    noexcept
    {
        return (it == end ? Int(0) : (Int(*it) | initializer_list_helper(it + 1, end)));
    }

    Int i;
};

#ifndef CX_MOC_RUN
#define CX_DECLARE_FLAGS(Flags, Enum)\
typedef CXFlags<Enum> Flags;
#endif

#define CX_DECLARE_INCOMPATIBLE_FLAGS(Flags) \
C_DECL_CONSTEXPR inline CXIncompatibleFlag operator|(Flags::enum_type f1, int f2) noexcept \
{ return CXIncompatibleFlag(int(f1) | f2); }

#define CX_DECLARE_OPERATORS_FOR_FLAGS(Flags) \
C_DECL_CONSTEXPR inline CXFlags<Flags::enum_type> operator|(Flags::enum_type f1, Flags::enum_type f2) noexcept \
{ return CXFlags<Flags::enum_type>(f1) | f2; } \
C_DECL_CONSTEXPR inline CXFlags<Flags::enum_type> operator|(Flags::enum_type f1, CXFlags<Flags::enum_type> f2) noexcept \
{ return f2 | f1; } CX_DECLARE_INCOMPATIBLE_FLAGS(Flags)

#else /* CX_NO_TYPESAFE_FLAGS */

#ifndef CX_MOC_RUN
#define CX_DECLARE_FLAGS(Flags, Enum) \
typedef cuint Flags;
#endif

#define CX_DECLARE_OPERATORS_FOR_FLAGS(Flags)

#endif

// restore bit-wise enum-enum operators deprecated in C++20,
// but used in a few places in the API
#if (__cplusplus > 201702L) // assume compilers don't warn if in C++17 mode
  // in C++20 mode, provide user-defined operators to override the deprecated operations:
# define CX_DECLARE_MIXED_ENUM_OPERATOR(op, Ret, LHS, RHS) \
    constexpr inline Ret operator op (LHS lhs, RHS rhs) noexcept \
    { return static_cast<Ret>(std::underlying_type_t<LHS>(lhs) op std::underlying_type_t<RHS>(rhs)); } \
    /* end */
#else
  // in C++11-17 mode, statically-assert that this compiler's result of the
  // operation is the same that the C++20 version would produce:
# define CX_DECLARE_MIXED_ENUM_OPERATOR(op, Ret, LHS, RHS) \
    CX_STATIC_ASSERT((std::is_same<decltype(std::declval<LHS>() op std::declval<RHS>()), Ret>::value));
#endif

#define CX_DECLARE_MIXED_ENUM_OPERATORS(Ret, Flags, Enum) \
    CX_DECLARE_MIXED_ENUM_OPERATOR(|, Ret, Flags, Enum) \
    CX_DECLARE_MIXED_ENUM_OPERATOR(&, Ret, Flags, Enum) \
    CX_DECLARE_MIXED_ENUM_OPERATOR(^, Ret, Flags, Enum) \
    /* end */

#define CX_DECLARE_MIXED_ENUM_OPERATORS_SYMMETRIC(Ret, Flags, Enum) \
    CX_DECLARE_MIXED_ENUM_OPERATORS(Ret, Flags, Enum) \
    CX_DECLARE_MIXED_ENUM_OPERATORS(Ret, Enum, Flags) \
    /* end */


CX_END_NAMESPACE

#endif // clibrary_FLAGS_H
