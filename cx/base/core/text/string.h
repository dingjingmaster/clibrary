//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_CX_STRING_H
#define clibrary_CX_STRING_H

#include "../global/flags.h"
#include "3thrd/macros/macros.h"
#include "cx/base/core/text/bytearray.h"
#include "cx/base/core/global/name-space.h"

#ifdef truncate
#error qstring.h must be included before any header file that defines truncate
#endif

CX_BEGIN_NAMESPACE

class CXRegExp;
class CXString;
class CXCharRef;
class CXTextCodec;
class CXStringRef;
class CXStringList;
class CXRegularExpression;
class CXRegularExpressionMatch;

template <typename T> class CXVector;

namespace CXPrivate
{
template <bool...B> class BoolList;
}

class CXLatin1String
{
public:
    C_DECL_CONSTEXPR inline CXLatin1String() noexcept : m_size(0), m_data(nullptr) {}
    C_DECL_CONSTEXPR inline explicit CXLatin1String(const char *s) noexcept : m_size(s ? int(c_strlen(s)) : 0), m_data(s) {}
    C_DECL_CONSTEXPR explicit CXLatin1String(const char *f, const char *l)
        : CXLatin1String(f, int(l - f)) {}
    C_DECL_CONSTEXPR inline explicit CXLatin1String(const char *s, int sz) noexcept : m_size(sz), m_data(s) {}
    inline explicit CXLatin1String(const CXByteArray &s) noexcept : m_size(int(cx_strnlen(s.constData(), s.size()))), m_data(s.constData()) {}

    C_DECL_CONSTEXPR const char *latin1() const noexcept { return m_data; }
    C_DECL_CONSTEXPR int size() const noexcept { return m_size; }
    C_DECL_CONSTEXPR const char *data() const noexcept { return m_data; }

    C_DECL_CONSTEXPR bool isNull() const noexcept { return !data(); }
    C_DECL_CONSTEXPR bool isEmpty() const noexcept { return !size(); }

    template <typename...Args>
    C_REQUIRED_RESULT inline CXString arg(Args &&...args) const;

    C_DECL_CONSTEXPR CXLatin1Char at(int i) const
    { return C_ASSERT(i >= 0), C_ASSERT(i < size()), CXLatin1Char(m_data[i]); }
    C_DECL_CONSTEXPR CXLatin1Char operator[](int i) const { return at(i); }

    C_REQUIRED_RESULT C_DECL_CONSTEXPR CXLatin1Char front() const { return at(0); }
    C_REQUIRED_RESULT C_DECL_CONSTEXPR CXLatin1Char back() const { return at(size() - 1); }

    C_REQUIRED_RESULT int compare(CXStringView other, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::compareStrings(*this, other, cs); }
    C_REQUIRED_RESULT int compare(CXLatin1String other, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::compareStrings(*this, other, cs); }
    C_REQUIRED_RESULT C_DECL_CONSTEXPR int compare(CXChar c) const noexcept
    { return isEmpty() || front() == c ? size() - 1 : cuchar(m_data[0]) - c.unicode() ; }
    C_REQUIRED_RESULT int compare(CXChar c, cx::CaseSensitivity cs) const noexcept
    { return CXPrivate::compareStrings(*this, CXStringView(&c, 1), cs); }

    C_REQUIRED_RESULT bool startsWith(CXStringView s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::startsWith(*this, s, cs); }
    C_REQUIRED_RESULT bool startsWith(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::startsWith(*this, s, cs); }
    C_REQUIRED_RESULT C_DECL_CONSTEXPR bool startsWith(CXChar c) const noexcept
    { return !isEmpty() && front() == c; }
    C_REQUIRED_RESULT inline bool startsWith(CXChar c, cx::CaseSensitivity cs) const noexcept
    { return CXPrivate::startsWith(*this, CXStringView(&c, 1), cs); }

    C_REQUIRED_RESULT bool endsWith(CXStringView s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::endsWith(*this, s, cs); }
    C_REQUIRED_RESULT bool endsWith(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::endsWith(*this, s, cs); }
    C_REQUIRED_RESULT C_DECL_CONSTEXPR bool endsWith(CXChar c) const noexcept
    { return !isEmpty() && back() == c; }
    C_REQUIRED_RESULT inline bool endsWith(CXChar c, cx::CaseSensitivity cs) const noexcept
    { return CXPrivate::endsWith(*this, CXStringView(&c, 1), cs); }

    C_REQUIRED_RESULT int indexOf(CXStringView s, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::findString(*this, from, s, cs)); }
    C_REQUIRED_RESULT int indexOf(CXLatin1String s, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::findString(*this, from, s, cs)); }
    C_REQUIRED_RESULT inline int indexOf(CXChar c, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::findString(*this, from, CXStringView(&c, 1), cs)); }

    C_REQUIRED_RESULT bool contains(CXStringView s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return indexOf(s, 0, cs) != -1; }
    C_REQUIRED_RESULT bool contains(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return indexOf(s, 0, cs) != -1; }
    C_REQUIRED_RESULT inline bool contains(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return indexOf(CXStringView(&c, 1), 0, cs) != -1; }

    C_REQUIRED_RESULT int lastIndexOf(CXStringView s, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::lastIndexOf(*this, from, s, cs)); }
    C_REQUIRED_RESULT int lastIndexOf(CXLatin1String s, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::lastIndexOf(*this, from, s, cs)); }
    C_REQUIRED_RESULT inline int lastIndexOf(CXChar c, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::lastIndexOf(*this, from, CXStringView(&c, 1), cs)); }

    using value_type = const char;
    using reference = value_type&;
    using const_reference = reference;
    using iterator = value_type*;
    using const_iterator = iterator;
    using difference_type = int; // violates Container concept requirements
    using size_type = int;       // violates Container concept requirements

    C_DECL_CONSTEXPR const_iterator begin() const noexcept { return data(); }
    C_DECL_CONSTEXPR const_iterator cbegin() const noexcept { return data(); }
    C_DECL_CONSTEXPR const_iterator end() const noexcept { return data() + size(); }
    C_DECL_CONSTEXPR const_iterator cend() const noexcept { return data() + size(); }

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = reverse_iterator;

    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

    C_DECL_CONSTEXPR CXLatin1String mid(int pos) const
    { return C_ASSERT(pos >= 0), C_ASSERT(pos <= size()), CXLatin1String(m_data + pos, m_size - pos); }
    C_DECL_CONSTEXPR CXLatin1String mid(int pos, int n) const
    { return C_ASSERT(pos >= 0), C_ASSERT(n >= 0), C_ASSERT(pos + n <= size()), CXLatin1String(m_data + pos, n); }
    C_DECL_CONSTEXPR CXLatin1String left(int n) const
    { return C_ASSERT(n >= 0), C_ASSERT(n <= size()), CXLatin1String(m_data, n); }
    C_DECL_CONSTEXPR CXLatin1String right(int n) const
    { return C_ASSERT(n >= 0), C_ASSERT(n <= size()), CXLatin1String(m_data + m_size - n, n); }
    C_REQUIRED_RESULT C_DECL_CONSTEXPR CXLatin1String chopped(int n) const
    { return C_ASSERT(n >= 0), C_ASSERT(n <= size()), CXLatin1String(m_data, m_size - n); }

    C_DECL_RELAXED_CONSTEXPR void chop(int n)
    { C_ASSERT(n >= 0); C_ASSERT(n <= size()); m_size -= n; }
    C_DECL_RELAXED_CONSTEXPR void truncate(int n)
    { C_ASSERT(n >= 0); C_ASSERT(n <= size()); m_size = n; }

    C_REQUIRED_RESULT CXLatin1String trimmed() const noexcept { return CXPrivate::trimmed(*this); }

    inline bool operator==(const CXString &s) const noexcept;
    inline bool operator!=(const CXString &s) const noexcept;
    inline bool operator>(const CXString &s) const noexcept;
    inline bool operator<(const CXString &s) const noexcept;
    inline bool operator>=(const CXString &s) const noexcept;
    inline bool operator<=(const CXString &s) const noexcept;

#if !defined(CX_NO_CAST_FROM_ASCII) && !defined(CX_RESTRICTED_CAST_FROM_ASCII)
    inline CX_ASCII_CAST_WARN bool operator==(const char *s) const;
    inline CX_ASCII_CAST_WARN bool operator!=(const char *s) const;
    inline CX_ASCII_CAST_WARN bool operator<(const char *s) const;
    inline CX_ASCII_CAST_WARN bool operator>(const char *s) const;
    inline CX_ASCII_CAST_WARN bool operator<=(const char *s) const;
    inline CX_ASCII_CAST_WARN bool operator>=(const char *s) const;

    inline CX_ASCII_CAST_WARN bool operator==(const CXByteArray &s) const;
    inline CX_ASCII_CAST_WARN bool operator!=(const CXByteArray &s) const;
    inline CX_ASCII_CAST_WARN bool operator<(const CXByteArray &s) const;
    inline CX_ASCII_CAST_WARN bool operator>(const CXByteArray &s) const;
    inline CX_ASCII_CAST_WARN bool operator<=(const CXByteArray &s) const;
    inline CX_ASCII_CAST_WARN bool operator>=(const CXByteArray &s) const;
#endif

private:
    int m_size;
    const char *m_data;
};
CX_DECLARE_TYPEINFO(CXLatin1String, CX_MOVABLE_TYPE);

typedef CXLatin1String CXLatin1Literal;

//
// CXLatin1String inline implementations
//
C_DECL_CONSTEXPR bool CXPrivate::isLatin1(CXLatin1String) noexcept
{ return true; }

//
// CXStringView members that require CXLatin1String:
//
int CXStringView::compare(CXLatin1String s, cx::CaseSensitivity cs) const noexcept
{ return CXPrivate::compareStrings(*this, s, cs); }
bool CXStringView::startsWith(CXLatin1String s, cx::CaseSensitivity cs) const noexcept
{ return CXPrivate::startsWith(*this, s, cs); }
bool CXStringView::endsWith(CXLatin1String s, cx::CaseSensitivity cs) const noexcept
{ return CXPrivate::endsWith(*this, s, cs); }
qsizetype CXStringView::indexOf(CXLatin1String s, qsizetype from, cx::CaseSensitivity cs) const noexcept
{ return CXPrivate::findString(*this, from, s, cs); }
bool CXStringView::contains(CXLatin1String s, cx::CaseSensitivity cs) const noexcept
{ return indexOf(s, 0, cs) != qsizetype(-1); }
qsizetype CXStringView::lastIndexOf(CXLatin1String s, qsizetype from, cx::CaseSensitivity cs) const noexcept
{ return CXPrivate::lastIndexOf(*this, from, s, cs); }

class C_SYMBOL_EXPORT CXString
{
public:
    typedef CXStringData Data;

    inline CXString() noexcept;
    explicit CXString(const CXChar *unicode, int size = -1);
    CXString(CXChar c);
    CXString(int size, CXChar c);
    inline CXString(CXLatin1String latin1);
    inline CXString(const CXString &) noexcept;
    inline ~CXString();
    CXString &operator=(CXChar c);
    CXString &operator=(const CXString &) noexcept;
    CXString &operator=(CXLatin1String latin1);
    inline CXString(CXString && other) noexcept : d(other.d) { other.d = Data::sharedNull(); }
    inline CXString &operator=(CXString &&other) noexcept
    { C_SWAP(d, other.d); return *this; }
    inline void swap(CXString &other) noexcept { C_SWAP(d, other.d); }
    inline int size() const { return d->size; }
    inline int count() const { return d->size; }
    inline int length() const;
    inline bool isEmpty() const;
    void resize(int size);
    void resize(int size, CXChar fillChar);

    CXString &fill(CXChar c, int size = -1);
    void truncate(int pos);
    void chop(int n);

    int capacity() const;
    inline void reserve(int size);
    inline void squeeze();

    inline const CXChar *unicode() const;
    inline CXChar *data();
    inline const CXChar *data() const;
    inline const CXChar *constData() const;

    inline void detach();
    inline bool isDetached() const;
    inline bool isSharedWith(const CXString &other) const { return d == other.d; }
    void clear();

    inline const CXChar at(int i) const;
    const CXChar operator[](int i) const;
    C_REQUIRED_RESULT CXCharRef operator[](int i);
    const CXChar operator[](cuint i) const;
    C_REQUIRED_RESULT CXCharRef operator[](cuint i);

    C_REQUIRED_RESULT inline CXChar front() const { return at(0); }
    C_REQUIRED_RESULT inline CXCharRef front();
    C_REQUIRED_RESULT inline CXChar back() const { return at(size() - 1); }
    C_REQUIRED_RESULT inline CXCharRef back();

    C_REQUIRED_RESULT CXString arg(clonglong a, int fieldwidth=0, int base=10,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(culonglong a, int fieldwidth=0, int base=10,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(clong a, int fieldwidth=0, int base=10,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(culong a, int fieldwidth=0, int base=10,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(cint a, int fieldWidth = 0, int base = 10,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(cuint a, int fieldWidth = 0, int base = 10,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(cshort a, int fieldWidth = 0, int base = 10,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(cushort a, int fieldWidth = 0, int base = 10,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(cdouble a, int fieldWidth = 0, char fmt = 'g', int prec = -1,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(cchar a, int fieldWidth = 0,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(CXChar a, int fieldWidth = 0,
                CXChar fillChar = CXLatin1Char(' ')) const;
#if QT_STRINGVIEW_LEVEL < 2
    C_REQUIRED_RESULT CXString arg(const CXString &a, int fieldWidth = 0,
                CXChar fillChar = CXLatin1Char(' ')) const;
#endif
    C_REQUIRED_RESULT CXString arg(CXStringView a, int fieldWidth = 0,
                CXChar fillChar = CXLatin1Char(' ')) const;
    C_REQUIRED_RESULT CXString arg(CXLatin1String a, int fieldWidth = 0,
                CXChar fillChar = CXLatin1Char(' ')) const;
#if QT_STRINGVIEW_LEVEL < 2
    C_REQUIRED_RESULT CXString arg(const CXString &a1, const CXString &a2) const;
    C_REQUIRED_RESULT CXString arg(const CXString &a1, const CXString &a2, const CXString &a3) const;
    C_REQUIRED_RESULT CXString arg(const CXString &a1, const CXString &a2, const CXString &a3,
                const CXString &a4) const;
    C_REQUIRED_RESULT CXString arg(const CXString &a1, const CXString &a2, const CXString &a3,
                const CXString &a4, const CXString &a5) const;
    C_REQUIRED_RESULT CXString arg(const CXString &a1, const CXString &a2, const CXString &a3,
                const CXString &a4, const CXString &a5, const CXString &a6) const;
    C_REQUIRED_RESULT CXString arg(const CXString &a1, const CXString &a2, const CXString &a3,
                const CXString &a4, const CXString &a5, const CXString &a6,
                const CXString &a7) const;
    C_REQUIRED_RESULT CXString arg(const CXString &a1, const CXString &a2, const CXString &a3,
                const CXString &a4, const CXString &a5, const CXString &a6,
                const CXString &a7, const CXString &a8) const;
    C_REQUIRED_RESULT CXString arg(const CXString &a1, const CXString &a2, const CXString &a3,
                const CXString &a4, const CXString &a5, const CXString &a6,
                const CXString &a7, const CXString &a8, const CXString &a9) const;
#endif
private:
    template <typename T>
    struct is_convertible_to_view_or_qstring
        : std::integral_constant<bool,
            std::is_convertible<T, CXString>::value ||
            std::is_convertible<T, CXStringView>::value ||
            std::is_convertible<T, CXLatin1String>::value> {};
public:
    template <typename...Args>
    C_REQUIRED_RESULT
#ifdef Q_CLANG_QDOC
    CXString
#else
    typename std::enable_if<
        sizeof...(Args) >= 2 && std::is_same<
            CXPrivate::BoolList<is_convertible_to_view_or_qstring<Args>::value..., true>,
            CXPrivate::BoolList<true, is_convertible_to_view_or_qstring<Args>::value...>
        >::value,
        CXString
    >::type
#endif
    arg(Args &&...args) const
    { return toStringViewIgnoringNull(*this).arg(std::forward<Args>(args)...); }

#if QT_DEPRECATED_SINCE(5, 14)
    QT_DEPRECATED_X("Use vasprintf(), arg() or QTextStream instead")
    CXString &vsprintf(const char *format, va_list ap) Q_ATTRIBUTE_FORMAT_PRINTF(2, 0);
    QT_DEPRECATED_X("Use asprintf(), arg() or QTextStream instead")
    CXString &sprintf(const char *format, ...) Q_ATTRIBUTE_FORMAT_PRINTF(2, 3);
#endif
    static CXString vasprintf(const char *format, va_list ap);
    static CXString asprintf(const char *format, ...);

    int indexOf(CXChar c, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int indexOf(CXLatin1String s, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#if QT_STRINGVIEW_LEVEL < 2
    int indexOf(const CXString &s, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int indexOf(const CXStringRef &s, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif
    C_REQUIRED_RESULT int indexOf(CXStringView s, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::findString(*this, from, s, cs)); } // ### Qt6: qsizetype
    int lastIndexOf(CXChar c, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int lastIndexOf(CXLatin1String s, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#if QT_STRINGVIEW_LEVEL < 2
    int lastIndexOf(const CXString &s, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int lastIndexOf(const CXStringRef &s, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif

    C_REQUIRED_RESULT int lastIndexOf(CXStringView s, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::lastIndexOf(*this, from, s, cs)); } // ### Qt6: qsizetype

    inline bool contains(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#if QT_STRINGVIEW_LEVEL < 2
    inline bool contains(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    inline bool contains(const CXStringRef &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif
    inline bool contains(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    inline bool contains(CXStringView s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;
    int count(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int count(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int count(const CXStringRef &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;

    int indexOf(const CXRegExp &, int from = 0) const;
    int lastIndexOf(const CXRegExp &, int from = -1) const;
    inline bool contains(const CXRegExp &rx) const { return indexOf(rx) != -1; }
    int count(const CXRegExp &) const;

    int indexOf(CXRegExp &, int from = 0) const;
    int lastIndexOf(CXRegExp &, int from = -1) const;
    inline bool contains(CXRegExp &rx) const { return indexOf(rx) != -1; }

#if CX_CONFIG(regularexpression)
    int indexOf(const QRegularExpression &re, int from = 0) const;
    int indexOf(const QRegularExpression &re, int from, QRegularExpressionMatch *rmatch) const; // ### Qt 6: merge overloads
    int lastIndexOf(const QRegularExpression &re, int from = -1) const;
    int lastIndexOf(const QRegularExpression &re, int from, QRegularExpressionMatch *rmatch) const; // ### Qt 6: merge overloads
    bool contains(const QRegularExpression &re) const;
    bool contains(const QRegularExpression &re, QRegularExpressionMatch *rmatch) const; // ### Qt 6: merge overloads
    int count(const QRegularExpression &re) const;
#endif

    enum SectionFlag {
        SectionDefault             = 0x00,
        SectionSkipEmpty           = 0x01,
        SectionIncludeLeadingSep   = 0x02,
        SectionIncludeTrailingSep  = 0x04,
        SectionCaseInsensitiveSeps = 0x08
    };
    CX_DECLARE_FLAGS(SectionFlags, SectionFlag)

    CXString section(CXChar sep, int start, int end = -1, SectionFlags flags = SectionDefault) const;
    CXString section(const CXString &in_sep, int start, int end = -1, SectionFlags flags = SectionDefault) const;
    CXString section(const CXRegExp &reg, int start, int end = -1, SectionFlags flags = SectionDefault) const;
#if CX_CONFIG(regularexpression)
    CXString section(const QRegularExpression &re, int start, int end = -1, SectionFlags flags = SectionDefault) const;
#endif
    C_REQUIRED_RESULT CXString left(int n) const;
    C_REQUIRED_RESULT CXString right(int n) const;
    C_REQUIRED_RESULT CXString mid(int position, int n = -1) const;
    C_REQUIRED_RESULT CXString chopped(int n) const
    { C_ASSERT(n >= 0); C_ASSERT(n <= size()); return left(size() - n); }


    C_REQUIRED_RESULT CXStringRef leftRef(int n) const;
    C_REQUIRED_RESULT CXStringRef rightRef(int n) const;
    C_REQUIRED_RESULT CXStringRef midRef(int position, int n = -1) const;

#if QT_STRINGVIEW_LEVEL < 2
    bool startsWith(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    bool startsWith(const CXStringRef &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif
    C_REQUIRED_RESULT bool startsWith(CXStringView s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::startsWith(*this, s, cs); }
    bool startsWith(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    bool startsWith(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive) const;

#if QT_STRINGVIEW_LEVEL < 2
    bool endsWith(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    bool endsWith(const CXStringRef &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif
    C_REQUIRED_RESULT bool endsWith(CXStringView s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::endsWith(*this, s, cs); }
    bool endsWith(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    bool endsWith(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive) const;

    bool isUpper() const;
    bool isLower() const;

    C_REQUIRED_RESULT CXString leftJustified(int width, CXChar fill = CXLatin1Char(' '), bool trunc = false) const;
    C_REQUIRED_RESULT CXString rightJustified(int width, CXChar fill = CXLatin1Char(' '), bool trunc = false) const;

#if defined(Q_COMPILER_REF_QUALIFIERS) && !defined(QT_COMPILING_QSTRING_COMPAT_CPP) && !defined(Q_CLANG_QDOC)
#  if defined(Q_CC_GNU) && !defined(Q_CC_CLANG) && !defined(Q_CC_INTEL) && !__has_cpp_attribute(nodiscard)
    // required due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61941
#    pragma push_macro("C_REQUIRED_RESULT")
#    undef C_REQUIRED_RESULT
#    define C_REQUIRED_RESULT
#    define C_REQUIRED_RESULT_pushed
#  endif
    C_REQUIRED_RESULT CXString toLower() const &
    { return toLower_helper(*this); }
    C_REQUIRED_RESULT CXString toLower() &&
    { return toLower_helper(*this); }
    C_REQUIRED_RESULT CXString toUpper() const &
    { return toUpper_helper(*this); }
    C_REQUIRED_RESULT CXString toUpper() &&
    { return toUpper_helper(*this); }
    C_REQUIRED_RESULT CXString toCaseFolded() const &
    { return toCaseFolded_helper(*this); }
    C_REQUIRED_RESULT CXString toCaseFolded() &&
    { return toCaseFolded_helper(*this); }
    C_REQUIRED_RESULT CXString trimmed() const &
    { return trimmed_helper(*this); }
    C_REQUIRED_RESULT CXString trimmed() &&
    { return trimmed_helper(*this); }
    C_REQUIRED_RESULT CXString simplified() const &
    { return simplified_helper(*this); }
    C_REQUIRED_RESULT CXString simplified() &&
    { return simplified_helper(*this); }
#  ifdef C_REQUIRED_RESULT_pushed
#    pragma pop_macro("C_REQUIRED_RESULT")
#  endif
#else
    C_REQUIRED_RESULT CXString toLower() const;
    C_REQUIRED_RESULT CXString toUpper() const;
    C_REQUIRED_RESULT CXString toCaseFolded() const;
    C_REQUIRED_RESULT CXString trimmed() const;
    C_REQUIRED_RESULT CXString simplified() const;
#endif
    C_REQUIRED_RESULT CXString toHtmlEscaped() const;

    CXString &insert(int i, CXChar c);
    CXString &insert(int i, const CXChar *uc, int len);
    inline CXString &insert(int i, const CXString &s) { return insert(i, s.constData(), s.length()); }
    inline CXString &insert(int i, const CXStringRef &s);
    inline CXString &insert(int i, CXStringView s)
    { return insert(i, s.data(), s.length()); }
    CXString &insert(int i, CXLatin1String s);
    CXString &append(CXChar c);
    CXString &append(const CXChar *uc, int len);
    CXString &append(const CXString &s);
    CXString &append(const CXStringRef &s);
    CXString &append(CXLatin1String s);
    inline CXString &append(CXStringView s) { return append(s.data(), s.length()); }
    inline CXString &prepend(CXChar c) { return insert(0, c); }
    inline CXString &prepend(const CXChar *uc, int len) { return insert(0, uc, len); }
    inline CXString &prepend(const CXString &s) { return insert(0, s); }
    inline CXString &prepend(const CXStringRef &s) { return insert(0, s); }
    inline CXString &prepend(CXLatin1String s) { return insert(0, s); }
    inline CXString &prepend(CXStringView s) { return insert(0, s); }

    inline CXString &operator+=(CXChar c) {
        if (d->ref.isShared() || uint(d->size) + 2u > d->alloc)
            reallocData(uint(d->size) + 2u, true);
        d->data()[d->size++] = c.unicode();
        d->data()[d->size] = '\0';
        return *this;
    }

    inline CXString &operator+=(CXChar::SpecialCharacter c) { return append(CXChar(c)); }
    inline CXString &operator+=(const CXString &s) { return append(s); }
    inline CXString &operator+=(const CXStringRef &s) { return append(s); }
    inline CXString &operator+=(CXLatin1String s) { return append(s); }
    inline CXString &operator+=(CXStringView s) { return append(s); }

    CXString &remove(int i, int len);
    CXString &remove(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &remove(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &remove(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &replace(int i, int len, CXChar after);
    CXString &replace(int i, int len, const CXChar *s, int slen);
    CXString &replace(int i, int len, const CXString &after);
    CXString &replace(CXChar before, CXChar after, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &replace(const CXChar *before, int blen, const CXChar *after, int alen, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &replace(CXLatin1String before, CXLatin1String after, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &replace(CXLatin1String before, const CXString &after, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &replace(const CXString &before, CXLatin1String after, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &replace(const CXString &before, const CXString &after,
                     cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &replace(CXChar c, const CXString &after, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &replace(CXChar c, CXLatin1String after, cx::CaseSensitivity cs = cx::CaseSensitive);
    CXString &replace(const CXRegExp &rx, const CXString &after);
    inline CXString &remove(const CXRegExp &rx)
    { return replace(rx, CXString()); }

    enum SplitBehavior
    {
        KeepEmptyParts ,
        SkipEmptyParts
    };

    CXStringList split(const CXString &sep, SplitBehavior behavior,
                                        cx::CaseSensitivity cs = cx::CaseSensitive) const;
    CXVector<CXStringRef> splitRef(const CXString &sep, SplitBehavior behavior,
                                                   cx::CaseSensitivity cs = cx::CaseSensitive) const;
    CXStringList split(CXChar sep, SplitBehavior behavior,
                                        cx::CaseSensitivity cs = cx::CaseSensitive) const;
    CXVector<CXStringRef> splitRef(CXChar sep, SplitBehavior behavior,
                                                   cx::CaseSensitivity cs = cx::CaseSensitive) const;
    CXStringList split(const QRegExp &sep, SplitBehavior behavior) const;
    CXVector<CXStringRef> splitRef(const QRegExp &sep, SplitBehavior behavior) const;

public:
    C_REQUIRED_RESULT
    CXStringList split(const CXString &sep, cx::SplitBehavior behavior = cx::KeepEmptyParts,
                      cx::CaseSensitivity cs = cx::CaseSensitive) const;
    C_REQUIRED_RESULT
    CXVector<CXStringRef> splitRef(const CXString &sep,
                                 cx::SplitBehavior behavior = cx::KeepEmptyParts,
                                 cx::CaseSensitivity cs = cx::CaseSensitive) const;
    C_REQUIRED_RESULT
    CXStringList split(CXChar sep, cx::SplitBehavior behavior = cx::KeepEmptyParts,
                      cx::CaseSensitivity cs = cx::CaseSensitive) const;
    C_REQUIRED_RESULT
    CXVector<CXStringRef> splitRef(CXChar sep, cx::SplitBehavior behavior = cx::KeepEmptyParts,
                                 cx::CaseSensitivity cs = cx::CaseSensitive) const;
#ifndef QT_NO_REGEXP
    C_REQUIRED_RESULT
    CXStringList split(const QRegExp &sep,
                      cx::SplitBehavior behavior = cx::KeepEmptyParts) const;
    C_REQUIRED_RESULT
    CXVector<CXStringRef> splitRef(const QRegExp &sep,
                                 cx::SplitBehavior behavior = cx::KeepEmptyParts) const;
#endif
#ifndef QT_NO_REGULAREXPRESSION
    C_REQUIRED_RESULT
    CXStringList split(const QRegularExpression &sep,
                      cx::SplitBehavior behavior = cx::KeepEmptyParts) const;
    C_REQUIRED_RESULT
    CXVector<CXStringRef> splitRef(const QRegularExpression &sep,
                                 cx::SplitBehavior behavior = cx::KeepEmptyParts) const;
#endif


    enum NormalizationForm {
        NormalizationForm_D,
        NormalizationForm_C,
        NormalizationForm_KD,
        NormalizationForm_KC
    };
    C_REQUIRED_RESULT CXString normalized(NormalizationForm mode, CXChar::UnicodeVersion version = CXChar::Unicode_Unassigned) const;

    C_REQUIRED_RESULT CXString repeated(int times) const;

    const ushort *utf16() const;

#if defined(Q_COMPILER_REF_QUALIFIERS) && !defined(QT_COMPILING_QSTRING_COMPAT_CPP) && !defined(Q_CLANG_QDOC)
    C_REQUIRED_RESULT CXByteArray toLatin1() const &
    { return toLatin1_helper(*this); }
    C_REQUIRED_RESULT CXByteArray toLatin1() &&
    { return toLatin1_helper_inplace(*this); }
    C_REQUIRED_RESULT CXByteArray toUtf8() const &
    { return toUtf8_helper(*this); }
    C_REQUIRED_RESULT CXByteArray toUtf8() &&
    { return toUtf8_helper(*this); }
    C_REQUIRED_RESULT CXByteArray toLocal8Bit() const &
    { return toLocal8Bit_helper(isNull() ? nullptr : constData(), size()); }
    C_REQUIRED_RESULT CXByteArray toLocal8Bit() &&
    { return toLocal8Bit_helper(isNull() ? nullptr : constData(), size()); }
#else
    C_REQUIRED_RESULT CXByteArray toLatin1() const;
    C_REQUIRED_RESULT CXByteArray toUtf8() const;
    C_REQUIRED_RESULT CXByteArray toLocal8Bit() const;
#endif
    C_REQUIRED_RESULT CXVector<uint> toUcs4() const;

    // note - this are all inline so we can benefit from strlen() compile time optimizations
    static inline CXString fromLatin1(const char *str, int size = -1)
    {
        CXStringDataPtr dataPtr = { fromLatin1_helper(str, (str && size == -1) ? int(strlen(str)) : size) };
        return CXString(dataPtr);
    }
    static inline CXString fromUtf8(const char *str, int size = -1)
    {
        return fromUtf8_helper(str, (str && size == -1) ? int(strlen(str)) : size);
    }
    static inline CXString fromLocal8Bit(const char *str, int size = -1)
    {
        return fromLocal8Bit_helper(str, (str && size == -1) ? int(strlen(str)) : size);
    }
    static inline CXString fromLatin1(const CXByteArray &str)
    { return str.isNull() ? CXString() : fromLatin1(str.data(), qstrnlen(str.constData(), str.size())); }
    static inline CXString fromUtf8(const CXByteArray &str)
    { return str.isNull() ? CXString() : fromUtf8(str.data(), qstrnlen(str.constData(), str.size())); }
    static inline CXString fromLocal8Bit(const CXByteArray &str)
    { return str.isNull() ? CXString() : fromLocal8Bit(str.data(), qstrnlen(str.constData(), str.size())); }
    static CXString fromUtf16(const ushort *, int size = -1);
    static CXString fromUcs4(const uint *, int size = -1);
    static CXString fromRawData(const CXChar *, int size);

#if defined(Q_COMPILER_UNICODE_STRINGS)
    static CXString fromUtf16(const char16_t *str, int size = -1)
    { return fromUtf16(reinterpret_cast<const ushort *>(str), size); }
    static CXString fromUcs4(const char32_t *str, int size = -1)
    { return fromUcs4(reinterpret_cast<const uint *>(str), size); }
#endif

#if QT_DEPRECATED_SINCE(5, 0)
    QT_DEPRECATED static inline CXString fromAscii(const char *str, int size = -1)
    { return fromLatin1(str, size); }
    QT_DEPRECATED static inline CXString fromAscii(const CXByteArray &str)
    { return fromLatin1(str); }
    C_REQUIRED_RESULT CXByteArray toAscii() const
    { return toLatin1(); }
#endif

    inline int toWCharArray(wchar_t *array) const;
    C_REQUIRED_RESULT static inline CXString fromWCharArray(const wchar_t *string, int size = -1);

    CXString &setRawData(const CXChar *unicode, int size);
    CXString &setUnicode(const CXChar *unicode, int size);
    inline CXString &setUtf16(const ushort *utf16, int size);

#if QT_STRINGVIEW_LEVEL < 2
    int compare(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;
    inline int compare(const CXStringRef &s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;
#endif
    int compare(CXLatin1String other, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;
    inline int compare(CXStringView s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;
    int compare(CXChar ch, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return compare(CXStringView{&ch, 1}, cs); }

    static inline int compare(const CXString &s1, const CXString &s2,
                              cx::CaseSensitivity cs = cx::CaseSensitive) noexcept
    { return s1.compare(s2, cs); }

    static inline int compare(const CXString &s1, CXLatin1String s2,
                              cx::CaseSensitivity cs = cx::CaseSensitive) noexcept
    { return s1.compare(s2, cs); }
    static inline int compare(CXLatin1String s1, const CXString &s2,
                              cx::CaseSensitivity cs = cx::CaseSensitive) noexcept
    { return -s2.compare(s1, cs); }

    static int compare(const CXString &s1, const CXStringRef &s2,
                       cx::CaseSensitivity = cx::CaseSensitive) noexcept;

    int localeAwareCompare(const CXString& s) const;
    static int localeAwareCompare(const CXString& s1, const CXString& s2)
    { return s1.localeAwareCompare(s2); }

    int localeAwareCompare(const CXStringRef &s) const;
    static int localeAwareCompare(const CXString& s1, const CXStringRef& s2);

    // ### Qt6: make inline except for the long long versions
    short  toShort(bool *ok=nullptr, int base=10) const;
    ushort toUShort(bool *ok=nullptr, int base=10) const;
    int toInt(bool *ok=nullptr, int base=10) const;
    uint toUInt(bool *ok=nullptr, int base=10) const;
    long toLong(bool *ok=nullptr, int base=10) const;
    ulong toULong(bool *ok=nullptr, int base=10) const;
    qlonglong toLongLong(bool *ok=nullptr, int base=10) const;
    qulonglong toULongLong(bool *ok=nullptr, int base=10) const;
    float toFloat(bool *ok=nullptr) const;
    double toDouble(bool *ok=nullptr) const;

    CXString &setNum(short, int base=10);
    CXString &setNum(ushort, int base=10);
    CXString &setNum(int, int base=10);
    CXString &setNum(uint, int base=10);
    CXString &setNum(long, int base=10);
    CXString &setNum(ulong, int base=10);
    CXString &setNum(qlonglong, int base=10);
    CXString &setNum(qulonglong, int base=10);
    CXString &setNum(float, char f='g', int prec=6);
    CXString &setNum(double, char f='g', int prec=6);

    static CXString number(int, int base=10);
    static CXString number(uint, int base=10);
    static CXString number(long, int base=10);
    static CXString number(ulong, int base=10);
    static CXString number(qlonglong, int base=10);
    static CXString number(qulonglong, int base=10);
    static CXString number(double, char f='g', int prec=6);

    friend Q_CORE_EXPORT bool operator==(const CXString &s1, const CXString &s2) noexcept;
    friend Q_CORE_EXPORT bool operator<(const CXString &s1, const CXString &s2) noexcept;
    friend inline bool operator>(const CXString &s1, const CXString &s2) noexcept { return s2 < s1; }
    friend inline bool operator!=(const CXString &s1, const CXString &s2) noexcept { return !(s1 == s2); }
    friend inline bool operator<=(const CXString &s1, const CXString &s2) noexcept { return !(s1 > s2); }
    friend inline bool operator>=(const CXString &s1, const CXString &s2) noexcept { return !(s1 < s2); }

    bool operator==(CXLatin1String s) const noexcept;
    bool operator<(CXLatin1String s) const noexcept;
    bool operator>(CXLatin1String s) const noexcept;
    inline bool operator!=(CXLatin1String s) const noexcept { return !operator==(s); }
    inline bool operator<=(CXLatin1String s) const noexcept { return !operator>(s); }
    inline bool operator>=(CXLatin1String s) const noexcept { return !operator<(s); }

    // ASCII compatibility
#if defined(QT_RESTRICTED_CAST_FROM_ASCII)
    template <int N>
    inline CXString(const char (&ch)[N])
        : d(fromAscii_helper(ch, N - 1))
    {}
    template <int N>
    CXString(char (&)[N]) = delete;
    template <int N>
    inline CXString &operator=(const char (&ch)[N])
    { return (*this = fromUtf8(ch, N - 1)); }
    template <int N>
    CXString &operator=(char (&)[N]) = delete;
#endif
#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
    inline QT_ASCII_CAST_WARN CXString(const char *ch)
        : d(fromAscii_helper(ch, ch ? int(strlen(ch)) : -1))
    {}
    inline QT_ASCII_CAST_WARN CXString(const CXByteArray &a)
        : d(fromAscii_helper(a.constData(), qstrnlen(a.constData(), a.size())))
    {}
    inline QT_ASCII_CAST_WARN CXString &operator=(const char *ch)
    { return (*this = fromUtf8(ch)); }
    inline QT_ASCII_CAST_WARN CXString &operator=(const CXByteArray &a)
    { return (*this = fromUtf8(a)); }
    inline QT_ASCII_CAST_WARN CXString &operator=(char c)
    { return (*this = CXChar::fromLatin1(c)); }

    // these are needed, so it compiles with STL support enabled
    inline QT_ASCII_CAST_WARN CXString &prepend(const char *s)
    { return prepend(CXString::fromUtf8(s)); }
    inline QT_ASCII_CAST_WARN CXString &prepend(const CXByteArray &s)
    { return prepend(CXString::fromUtf8(s)); }
    inline QT_ASCII_CAST_WARN CXString &append(const char *s)
    { return append(CXString::fromUtf8(s)); }
    inline QT_ASCII_CAST_WARN CXString &append(const CXByteArray &s)
    { return append(CXString::fromUtf8(s)); }
    inline QT_ASCII_CAST_WARN CXString &insert(int i, const char *s)
    { return insert(i, CXString::fromUtf8(s)); }
    inline QT_ASCII_CAST_WARN CXString &insert(int i, const CXByteArray &s)
    { return insert(i, CXString::fromUtf8(s)); }
    inline QT_ASCII_CAST_WARN CXString &operator+=(const char *s)
    { return append(CXString::fromUtf8(s)); }
    inline QT_ASCII_CAST_WARN CXString &operator+=(const CXByteArray &s)
    { return append(CXString::fromUtf8(s)); }
    inline QT_ASCII_CAST_WARN CXString &operator+=(char c)
    { return append(CXChar::fromLatin1(c)); }

    inline QT_ASCII_CAST_WARN bool operator==(const char *s) const;
    inline QT_ASCII_CAST_WARN bool operator!=(const char *s) const;
    inline QT_ASCII_CAST_WARN bool operator<(const char *s) const;
    inline QT_ASCII_CAST_WARN bool operator<=(const char *s) const;
    inline QT_ASCII_CAST_WARN bool operator>(const char *s) const;
    inline QT_ASCII_CAST_WARN bool operator>=(const char *s) const;

    inline QT_ASCII_CAST_WARN bool operator==(const CXByteArray &s) const;
    inline QT_ASCII_CAST_WARN bool operator!=(const CXByteArray &s) const;
    inline QT_ASCII_CAST_WARN bool operator<(const CXByteArray &s) const;
    inline QT_ASCII_CAST_WARN bool operator>(const CXByteArray &s) const;
    inline QT_ASCII_CAST_WARN bool operator<=(const CXByteArray &s) const;
    inline QT_ASCII_CAST_WARN bool operator>=(const CXByteArray &s) const;

    friend inline QT_ASCII_CAST_WARN bool operator==(const char *s1, const CXString &s2);
    friend inline QT_ASCII_CAST_WARN bool operator!=(const char *s1, const CXString &s2);
    friend inline QT_ASCII_CAST_WARN bool operator<(const char *s1, const CXString &s2);
    friend inline QT_ASCII_CAST_WARN bool operator>(const char *s1, const CXString &s2);
    friend inline QT_ASCII_CAST_WARN bool operator<=(const char *s1, const CXString &s2);
    friend inline QT_ASCII_CAST_WARN bool operator>=(const char *s1, const CXString &s2);

    friend inline QT_ASCII_CAST_WARN bool operator==(const char *s1, const CXStringRef &s2);
    friend inline QT_ASCII_CAST_WARN bool operator!=(const char *s1, const CXStringRef &s2);
    friend inline QT_ASCII_CAST_WARN bool operator<(const char *s1, const CXStringRef &s2);
    friend inline QT_ASCII_CAST_WARN bool operator>(const char *s1, const CXStringRef &s2);
    friend inline QT_ASCII_CAST_WARN bool operator<=(const char *s1, const CXStringRef &s2);
    friend inline QT_ASCII_CAST_WARN bool operator>=(const char *s1, const CXStringRef &s2);
#endif

    typedef CXChar *iterator;
    typedef const CXChar *const_iterator;
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    inline iterator begin();
    inline const_iterator begin() const;
    inline const_iterator cbegin() const;
    inline const_iterator constBegin() const;
    inline iterator end();
    inline const_iterator end() const;
    inline const_iterator cend() const;
    inline const_iterator constEnd() const;
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    // STL compatibility
    typedef int size_type;
    typedef qptrdiff difference_type;
    typedef const CXChar & const_reference;
    typedef CXChar & reference;
    typedef CXChar *pointer;
    typedef const CXChar *const_pointer;
    typedef CXChar value_type;
    inline void push_back(CXChar c) { append(c); }
    inline void push_back(const CXString &s) { append(s); }
    inline void push_front(CXChar c) { prepend(c); }
    inline void push_front(const CXString &s) { prepend(s); }
    void shrink_to_fit() { squeeze(); }

    static inline CXString fromStdString(const std::string &s);
    inline std::string toStdString() const;
    static inline CXString fromStdWString(const std::wstring &s);
    inline std::wstring toStdWString() const;

#if defined(Q_STDLIB_UNICODE_STRINGS) || defined(Q_QDOC)
    static inline CXString fromStdU16String(const std::u16string &s);
    inline std::u16string toStdU16String() const;
    static inline CXString fromStdU32String(const std::u32string &s);
    inline std::u32string toStdU32String() const;
#endif

#if defined(Q_OS_DARWIN) || defined(Q_QDOC)
    static CXString fromCFString(CFStringRef string);
    CFStringRef toCFString() const Q_DECL_CF_RETURNS_RETAINED;
    static CXString fromNSString(const NSString *string);
    NSString *toNSString() const Q_DECL_NS_RETURNS_AUTORELEASED;
#endif
    // compatibility
#if QT_DEPRECATED_SINCE(5, 9)
    struct Null { };
    QT_DEPRECATED_X("use CXString()")
    static const Null null;
    inline CXString(const Null &): d(Data::sharedNull()) {}
    inline CXString &operator=(const Null &) { *this = CXString(); return *this; }
#endif
    inline bool isNull() const { return d == Data::sharedNull(); }


    bool isSimpleText() const;
    bool isRightToLeft() const;
    C_REQUIRED_RESULT bool isValidUtf16() const noexcept
    { return CXStringView(*this).isValidUtf16(); }

    CXString(int size, cx::Initialization);
    C_DECL_CONSTEXPR inline CXString(CXStringDataPtr dd) : d(dd.ptr) {}

private:
#if defined(QT_NO_CAST_FROM_ASCII)
    CXString &operator+=(const char *s);
    CXString &operator+=(const CXByteArray &s);
    CXString(const char *ch);
    CXString(const CXByteArray &a);
    CXString &operator=(const char  *ch);
    CXString &operator=(const CXByteArray &a);
#endif

    Data *d;

    friend inline bool operator==(CXChar, const CXString &) noexcept;
    friend inline bool operator< (CXChar, const CXString &) noexcept;
    friend inline bool operator> (CXChar, const CXString &) noexcept;
    friend inline bool operator==(CXChar, const CXStringRef &) noexcept;
    friend inline bool operator< (CXChar, const CXStringRef &) noexcept;
    friend inline bool operator> (CXChar, const CXStringRef &) noexcept;
    friend inline bool operator==(CXChar, CXLatin1String) noexcept;
    friend inline bool operator< (CXChar, CXLatin1String) noexcept;
    friend inline bool operator> (CXChar, CXLatin1String) noexcept;

    void reallocData(uint alloc, bool grow = false);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void expand(int i);
    CXString multiArg(int numArgs, const CXString **args) const;
#endif
    static int compare_helper(const CXChar *data1, int length1,
                              const CXChar *data2, int length2,
                              cx::CaseSensitivity cs = cx::CaseSensitive) noexcept;
    static int compare_helper(const CXChar *data1, int length1,
                              const char *data2, int length2,
                              cx::CaseSensitivity cs = cx::CaseSensitive);
    static int compare_helper(const CXChar *data1, int length1,
                              CXLatin1String s2,
                              cx::CaseSensitivity cs = cx::CaseSensitive) noexcept;
    static int localeAwareCompare_helper(const CXChar *data1, int length1,
                                         const CXChar *data2, int length2);
    static CXString toLower_helper(const CXString &str);
    static CXString toLower_helper(CXString &str);
    static CXString toUpper_helper(const CXString &str);
    static CXString toUpper_helper(CXString &str);
    static CXString toCaseFolded_helper(const CXString &str);
    static CXString toCaseFolded_helper(CXString &str);
    static CXString trimmed_helper(const CXString &str);
    static CXString trimmed_helper(CXString &str);
    static CXString simplified_helper(const CXString &str);
    static CXString simplified_helper(CXString &str);
    static Data *fromLatin1_helper(const char *str, int size = -1);
    static Data *fromAscii_helper(const char *str, int size = -1);
    static CXString fromUtf8_helper(const char *str, int size);
    static CXString fromLocal8Bit_helper(const char *, int size);
    static CXByteArray toLatin1_helper(const CXString &);
    static CXByteArray toLatin1_helper_inplace(CXString &);
    static CXByteArray toUtf8_helper(const CXString &);
    static CXByteArray toLocal8Bit_helper(const CXChar *data, int size);
    static int toUcs4_helper(const ushort *uc, int length, uint *out);
    static qlonglong toIntegral_helper(const CXChar *data, int len, bool *ok, int base);
    static qulonglong toIntegral_helper(const CXChar *data, uint len, bool *ok, int base);
    void replace_helper(uint *indices, int nIndices, int blen, const CXChar *after, int alen);
    friend class CXCharRef;
    friend class QTextCodec;
    friend class CXStringRef;
    friend class CXStringView;
    friend class CXByteArray;
    friend class QCollator;
    friend struct QAbstractConcatenable;

    template <typename T> static
    T toIntegral_helper(const CXChar *data, int len, bool *ok, int base)
    {
        using Int64 = typename std::conditional<std::is_unsigned<T>::value, qulonglong, qlonglong>::type;
        using Int32 = typename std::conditional<std::is_unsigned<T>::value, uint, int>::type;

        // we select the right overload by casting size() to int or uint
        Int64 val = toIntegral_helper(data, Int32(len), ok, base);
        if (T(val) != val) {
            if (ok)
                *ok = false;
            val = 0;
        }
        return T(val);
    }

public:
    typedef Data * DataPtr;
    inline DataPtr &data_ptr() { return d; }
};

//
// CXStringView inline members that require CXString:
//
CXString CXStringView::toString() const
{ return C_ASSERT(size() == length()), CXString(data(), length()); }

//
// CXString inline members
//
inline CXString::CXString(CXLatin1String aLatin1) : d(fromLatin1_helper(aLatin1.latin1(), aLatin1.size()))
{ }
inline int CXString::length() const
{ return d->size; }
inline const CXChar CXString::at(int i) const
{ C_ASSERT(uint(i) < uint(size())); return CXChar(d->data()[i]); }
inline const CXChar CXString::operator[](int i) const
{ C_ASSERT(uint(i) < uint(size())); return CXChar(d->data()[i]); }
inline const CXChar CXString::operator[](uint i) const
{ C_ASSERT(i < uint(size())); return CXChar(d->data()[i]); }
inline bool CXString::isEmpty() const
{ return d->size == 0; }
inline const CXChar *CXString::unicode() const
{ return reinterpret_cast<const CXChar*>(d->data()); }
inline const CXChar *CXString::data() const
{ return reinterpret_cast<const CXChar*>(d->data()); }
inline CXChar *CXString::data()
{ detach(); return reinterpret_cast<CXChar*>(d->data()); }
inline const CXChar *CXString::constData() const
{ return reinterpret_cast<const CXChar*>(d->data()); }
inline void CXString::detach()
{ if (d->ref.isShared() || (d->offset != sizeof(CXStringData))) reallocData(uint(d->size) + 1u); }
inline bool CXString::isDetached() const
{ return !d->ref.isShared(); }
inline void CXString::clear()
{ if (!isNull()) *this = CXString(); }
inline CXString::CXString(const CXString &other) noexcept : d(other.d)
{ C_ASSERT(&other != this); d->ref.ref(); }
inline int CXString::capacity() const
{ return d->alloc ? d->alloc - 1 : 0; }
inline CXString &CXString::setNum(short n, int base)
{ return setNum(qlonglong(n), base); }
inline CXString &CXString::setNum(ushort n, int base)
{ return setNum(qulonglong(n), base); }
inline CXString &CXString::setNum(int n, int base)
{ return setNum(qlonglong(n), base); }
inline CXString &CXString::setNum(uint n, int base)
{ return setNum(qulonglong(n), base); }
inline CXString &CXString::setNum(long n, int base)
{ return setNum(qlonglong(n), base); }
inline CXString &CXString::setNum(ulong n, int base)
{ return setNum(qulonglong(n), base); }
inline CXString &CXString::setNum(float n, char f, int prec)
{ return setNum(double(n),f,prec); }
inline CXString CXString::arg(int a, int fieldWidth, int base, CXChar fillChar) const
{ return arg(qlonglong(a), fieldWidth, base, fillChar); }
inline CXString CXString::arg(uint a, int fieldWidth, int base, CXChar fillChar) const
{ return arg(qulonglong(a), fieldWidth, base, fillChar); }
inline CXString CXString::arg(long a, int fieldWidth, int base, CXChar fillChar) const
{ return arg(qlonglong(a), fieldWidth, base, fillChar); }
inline CXString CXString::arg(ulong a, int fieldWidth, int base, CXChar fillChar) const
{ return arg(qulonglong(a), fieldWidth, base, fillChar); }
inline CXString CXString::arg(short a, int fieldWidth, int base, CXChar fillChar) const
{ return arg(qlonglong(a), fieldWidth, base, fillChar); }
inline CXString CXString::arg(ushort a, int fieldWidth, int base, CXChar fillChar) const
{ return arg(qulonglong(a), fieldWidth, base, fillChar); }
#if QT_STRINGVIEW_LEVEL < 2
inline CXString CXString::arg(const CXString &a1, const CXString &a2) const
{ return qToStringViewIgnoringNull(*this).arg(a1, a2); }
inline CXString CXString::arg(const CXString &a1, const CXString &a2, const CXString &a3) const
{ return qToStringViewIgnoringNull(*this).arg(a1, a2, a3); }
inline CXString CXString::arg(const CXString &a1, const CXString &a2, const CXString &a3,
                            const CXString &a4) const
{ return qToStringViewIgnoringNull(*this).arg(a1, a2, a3, a4); }
inline CXString CXString::arg(const CXString &a1, const CXString &a2, const CXString &a3,
                            const CXString &a4, const CXString &a5) const
{ return qToStringViewIgnoringNull(*this).arg(a1, a2, a3, a4, a5); }
inline CXString CXString::arg(const CXString &a1, const CXString &a2, const CXString &a3,
                            const CXString &a4, const CXString &a5, const CXString &a6) const
{ return qToStringViewIgnoringNull(*this).arg(a1, a2, a3, a4, a5, a6); }
inline CXString CXString::arg(const CXString &a1, const CXString &a2, const CXString &a3,
                            const CXString &a4, const CXString &a5, const CXString &a6,
                            const CXString &a7) const
{ return qToStringViewIgnoringNull(*this).arg(a1, a2, a3, a4, a5, a6, a7); }
inline CXString CXString::arg(const CXString &a1, const CXString &a2, const CXString &a3,
                            const CXString &a4, const CXString &a5, const CXString &a6,
                            const CXString &a7, const CXString &a8) const
{ return qToStringViewIgnoringNull(*this).arg(a1, a2, a3, a4, a5, a6, a7, a8); }
inline CXString CXString::arg(const CXString &a1, const CXString &a2, const CXString &a3,
                            const CXString &a4, const CXString &a5, const CXString &a6,
                            const CXString &a7, const CXString &a8, const CXString &a9) const
{ return qToStringViewIgnoringNull(*this).arg(a1, a2, a3, a4, a5, a6, a7, a8, a9); }
#endif

inline CXString CXString::section(CXChar asep, int astart, int aend, SectionFlags aflags) const
{ return section(CXString(asep), astart, aend, aflags); }

QT_WARNING_PUSH
QT_WARNING_DISABLE_MSVC(4127)   // "conditional expression is constant"
QT_WARNING_DISABLE_INTEL(111)   // "statement is unreachable"

inline int CXString::toWCharArray(wchar_t *array) const
{
    return qToStringViewIgnoringNull(*this).toWCharArray(array);
}

int CXStringView::toWCharArray(wchar_t *array) const
{
    if (sizeof(wchar_t) == sizeof(CXChar)) {
        if (auto src = data())
            memcpy(array, src, sizeof(CXChar) * size());
        return int(size());     // ### q6sizetype
    } else {
        return CXString::toUcs4_helper(reinterpret_cast<const ushort *>(data()), int(size()),
                                      reinterpret_cast<uint *>(array));
    }
}

QT_WARNING_POP

inline CXString CXString::fromWCharArray(const wchar_t *string, int size)
{
    return sizeof(wchar_t) == sizeof(CXChar) ? fromUtf16(reinterpret_cast<const ushort *>(string), size)
                                            : fromUcs4(reinterpret_cast<const uint *>(string), size);
}

class
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_CORE_EXPORT
#endif
CXCharRef { // ### Qt 7: remove
    CXString &s;
    int i;
    inline CXCharRef(CXString &str, int idx)
        : s(str),i(idx) {}
    friend class CXString;
public:
    CXCharRef(const CXCharRef &) = default;

    // most CXChar operations repeated here

    // all this is not documented: We just say "like CXChar" and let it be.
    inline operator CXChar() const
    {
        using namespace CXPrivate::DeprecatedRefClassBehavior;
        if (Q_LIKELY(i < s.d->size))
            return CXChar(s.d->data()[i]);
#ifdef QT_DEBUG
        warn(WarningType::OutOfRange, EmittingClass::CXCharRef);
#endif
        return CXChar();
    }
    inline CXCharRef &operator=(CXChar c)
    {
        using namespace CXPrivate::DeprecatedRefClassBehavior;
        if (Q_UNLIKELY(i >= s.d->size)) {
#ifdef QT_DEBUG
            warn(WarningType::OutOfRange, EmittingClass::CXCharRef);
#endif
            s.resize(i + 1, CXLatin1Char(' '));
        } else {
#ifdef QT_DEBUG
            if (Q_UNLIKELY(!s.isDetached()))
                warn(WarningType::DelayedDetach, EmittingClass::CXCharRef);
#endif
            s.detach();
        }
        s.d->data()[i] = c.unicode();
        return *this;
    }

    // An operator= for each CXChar cast constructors
#ifndef QT_NO_CAST_FROM_ASCII
    inline QT_ASCII_CAST_WARN CXCharRef &operator=(char c)
    { return operator=(CXChar::fromLatin1(c)); }
    inline QT_ASCII_CAST_WARN CXCharRef &operator=(uchar c)
    { return operator=(CXChar::fromLatin1(c)); }
#else
    // prevent char -> int promotion, bypassing QT_NO_CAST_FROM_ASCII
    CXCharRef &operator=(char c) = delete;
    CXCharRef &operator=(uchar c) = delete;
#endif
    inline CXCharRef &operator=(const CXCharRef &c) { return operator=(CXChar(c)); }
    inline CXCharRef &operator=(ushort rc) { return operator=(CXChar(rc)); }
    inline CXCharRef &operator=(short rc) { return operator=(CXChar(rc)); }
    inline CXCharRef &operator=(uint rc) { return operator=(CXChar(rc)); }
    inline CXCharRef &operator=(int rc) { return operator=(CXChar(rc)); }

    // each function...
    inline bool isNull() const { return CXChar(*this).isNull(); }
    inline bool isPrint() const { return CXChar(*this).isPrint(); }
    inline bool isPunct() const { return CXChar(*this).isPunct(); }
    inline bool isSpace() const { return CXChar(*this).isSpace(); }
    inline bool isMark() const { return CXChar(*this).isMark(); }
    inline bool isLetter() const { return CXChar(*this).isLetter(); }
    inline bool isNumber() const { return CXChar(*this).isNumber(); }
    inline bool isLetterOrNumber() { return CXChar(*this).isLetterOrNumber(); }
    inline bool isDigit() const { return CXChar(*this).isDigit(); }
    inline bool isLower() const { return CXChar(*this).isLower(); }
    inline bool isUpper() const { return CXChar(*this).isUpper(); }
    inline bool isTitleCase() const { return CXChar(*this).isTitleCase(); }

    inline int digitValue() const { return CXChar(*this).digitValue(); }
    CXChar toLower() const { return CXChar(*this).toLower(); }
    CXChar toUpper() const { return CXChar(*this).toUpper(); }
    CXChar toTitleCase () const { return CXChar(*this).toTitleCase(); }

    CXChar::Category category() const { return CXChar(*this).category(); }
    CXChar::Direction direction() const { return CXChar(*this).direction(); }
    CXChar::JoiningType joiningType() const { return CXChar(*this).joiningType(); }
#if QT_DEPRECATED_SINCE(5, 3)
    QT_DEPRECATED CXChar::Joining joining() const
    {
        switch (CXChar(*this).joiningType()) {
        case CXChar::Joining_Causing: return CXChar::Center;
        case CXChar::Joining_Dual: return CXChar::Dual;
        case CXChar::Joining_Right: return CXChar::Right;
        case CXChar::Joining_None:
        case CXChar::Joining_Left:
        case CXChar::Joining_Transparent:
        default: return CXChar::OtherJoining;
        }
    }
#endif
    bool hasMirrored() const { return CXChar(*this).hasMirrored(); }
    CXChar mirroredChar() const { return CXChar(*this).mirroredChar(); }
    CXString decomposition() const { return CXChar(*this).decomposition(); }
    CXChar::Decomposition decompositionTag() const { return CXChar(*this).decompositionTag(); }
    uchar combiningClass() const { return CXChar(*this).combiningClass(); }

    inline CXChar::Script script() const { return CXChar(*this).script(); }

    CXChar::UnicodeVersion unicodeVersion() const { return CXChar(*this).unicodeVersion(); }

    inline uchar cell() const { return CXChar(*this).cell(); }
    inline uchar row() const { return CXChar(*this).row(); }
    inline void setCell(uchar cell);
    inline void setRow(uchar row);

#if QT_DEPRECATED_SINCE(5, 0)
    QT_DEPRECATED  char toAscii() const { return CXChar(*this).toLatin1(); }
#endif
    char toLatin1() const { return CXChar(*this).toLatin1(); }
    ushort unicode() const { return CXChar(*this).unicode(); }
    ushort& unicode() { return s.data()[i].unicode(); }

};
Q_DECLARE_TYPEINFO(CXCharRef, Q_MOVABLE_TYPE);

inline void CXCharRef::setRow(uchar arow) { CXChar(*this).setRow(arow); }
inline void CXCharRef::setCell(uchar acell) { CXChar(*this).setCell(acell); }


inline CXString::CXString() noexcept : d(Data::sharedNull()) {}
inline CXString::~CXString() { if (!d->ref.deref()) Data::deallocate(d); }

inline void CXString::reserve(int asize)
{
    if (d->ref.isShared() || uint(asize) >= d->alloc)
        reallocData(qMax(asize, d->size) + 1u);

    if (!d->capacityReserved) {
        // cannot set unconditionally, since d could be the shared_null/shared_empty (which is const)
        d->capacityReserved = true;
    }
}

inline void CXString::squeeze()
{
    if (d->ref.isShared() || uint(d->size) + 1u < d->alloc)
        reallocData(uint(d->size) + 1u);

    if (d->capacityReserved) {
        // cannot set unconditionally, since d could be shared_null or
        // otherwise static.
        d->capacityReserved = false;
    }
}

inline CXString &CXString::setUtf16(const ushort *autf16, int asize)
{ return setUnicode(reinterpret_cast<const CXChar *>(autf16), asize); }
inline CXCharRef CXString::operator[](int i)
{ C_ASSERT(i >= 0); detach(); return CXCharRef(*this, i); }
inline CXCharRef CXString::operator[](uint i)
{  detach(); return CXCharRef(*this, i); }
inline CXCharRef CXString::front() { return operator[](0); }
inline CXCharRef CXString::back() { return operator[](size() - 1); }
inline CXString::iterator CXString::begin()
{ detach(); return reinterpret_cast<CXChar*>(d->data()); }
inline CXString::const_iterator CXString::begin() const
{ return reinterpret_cast<const CXChar*>(d->data()); }
inline CXString::const_iterator CXString::cbegin() const
{ return reinterpret_cast<const CXChar*>(d->data()); }
inline CXString::const_iterator CXString::constBegin() const
{ return reinterpret_cast<const CXChar*>(d->data()); }
inline CXString::iterator CXString::end()
{ detach(); return reinterpret_cast<CXChar*>(d->data() + d->size); }
inline CXString::const_iterator CXString::end() const
{ return reinterpret_cast<const CXChar*>(d->data() + d->size); }
inline CXString::const_iterator CXString::cend() const
{ return reinterpret_cast<const CXChar*>(d->data() + d->size); }
inline CXString::const_iterator CXString::constEnd() const
{ return reinterpret_cast<const CXChar*>(d->data() + d->size); }
#if QT_STRINGVIEW_LEVEL < 2
inline bool CXString::contains(const CXString &s, cx::CaseSensitivity cs) const
{ return indexOf(s, 0, cs) != -1; }
inline bool CXString::contains(const CXStringRef &s, cx::CaseSensitivity cs) const
{ return indexOf(s, 0, cs) != -1; }
#endif
inline bool CXString::contains(CXLatin1String s, cx::CaseSensitivity cs) const
{ return indexOf(s, 0, cs) != -1; }
inline bool CXString::contains(CXChar c, cx::CaseSensitivity cs) const
{ return indexOf(c, 0, cs) != -1; }
inline bool CXString::contains(CXStringView s, cx::CaseSensitivity cs) const noexcept
{ return indexOf(s, 0, cs) != -1; }

#if QT_DEPRECATED_SINCE(5, 9)
inline bool operator==(CXString::Null, CXString::Null) { return true; }
QT_DEPRECATED_X("use CXString::isNull()")
inline bool operator==(CXString::Null, const CXString &s) { return s.isNull(); }
QT_DEPRECATED_X("use CXString::isNull()")
inline bool operator==(const CXString &s, CXString::Null) { return s.isNull(); }
inline bool operator!=(CXString::Null, CXString::Null) { return false; }
QT_DEPRECATED_X("use !CXString::isNull()")
inline bool operator!=(CXString::Null, const CXString &s) { return !s.isNull(); }
QT_DEPRECATED_X("use !CXString::isNull()")
inline bool operator!=(const CXString &s, CXString::Null) { return !s.isNull(); }
#endif

inline bool operator==(CXLatin1String s1, CXLatin1String s2) noexcept
{ return s1.size() == s2.size() && (!s1.size() || !memcmp(s1.latin1(), s2.latin1(), s1.size())); }
inline bool operator!=(CXLatin1String s1, CXLatin1String s2) noexcept
{ return !operator==(s1, s2); }
inline bool operator<(CXLatin1String s1, CXLatin1String s2) noexcept
{
    const int len = qMin(s1.size(), s2.size());
    const int r = len ? memcmp(s1.latin1(), s2.latin1(), len) : 0;
    return r < 0 || (r == 0 && s1.size() < s2.size());
}
inline bool operator>(CXLatin1String s1, CXLatin1String s2) noexcept
{ return operator<(s2, s1); }
inline bool operator<=(CXLatin1String s1, CXLatin1String s2) noexcept
{ return !operator>(s1, s2); }
inline bool operator>=(CXLatin1String s1, CXLatin1String s2) noexcept
{ return !operator<(s1, s2); }

inline bool CXLatin1String::operator==(const CXString &s) const noexcept
{ return s == *this; }
inline bool CXLatin1String::operator!=(const CXString &s) const noexcept
{ return s != *this; }
inline bool CXLatin1String::operator>(const CXString &s) const noexcept
{ return s < *this; }
inline bool CXLatin1String::operator<(const CXString &s) const noexcept
{ return s > *this; }
inline bool CXLatin1String::operator>=(const CXString &s) const noexcept
{ return s <= *this; }
inline bool CXLatin1String::operator<=(const CXString &s) const noexcept
{ return s >= *this; }

#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
inline bool CXString::operator==(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) == 0; }
inline bool CXString::operator!=(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) != 0; }
inline bool CXString::operator<(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) < 0; }
inline bool CXString::operator>(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) > 0; }
inline bool CXString::operator<=(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) <= 0; }
inline bool CXString::operator>=(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) >= 0; }

inline QT_ASCII_CAST_WARN bool operator==(const char *s1, const CXString &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) == 0; }
inline QT_ASCII_CAST_WARN bool operator!=(const char *s1, const CXString &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) != 0; }
inline QT_ASCII_CAST_WARN bool operator<(const char *s1, const CXString &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) > 0; }
inline QT_ASCII_CAST_WARN bool operator>(const char *s1, const CXString &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) < 0; }
inline QT_ASCII_CAST_WARN bool operator<=(const char *s1, const CXString &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) >= 0; }
inline QT_ASCII_CAST_WARN bool operator>=(const char *s1, const CXString &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) <= 0; }

inline QT_ASCII_CAST_WARN bool operator==(const char *s1, CXLatin1String s2)
{ return CXString::fromUtf8(s1) == s2; }
inline QT_ASCII_CAST_WARN bool operator!=(const char *s1, CXLatin1String s2)
{ return CXString::fromUtf8(s1) != s2; }
inline QT_ASCII_CAST_WARN bool operator<(const char *s1, CXLatin1String s2)
{ return (CXString::fromUtf8(s1) < s2); }
inline QT_ASCII_CAST_WARN bool operator>(const char *s1, CXLatin1String s2)
{ return (CXString::fromUtf8(s1) > s2); }
inline QT_ASCII_CAST_WARN bool operator<=(const char *s1, CXLatin1String s2)
{ return (CXString::fromUtf8(s1) <= s2); }
inline QT_ASCII_CAST_WARN bool operator>=(const char *s1, CXLatin1String s2)
{ return (CXString::fromUtf8(s1) >= s2); }

inline QT_ASCII_CAST_WARN bool CXLatin1String::operator==(const char *s) const
{ return CXString::fromUtf8(s) == *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator!=(const char *s) const
{ return CXString::fromUtf8(s) != *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator<(const char *s) const
{ return CXString::fromUtf8(s) > *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator>(const char *s) const
{ return CXString::fromUtf8(s) < *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator<=(const char *s) const
{ return CXString::fromUtf8(s) >= *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator>=(const char *s) const
{ return CXString::fromUtf8(s) <= *this; }

inline QT_ASCII_CAST_WARN bool CXLatin1String::operator==(const CXByteArray &s) const
{ return CXString::fromUtf8(s) == *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator!=(const CXByteArray &s) const
{ return CXString::fromUtf8(s) != *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator<(const CXByteArray &s) const
{ return CXString::fromUtf8(s) > *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator>(const CXByteArray &s) const
{ return CXString::fromUtf8(s) < *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator<=(const CXByteArray &s) const
{ return CXString::fromUtf8(s) >= *this; }
inline QT_ASCII_CAST_WARN bool CXLatin1String::operator>=(const CXByteArray &s) const
{ return CXString::fromUtf8(s) <= *this; }

inline QT_ASCII_CAST_WARN bool CXString::operator==(const CXByteArray &s) const
{ return CXString::compare_helper(constData(), size(), s.constData(), qstrnlen(s.constData(), s.size())) == 0; }
inline QT_ASCII_CAST_WARN bool CXString::operator!=(const CXByteArray &s) const
{ return CXString::compare_helper(constData(), size(), s.constData(), qstrnlen(s.constData(), s.size())) != 0; }
inline QT_ASCII_CAST_WARN bool CXString::operator<(const CXByteArray &s) const
{ return CXString::compare_helper(constData(), size(), s.constData(), s.size()) < 0; }
inline QT_ASCII_CAST_WARN bool CXString::operator>(const CXByteArray &s) const
{ return CXString::compare_helper(constData(), size(), s.constData(), s.size()) > 0; }
inline QT_ASCII_CAST_WARN bool CXString::operator<=(const CXByteArray &s) const
{ return CXString::compare_helper(constData(), size(), s.constData(), s.size()) <= 0; }
inline QT_ASCII_CAST_WARN bool CXString::operator>=(const CXByteArray &s) const
{ return CXString::compare_helper(constData(), size(), s.constData(), s.size()) >= 0; }

inline bool CXByteArray::operator==(const CXString &s) const
{ return CXString::compare_helper(s.constData(), s.size(), constData(), qstrnlen(constData(), size())) == 0; }
inline bool CXByteArray::operator!=(const CXString &s) const
{ return CXString::compare_helper(s.constData(), s.size(), constData(), qstrnlen(constData(), size())) != 0; }
inline bool CXByteArray::operator<(const CXString &s) const
{ return CXString::compare_helper(s.constData(), s.size(), constData(), size()) > 0; }
inline bool CXByteArray::operator>(const CXString &s) const
{ return CXString::compare_helper(s.constData(), s.size(), constData(), size()) < 0; }
inline bool CXByteArray::operator<=(const CXString &s) const
{ return CXString::compare_helper(s.constData(), s.size(), constData(), size()) >= 0; }
inline bool CXByteArray::operator>=(const CXString &s) const
{ return CXString::compare_helper(s.constData(), s.size(), constData(), size()) <= 0; }

#endif // !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)

#if !defined(QT_NO_CAST_TO_ASCII) && QT_DEPRECATED_SINCE(5, 15)
inline CXByteArray &CXByteArray::append(const CXString &s)
{ return append(s.toUtf8()); }
inline CXByteArray &CXByteArray::insert(int i, const CXString &s)
{ return insert(i, s.toUtf8()); }
inline CXByteArray &CXByteArray::replace(char c, const CXString &after)
{ return replace(c, after.toUtf8()); }
inline CXByteArray &CXByteArray::replace(const CXString &before, const char *after)
{ return replace(before.toUtf8(), after); }
inline CXByteArray &CXByteArray::replace(const CXString &before, const CXByteArray &after)
{ return replace(before.toUtf8(), after); }
inline CXByteArray &CXByteArray::operator+=(const CXString &s)
{ return operator+=(s.toUtf8()); }
inline int CXByteArray::indexOf(const CXString &s, int from) const
{ return indexOf(s.toUtf8(), from); }
inline int CXByteArray::lastIndexOf(const CXString &s, int from) const
{ return lastIndexOf(s.toUtf8(), from); }
#endif // !defined(QT_NO_CAST_TO_ASCII) && QT_DEPRECATED_SINCE(5, 15)

#if !defined(QT_USE_FAST_OPERATOR_PLUS) && !defined(QT_USE_QSTRINGBUILDER)
inline const CXString operator+(const CXString &s1, const CXString &s2)
{ CXString t(s1); t += s2; return t; }
inline const CXString operator+(const CXString &s1, CXChar s2)
{ CXString t(s1); t += s2; return t; }
inline const CXString operator+(CXChar s1, const CXString &s2)
{ CXString t(s1); t += s2; return t; }
#  if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
inline QT_ASCII_CAST_WARN const CXString operator+(const CXString &s1, const char *s2)
{ CXString t(s1); t += CXString::fromUtf8(s2); return t; }
inline QT_ASCII_CAST_WARN const CXString operator+(const char *s1, const CXString &s2)
{ CXString t = CXString::fromUtf8(s1); t += s2; return t; }
inline QT_ASCII_CAST_WARN const CXString operator+(char c, const CXString &s)
{ CXString t = s; t.prepend(CXChar::fromLatin1(c)); return t; }
inline QT_ASCII_CAST_WARN const CXString operator+(const CXString &s, char c)
{ CXString t = s; t += CXChar::fromLatin1(c); return t; }
inline QT_ASCII_CAST_WARN const CXString operator+(const CXByteArray &ba, const CXString &s)
{ CXString t = CXString::fromUtf8(ba); t += s; return t; }
inline QT_ASCII_CAST_WARN const CXString operator+(const CXString &s, const CXByteArray &ba)
{ CXString t(s); t += CXString::fromUtf8(ba); return t; }
#  endif // QT_NO_CAST_FROM_ASCII
#endif // QT_USE_QSTRINGBUILDER

inline std::string CXString::toStdString() const
{ return toUtf8().toStdString(); }

inline CXString CXString::fromStdString(const std::string &s)
{ return fromUtf8(s.data(), int(s.size())); }

inline std::wstring CXString::toStdWString() const
{
    std::wstring str;
    str.resize(length());
#if __cplusplus >= 201703L
    str.resize(toWCharArray(str.data()));
#else
    if (length())
        str.resize(toWCharArray(&str.front()));
#endif
    return str;
}

inline CXString CXString::fromStdWString(const std::wstring &s)
{ return fromWCharArray(s.data(), int(s.size())); }

#if defined(Q_STDLIB_UNICODE_STRINGS)
inline CXString CXString::fromStdU16String(const std::u16string &s)
{ return fromUtf16(s.data(), int(s.size())); }

inline std::u16string CXString::toStdU16String() const
{ return std::u16string(reinterpret_cast<const char16_t*>(utf16()), length()); }

inline CXString CXString::fromStdU32String(const std::u32string &s)
{ return fromUcs4(s.data(), int(s.size())); }

inline std::u32string CXString::toStdU32String() const
{
    std::u32string u32str(length(), char32_t(0));
    int len = toUcs4_helper(d->data(), length(), reinterpret_cast<uint*>(&u32str[0]));
    u32str.resize(len);
    return u32str;
}
#endif

#if !defined(QT_NO_DATASTREAM) || (defined(QT_BOOTSTRAPPED) && !defined(QT_BUILD_QMAKE))
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const CXString &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, CXString &);
#endif

Q_DECLARE_SHARED(CXString)
Q_DECLARE_OPERATORS_FOR_FLAGS(CXString::SectionFlags)


class Q_CORE_EXPORT CXStringRef {
    const CXString *m_string;
    int m_position;
    int m_size;
public:
    typedef CXString::size_type size_type;
    typedef CXString::value_type value_type;
    typedef const CXChar *const_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef CXString::const_pointer const_pointer;
    typedef CXString::const_reference const_reference;

    // ### Qt 6: make this constructor constexpr, after the destructor is made trivial
    inline CXStringRef() : m_string(nullptr), m_position(0), m_size(0) {}
    inline CXStringRef(const CXString *string, int position, int size);
    inline CXStringRef(const CXString *string);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    // ### Qt 6: remove all of these, the implicit ones are fine
    CXStringRef(const CXStringRef &other) noexcept
        :m_string(other.m_string), m_position(other.m_position), m_size(other.m_size)
        {}
    CXStringRef(CXStringRef &&other) noexcept : m_string(other.m_string), m_position(other.m_position), m_size(other.m_size) {}
    CXStringRef &operator=(CXStringRef &&other) noexcept { return *this = other; }
    CXStringRef &operator=(const CXStringRef &other) noexcept
    {
        m_string = other.m_string; m_position = other.m_position;
        m_size = other.m_size; return *this;
    }
    inline ~CXStringRef(){}
#endif // Qt < 6.0.0

    inline const CXString *string() const { return m_string; }
    inline int position() const { return m_position; }
    inline int size() const { return m_size; }
    inline int count() const { return m_size; }
    inline int length() const { return m_size; }

#if QT_STRINGVIEW_LEVEL < 2
    int indexOf(const CXString &str, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int indexOf(const CXStringRef &str, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif
    C_REQUIRED_RESULT int indexOf(CXStringView s, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::findString(*this, from, s, cs)); } // ### Qt6: qsizetype
    int indexOf(CXChar ch, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int indexOf(CXLatin1String str, int from = 0, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#if QT_STRINGVIEW_LEVEL < 2
    int lastIndexOf(const CXStringRef &str, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int lastIndexOf(const CXString &str, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif
    int lastIndexOf(CXChar ch, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int lastIndexOf(CXLatin1String str, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    C_REQUIRED_RESULT int lastIndexOf(CXStringView s, int from = -1, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return int(CXPrivate::lastIndexOf(*this, from, s, cs)); } // ### Qt6: qsizetype

#if QT_STRINGVIEW_LEVEL < 2
    inline bool contains(const CXString &str, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    inline bool contains(const CXStringRef &str, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif
    inline bool contains(CXChar ch, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    inline bool contains(CXLatin1String str, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    inline bool contains(CXStringView str, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;

    int count(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int count(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    int count(const CXStringRef &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;

#if QT_DEPRECATED_SINCE(5, 15)
    C_REQUIRED_RESULT QT_DEPRECATED_VERSION_X_5_15("Use cx::SplitBehavior variant instead")
    CXVector<CXStringRef> split(const CXString &sep, CXString::SplitBehavior behavior,
                              cx::CaseSensitivity cs = cx::CaseSensitive) const;
    C_REQUIRED_RESULT QT_DEPRECATED_VERSION_X_5_15("Use cx::SplitBehavior variant instead")
    CXVector<CXStringRef> split(CXChar sep, CXString::SplitBehavior behavior,
                              cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif // 5.15 deprecations

    C_REQUIRED_RESULT
    CXVector<CXStringRef> split(const CXString &sep, cx::SplitBehavior behavior = cx::KeepEmptyParts,
                              cx::CaseSensitivity cs = cx::CaseSensitive) const;
    C_REQUIRED_RESULT
    CXVector<CXStringRef> split(CXChar sep, cx::SplitBehavior behavior = cx::KeepEmptyParts,
                              cx::CaseSensitivity cs = cx::CaseSensitive) const;

    C_REQUIRED_RESULT CXStringRef left(int n) const;
    C_REQUIRED_RESULT CXStringRef right(int n) const;
    C_REQUIRED_RESULT CXStringRef mid(int pos, int n = -1) const;
    C_REQUIRED_RESULT CXStringRef chopped(int n) const
    { C_ASSERT(n >= 0); C_ASSERT(n <= size()); return left(size() - n); }

    void truncate(int pos) noexcept { m_size = qBound(0, pos, m_size); }
    void chop(int n) noexcept
    {
        if (n >= m_size)
            m_size = 0;
        else if (n > 0)
            m_size -= n;
    }

    bool isRightToLeft() const;

    C_REQUIRED_RESULT bool startsWith(CXStringView s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::startsWith(*this, s, cs); }
    bool startsWith(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    bool startsWith(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#if QT_STRINGVIEW_LEVEL < 2
    bool startsWith(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    bool startsWith(const CXStringRef &c, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif

    C_REQUIRED_RESULT bool endsWith(CXStringView s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::endsWith(*this, s, cs); }
    bool endsWith(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    bool endsWith(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#if QT_STRINGVIEW_LEVEL < 2
    bool endsWith(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive) const;
    bool endsWith(const CXStringRef &c, cx::CaseSensitivity cs = cx::CaseSensitive) const;
#endif

    inline CXStringRef &operator=(const CXString *string);

    inline const CXChar *unicode() const
    {
        if (!m_string)
            return reinterpret_cast<const CXChar *>(CXString::Data::sharedNull()->data());
        return m_string->unicode() + m_position;
    }
    inline const CXChar *data() const { return unicode(); }
    inline const CXChar *constData() const {  return unicode(); }

    inline const_iterator begin() const { return unicode(); }
    inline const_iterator cbegin() const { return unicode(); }
    inline const_iterator constBegin() const { return unicode(); }
    inline const_iterator end() const { return unicode() + size(); }
    inline const_iterator cend() const { return unicode() + size(); }
    inline const_iterator constEnd() const { return unicode() + size(); }
    inline const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    inline const_reverse_iterator crbegin() const { return rbegin(); }
    inline const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    inline const_reverse_iterator crend() const { return rend(); }

    C_REQUIRED_RESULT CXByteArray toAscii() const { return toLatin1(); }
    C_REQUIRED_RESULT CXByteArray toLatin1() const;
    C_REQUIRED_RESULT CXByteArray toUtf8() const;
    C_REQUIRED_RESULT CXByteArray toLocal8Bit() const;
    C_REQUIRED_RESULT CXVector<cuint> toUcs4() const;

    inline void clear() { m_string = nullptr; m_position = m_size = 0; }
    CXString toString() const;
    inline bool isEmpty() const { return m_size == 0; }
    inline bool isNull() const { return m_string == nullptr || m_string->isNull(); }

    CXStringRef appendTo(CXString *string) const;

    inline const CXChar at(int i) const { C_ASSERT(static_cast<cuint>(i) < static_cast<cuint>(size())); return m_string->at(i + m_position); }
    CXChar operator[](int i) const { return at(i); }
    C_REQUIRED_RESULT CXChar front() const { return at(0); }
    C_REQUIRED_RESULT CXChar back() const { return at(size() - 1); }

#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
    // ASCII compatibility
    inline bool operator==(const char *s) const;
    inline bool operator!=(const char *s) const;
    inline bool operator<(const char *s) const;
    inline bool operator<=(const char *s) const;
    inline bool operator>(const char *s) const;
    inline bool operator>=(const char *s) const;
#endif

    int compare(const CXString &s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;
    int compare(const CXStringRef &s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;
    int compare(CXChar c, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept
    { return CXPrivate::compareStrings(*this, CXStringView(&c, 1), cs); }
    int compare(CXLatin1String s, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;
#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
    int compare(const CXByteArray &s, cx::CaseSensitivity cs = cx::CaseSensitive) const
    { return CXString::compare_helper(unicode(), size(), s.data(), cx_strnlen(s.data(), s.size()), cs); }
#endif
    static int compare(const CXStringRef &s1, const CXString &s2,
                       cx::CaseSensitivity = cx::CaseSensitive) noexcept;
    static int compare(const CXStringRef &s1, const CXStringRef &s2,
                       cx::CaseSensitivity = cx::CaseSensitive) noexcept;
    static int compare(const CXStringRef &s1, CXLatin1String s2,
                       cx::CaseSensitivity cs = cx::CaseSensitive) noexcept;

    int localeAwareCompare(const CXString &s) const;
    int localeAwareCompare(const CXStringRef &s) const;
    int localeAwareCompare(CXStringView str) const;
    static int localeAwareCompare(const CXStringRef &s1, const CXString &s2);
    static int localeAwareCompare(const CXStringRef &s1, const CXStringRef &s2);
    static int localeAwareCompare(CXStringView s1, CXStringView s2);

    C_REQUIRED_RESULT CXStringRef trimmed() const;
    short  toShort(bool *ok = nullptr, int base = 10) const;
    cushort toUShort(bool *ok = nullptr, int base = 10) const;
    int toInt(bool *ok = nullptr, int base = 10) const;
    cuint toUInt(bool *ok = nullptr, int base = 10) const;
    long toLong(bool *ok = nullptr, int base = 10) const;
    culong toULong(bool *ok = nullptr, int base = 10) const;
    clonglong toLongLong(bool *ok = nullptr, int base = 10) const;
    culonglong toULongLong(bool *ok = nullptr, int base = 10) const;
    float toFloat(bool *ok = nullptr) const;
    double toDouble(bool *ok = nullptr) const;
};
CX_DECLARE_TYPEINFO(CXStringRef, CX_PRIMITIVE_TYPE);

inline CXStringRef &CXStringRef::operator=(const CXString *aString)
{ m_string = aString; m_position = 0; m_size = aString?aString->size():0; return *this; }

inline CXStringRef::CXStringRef(const CXString *aString, int aPosition, int aSize)
        :m_string(aString), m_position(aPosition), m_size(aSize){}

inline CXStringRef::CXStringRef(const CXString *aString)
    :m_string(aString), m_position(0), m_size(aString?aString->size() : 0){}

// CXStringRef <> CXStringRef
C_SYMBOL_EXPORT bool operator==(const CXStringRef &s1, const CXStringRef &s2) noexcept;
inline bool operator!=(const CXStringRef &s1, const CXStringRef &s2) noexcept
{ return !(s1 == s2); }
C_SYMBOL_EXPORT bool operator<(const CXStringRef &s1, const CXStringRef &s2) noexcept;
inline bool operator>(const CXStringRef &s1, const CXStringRef &s2) noexcept
{ return s2 < s1; }
inline bool operator<=(const CXStringRef &s1, const CXStringRef &s2) noexcept
{ return !(s1 > s2); }
inline bool operator>=(const CXStringRef &s1, const CXStringRef &s2) noexcept
{ return !(s1 < s2); }

// CXString <> CXStringRef
C_SYMBOL_EXPORT bool operator==(const CXString &lhs, const CXStringRef &rhs) noexcept;
inline bool operator!=(const CXString &lhs, const CXStringRef &rhs) noexcept { return lhs.compare(rhs) != 0; }
inline bool operator< (const CXString &lhs, const CXStringRef &rhs) noexcept { return lhs.compare(rhs) <  0; }
inline bool operator> (const CXString &lhs, const CXStringRef &rhs) noexcept { return lhs.compare(rhs) >  0; }
inline bool operator<=(const CXString &lhs, const CXStringRef &rhs) noexcept { return lhs.compare(rhs) <= 0; }
inline bool operator>=(const CXString &lhs, const CXStringRef &rhs) noexcept { return lhs.compare(rhs) >= 0; }

inline bool operator==(const CXStringRef &lhs, const CXString &rhs) noexcept { return rhs == lhs; }
inline bool operator!=(const CXStringRef &lhs, const CXString &rhs) noexcept { return rhs != lhs; }
inline bool operator< (const CXStringRef &lhs, const CXString &rhs) noexcept { return rhs >  lhs; }
inline bool operator> (const CXStringRef &lhs, const CXString &rhs) noexcept { return rhs <  lhs; }
inline bool operator<=(const CXStringRef &lhs, const CXString &rhs) noexcept { return rhs >= lhs; }
inline bool operator>=(const CXStringRef &lhs, const CXString &rhs) noexcept { return rhs <= lhs; }

#if QT_STRINGVIEW_LEVEL < 2
inline int CXString::compare(const CXStringRef &s, cx::CaseSensitivity cs) const noexcept
{ return CXString::compare_helper(constData(), length(), s.constData(), s.length(), cs); }
#endif
inline int CXString::compare(CXStringView s, cx::CaseSensitivity cs) const noexcept
{ return -s.compare(*this, cs); }
inline int CXString::compare(const CXString &s1, const CXStringRef &s2, cx::CaseSensitivity cs) noexcept
{ return CXString::compare_helper(s1.constData(), s1.length(), s2.constData(), s2.length(), cs); }
inline int CXStringRef::compare(const CXString &s, cx::CaseSensitivity cs) const noexcept
{ return CXString::compare_helper(constData(), length(), s.constData(), s.length(), cs); }
inline int CXStringRef::compare(const CXStringRef &s, cx::CaseSensitivity cs) const noexcept
{ return CXString::compare_helper(constData(), length(), s.constData(), s.length(), cs); }
inline int CXStringRef::compare(CXLatin1String s, cx::CaseSensitivity cs) const noexcept
{ return CXString::compare_helper(constData(), length(), s, cs); }
inline int CXStringRef::compare(const CXStringRef &s1, const CXString &s2, cx::CaseSensitivity cs) noexcept
{ return CXString::compare_helper(s1.constData(), s1.length(), s2.constData(), s2.length(), cs); }
inline int CXStringRef::compare(const CXStringRef &s1, const CXStringRef &s2, cx::CaseSensitivity cs) noexcept
{ return CXString::compare_helper(s1.constData(), s1.length(), s2.constData(), s2.length(), cs); }
inline int CXStringRef::compare(const CXStringRef &s1, CXLatin1String s2, cx::CaseSensitivity cs) noexcept
{ return CXString::compare_helper(s1.constData(), s1.length(), s2, cs); }

// CXLatin1String <> CXStringRef
C_SYMBOL_EXPORT bool operator==(CXLatin1String lhs, const CXStringRef &rhs) noexcept;
inline bool operator!=(CXLatin1String lhs, const CXStringRef &rhs) noexcept { return rhs.compare(lhs) != 0; }
inline bool operator< (CXLatin1String lhs, const CXStringRef &rhs) noexcept { return rhs.compare(lhs) >  0; }
inline bool operator> (CXLatin1String lhs, const CXStringRef &rhs) noexcept { return rhs.compare(lhs) <  0; }
inline bool operator<=(CXLatin1String lhs, const CXStringRef &rhs) noexcept { return rhs.compare(lhs) >= 0; }
inline bool operator>=(CXLatin1String lhs, const CXStringRef &rhs) noexcept { return rhs.compare(lhs) <= 0; }

inline bool operator==(const CXStringRef &lhs, CXLatin1String rhs) noexcept { return rhs == lhs; }
inline bool operator!=(const CXStringRef &lhs, CXLatin1String rhs) noexcept { return rhs != lhs; }
inline bool operator< (const CXStringRef &lhs, CXLatin1String rhs) noexcept { return rhs >  lhs; }
inline bool operator> (const CXStringRef &lhs, CXLatin1String rhs) noexcept { return rhs <  lhs; }
inline bool operator<=(const CXStringRef &lhs, CXLatin1String rhs) noexcept { return rhs >= lhs; }
inline bool operator>=(const CXStringRef &lhs, CXLatin1String rhs) noexcept { return rhs <= lhs; }

// CXChar <> CXString
inline bool operator==(CXChar lhs, const CXString &rhs) noexcept
{ return rhs.size() == 1 && lhs == rhs.front(); }
inline bool operator< (CXChar lhs, const CXString &rhs) noexcept
{ return CXString::compare_helper(&lhs, 1, rhs.data(), rhs.size()) <  0; }
inline bool operator> (CXChar lhs, const CXString &rhs) noexcept
{ return CXString::compare_helper(&lhs, 1, rhs.data(), rhs.size()) >  0; }

inline bool operator!=(CXChar lhs, const CXString &rhs) noexcept { return !(lhs == rhs); }
inline bool operator<=(CXChar lhs, const CXString &rhs) noexcept { return !(lhs >  rhs); }
inline bool operator>=(CXChar lhs, const CXString &rhs) noexcept { return !(lhs <  rhs); }

inline bool operator==(const CXString &lhs, CXChar rhs) noexcept { return   rhs == lhs; }
inline bool operator!=(const CXString &lhs, CXChar rhs) noexcept { return !(rhs == lhs); }
inline bool operator< (const CXString &lhs, CXChar rhs) noexcept { return   rhs >  lhs; }
inline bool operator> (const CXString &lhs, CXChar rhs) noexcept { return   rhs <  lhs; }
inline bool operator<=(const CXString &lhs, CXChar rhs) noexcept { return !(rhs <  lhs); }
inline bool operator>=(const CXString &lhs, CXChar rhs) noexcept { return !(rhs >  lhs); }

// CXChar <> CXStringRef
inline bool operator==(CXChar lhs, const CXStringRef &rhs) noexcept
{ return rhs.size() == 1 && lhs == rhs.front(); }
inline bool operator< (CXChar lhs, const CXStringRef &rhs) noexcept
{ return CXString::compare_helper(&lhs, 1, rhs.data(), rhs.size()) <  0; }
inline bool operator> (CXChar lhs, const CXStringRef &rhs) noexcept
{ return CXString::compare_helper(&lhs, 1, rhs.data(), rhs.size()) >  0; }

inline bool operator!=(CXChar lhs, const CXStringRef &rhs) noexcept { return !(lhs == rhs); }
inline bool operator<=(CXChar lhs, const CXStringRef &rhs) noexcept { return !(lhs >  rhs); }
inline bool operator>=(CXChar lhs, const CXStringRef &rhs) noexcept { return !(lhs <  rhs); }

inline bool operator==(const CXStringRef &lhs, CXChar rhs) noexcept { return   rhs == lhs; }
inline bool operator!=(const CXStringRef &lhs, CXChar rhs) noexcept { return !(rhs == lhs); }
inline bool operator< (const CXStringRef &lhs, CXChar rhs) noexcept { return   rhs >  lhs; }
inline bool operator> (const CXStringRef &lhs, CXChar rhs) noexcept { return   rhs <  lhs; }
inline bool operator<=(const CXStringRef &lhs, CXChar rhs) noexcept { return !(rhs <  lhs); }
inline bool operator>=(const CXStringRef &lhs, CXChar rhs) noexcept { return !(rhs >  lhs); }

// CXChar <> CXLatin1String
inline bool operator==(CXChar lhs, CXLatin1String rhs) noexcept
{ return rhs.size() == 1 && lhs == rhs.front(); }
inline bool operator< (CXChar lhs, CXLatin1String rhs) noexcept
{ return CXString::compare_helper(&lhs, 1, rhs) <  0; }
inline bool operator> (CXChar lhs, CXLatin1String rhs) noexcept
{ return CXString::compare_helper(&lhs, 1, rhs) >  0; }

inline bool operator!=(CXChar lhs, CXLatin1String rhs) noexcept { return !(lhs == rhs); }
inline bool operator<=(CXChar lhs, CXLatin1String rhs) noexcept { return !(lhs >  rhs); }
inline bool operator>=(CXChar lhs, CXLatin1String rhs) noexcept { return !(lhs <  rhs); }

inline bool operator==(CXLatin1String lhs, CXChar rhs) noexcept { return   rhs == lhs; }
inline bool operator!=(CXLatin1String lhs, CXChar rhs) noexcept { return !(rhs == lhs); }
inline bool operator< (CXLatin1String lhs, CXChar rhs) noexcept { return   rhs >  lhs; }
inline bool operator> (CXLatin1String lhs, CXChar rhs) noexcept { return   rhs <  lhs; }
inline bool operator<=(CXLatin1String lhs, CXChar rhs) noexcept { return !(rhs <  lhs); }
inline bool operator>=(CXLatin1String lhs, CXChar rhs) noexcept { return !(rhs >  lhs); }

// CXStringView <> CXStringView
inline bool operator==(CXStringView lhs, CXStringView rhs) noexcept { return lhs.size() == rhs.size() && CXPrivate::compareStrings(lhs, rhs) == 0; }
inline bool operator!=(CXStringView lhs, CXStringView rhs) noexcept { return !(lhs == rhs); }
inline bool operator< (CXStringView lhs, CXStringView rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) <  0; }
inline bool operator<=(CXStringView lhs, CXStringView rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) <= 0; }
inline bool operator> (CXStringView lhs, CXStringView rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) >  0; }
inline bool operator>=(CXStringView lhs, CXStringView rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) >= 0; }

// CXStringView <> CXChar
inline bool operator==(CXStringView lhs, CXChar rhs) noexcept { return lhs == CXStringView(&rhs, 1); }
inline bool operator!=(CXStringView lhs, CXChar rhs) noexcept { return lhs != CXStringView(&rhs, 1); }
inline bool operator< (CXStringView lhs, CXChar rhs) noexcept { return lhs <  CXStringView(&rhs, 1); }
inline bool operator<=(CXStringView lhs, CXChar rhs) noexcept { return lhs <= CXStringView(&rhs, 1); }
inline bool operator> (CXStringView lhs, CXChar rhs) noexcept { return lhs >  CXStringView(&rhs, 1); }
inline bool operator>=(CXStringView lhs, CXChar rhs) noexcept { return lhs >= CXStringView(&rhs, 1); }

inline bool operator==(CXChar lhs, CXStringView rhs) noexcept { return CXStringView(&lhs, 1) == rhs; }
inline bool operator!=(CXChar lhs, CXStringView rhs) noexcept { return CXStringView(&lhs, 1) != rhs; }
inline bool operator< (CXChar lhs, CXStringView rhs) noexcept { return CXStringView(&lhs, 1) <  rhs; }
inline bool operator<=(CXChar lhs, CXStringView rhs) noexcept { return CXStringView(&lhs, 1) <= rhs; }
inline bool operator> (CXChar lhs, CXStringView rhs) noexcept { return CXStringView(&lhs, 1) >  rhs; }
inline bool operator>=(CXChar lhs, CXStringView rhs) noexcept { return CXStringView(&lhs, 1) >= rhs; }

// CXStringView <> CXLatin1String
inline bool operator==(CXStringView lhs, CXLatin1String rhs) noexcept { return lhs.size() == rhs.size() && CXPrivate::compareStrings(lhs, rhs) == 0; }
inline bool operator!=(CXStringView lhs, CXLatin1String rhs) noexcept { return !(lhs == rhs); }
inline bool operator< (CXStringView lhs, CXLatin1String rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) <  0; }
inline bool operator<=(CXStringView lhs, CXLatin1String rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) <= 0; }
inline bool operator> (CXStringView lhs, CXLatin1String rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) >  0; }
inline bool operator>=(CXStringView lhs, CXLatin1String rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) >= 0; }

inline bool operator==(CXLatin1String lhs, CXStringView rhs) noexcept { return lhs.size() == rhs.size() && CXPrivate::compareStrings(lhs, rhs) == 0; }
inline bool operator!=(CXLatin1String lhs, CXStringView rhs) noexcept { return !(lhs == rhs); }
inline bool operator< (CXLatin1String lhs, CXStringView rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) <  0; }
inline bool operator<=(CXLatin1String lhs, CXStringView rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) <= 0; }
inline bool operator> (CXLatin1String lhs, CXStringView rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) >  0; }
inline bool operator>=(CXLatin1String lhs, CXStringView rhs) noexcept { return CXPrivate::compareStrings(lhs, rhs) >= 0; }

#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
// CXStringRef <> CXByteArray
inline bool operator==(const CXStringRef &lhs, const CXByteArray &rhs) { return lhs.compare(rhs) == 0; }
inline bool operator!=(const CXStringRef &lhs, const CXByteArray &rhs) { return lhs.compare(rhs) != 0; }
inline bool operator< (const CXStringRef &lhs, const CXByteArray &rhs) { return lhs.compare(rhs) <  0; }
inline bool operator> (const CXStringRef &lhs, const CXByteArray &rhs) { return lhs.compare(rhs) >  0; }
inline bool operator<=(const CXStringRef &lhs, const CXByteArray &rhs) { return lhs.compare(rhs) <= 0; }
inline bool operator>=(const CXStringRef &lhs, const CXByteArray &rhs) { return lhs.compare(rhs) >= 0; }

inline bool operator==(const CXByteArray &lhs, const CXStringRef &rhs) { return rhs.compare(lhs) == 0; }
inline bool operator!=(const CXByteArray &lhs, const CXStringRef &rhs) { return rhs.compare(lhs) != 0; }
inline bool operator< (const CXByteArray &lhs, const CXStringRef &rhs) { return rhs.compare(lhs) >  0; }
inline bool operator> (const CXByteArray &lhs, const CXStringRef &rhs) { return rhs.compare(lhs) <  0; }
inline bool operator<=(const CXByteArray &lhs, const CXStringRef &rhs) { return rhs.compare(lhs) >= 0; }
inline bool operator>=(const CXByteArray &lhs, const CXStringRef &rhs) { return rhs.compare(lhs) <= 0; }

// CXStringRef <> const char *
inline bool CXStringRef::operator==(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) == 0; }
inline bool CXStringRef::operator!=(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) != 0; }
inline bool CXStringRef::operator<(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) < 0; }
inline bool CXStringRef::operator<=(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) <= 0; }
inline bool CXStringRef::operator>(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) > 0; }
inline bool CXStringRef::operator>=(const char *s) const
{ return CXString::compare_helper(constData(), size(), s, -1) >= 0; }

inline bool operator==(const char *s1, const CXStringRef &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) == 0; }
inline bool operator!=(const char *s1, const CXStringRef &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) != 0; }
inline bool operator<(const char *s1, const CXStringRef &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) > 0; }
inline bool operator<=(const char *s1, const CXStringRef &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) >= 0; }
inline bool operator>(const char *s1, const CXStringRef &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) < 0; }
inline bool operator>=(const char *s1, const CXStringRef &s2)
{ return CXString::compare_helper(s2.constData(), s2.size(), s1, -1) <= 0; }
#endif // !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)

inline int CXString::localeAwareCompare(const CXStringRef &s) const
{ return localeAwareCompare_helper(constData(), length(), s.constData(), s.length()); }
inline int CXString::localeAwareCompare(const CXString& s1, const CXStringRef& s2)
{ return localeAwareCompare_helper(s1.constData(), s1.length(), s2.constData(), s2.length()); }
inline int CXStringRef::localeAwareCompare(const CXString &s) const
{ return CXString::localeAwareCompare_helper(constData(), length(), s.constData(), s.length()); }
inline int CXStringRef::localeAwareCompare(const CXStringRef &s) const
{ return CXString::localeAwareCompare_helper(constData(), length(), s.constData(), s.length()); }
inline int CXStringRef::localeAwareCompare(CXStringView s) const
{ return CXString::localeAwareCompare_helper(constData(), length(), s.data(), int(s.size())); }
inline int CXStringRef::localeAwareCompare(const CXStringRef &s1, const CXString &s2)
{ return CXString::localeAwareCompare_helper(s1.constData(), s1.length(), s2.constData(), s2.length()); }
inline int CXStringRef::localeAwareCompare(const CXStringRef &s1, const CXStringRef &s2)
{ return CXString::localeAwareCompare_helper(s1.constData(), s1.length(), s2.constData(), s2.length()); }
inline int CXStringRef::localeAwareCompare(CXStringView s1, CXStringView s2)
{ return CXString::localeAwareCompare_helper(s1.data(), int(s1.size()), s2.data(), int(s2.size())); }

#if QT_STRINGVIEW_LEVEL < 2
inline bool CXStringRef::contains(const CXString &s, cx::CaseSensitivity cs) const
{ return indexOf(s, 0, cs) != -1; }
inline bool CXStringRef::contains(const CXStringRef &s, cx::CaseSensitivity cs) const
{ return indexOf(s, 0, cs) != -1; }
#endif
inline bool CXStringRef::contains(CXLatin1String s, cx::CaseSensitivity cs) const
{ return indexOf(s, 0, cs) != -1; }
inline bool CXStringRef::contains(CXChar c, cx::CaseSensitivity cs) const
{ return indexOf(c, 0, cs) != -1; }
inline bool CXStringRef::contains(CXStringView s, cx::CaseSensitivity cs) const noexcept
{ return indexOf(s, 0, cs) != -1; }

inline CXString &CXString::insert(int i, const CXStringRef &s)
{ return insert(i, s.constData(), s.length()); }

inline CXString operator+(const CXString &s1, const CXStringRef &s2)
{ CXString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline CXString operator+(const CXStringRef &s1, const CXString &s2)
{ CXString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline CXString operator+(const CXStringRef &s1, CXLatin1String s2)
{ CXString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline CXString operator+(CXLatin1String s1, const CXStringRef &s2)
{ CXString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline CXString operator+(const CXStringRef &s1, const CXStringRef &s2)
{ CXString t; t.reserve(s1.size() + s2.size()); t += s1; t += s2; return t; }
inline CXString operator+(const CXStringRef &s1, CXChar s2)
{ CXString t; t.reserve(s1.size() + 1); t += s1; t += s2; return t; }
inline CXString operator+(CXChar s1, const CXStringRef &s2)
{ CXString t; t.reserve(1 + s2.size()); t += s1; t += s2; return t; }

namespace cx {
inline CXString escape(const CXString &plain) {
    return plain.toHtmlEscaped();
}
}

namespace CXPrivate {
// used by qPrintable() and qUtf8Printable() macros
inline const CXString &asString(const CXString &s)    { return s; }
inline CXString &&asString(CXString &&s)              { return std::move(s); }
}

//
// CXStringView::arg() implementation
//

namespace CXPrivate {

struct ArgBase {
    enum Tag : cuchar { L1, U8, U16 } tag;
};

struct CXStringViewArg : ArgBase {
    CXStringView string;
    CXStringViewArg() = default;
    C_DECL_CONSTEXPR explicit CXStringViewArg(CXStringView v) noexcept : ArgBase{U16}, string{v} {}
};

struct CXLatin1StringArg : ArgBase {
    CXLatin1String string;
    CXLatin1StringArg() = default;
    C_DECL_CONSTEXPR explicit CXLatin1StringArg(CXLatin1String v) noexcept : ArgBase{L1}, string{v} {}
};

C_REQUIRED_RESULT C_SYMBOL_EXPORT CXString argToCXString(CXStringView pattern, size_t n, const ArgBase **args);
C_REQUIRED_RESULT C_SYMBOL_EXPORT CXString argToCXString(CXLatin1String pattern, size_t n, const ArgBase **args);

template <typename StringView, typename...Args>
C_REQUIRED_RESULT inline CXString argToCXStringDispatch(StringView pattern, const Args &...args)
{
    const ArgBase *argBases[] = {&args..., /* avoid zero-sized array */ nullptr};
    return CXPrivate::argToCXString(pattern, sizeof...(Args), argBases);
}

                 inline CXStringViewArg   qStringLikeToArg(const CXString &s) noexcept { return CXStringViewArg{cxToStringViewIgnoringNull(s)}; }
C_DECL_CONSTEXPR inline CXStringViewArg   qStringLikeToArg(CXStringView s) noexcept { return CXStringViewArg{s}; }
                 inline CXStringViewArg   qStringLikeToArg(const CXChar &c) noexcept { return CXStringViewArg{CXStringView{&c, 1}}; }
C_DECL_CONSTEXPR inline CXLatin1StringArg qStringLikeToArg(CXLatin1String s) noexcept { return CXLatin1StringArg{s}; }

} // namespace CXPrivate

template <typename...Args>
inline CXString CXStringView::arg(Args &&...args) const
{
    return CXPrivate::argToCXStringDispatch(*this, CXPrivate::qStringLikeToArg(args)...);
}

template <typename...Args>
inline CXString CXLatin1String::arg(Args &&...args) const
{
    return CXPrivate::argToCXStringDispatch(*this, CXPrivate::qStringLikeToArg(args)...);
}

inline csizetype CXStringView::count(CXChar c, cx::CaseSensitivity cs) const noexcept
{ return toString().count(c, cs); }
inline csizetype CXStringView::count(CXStringView s, cx::CaseSensitivity cs) const noexcept
{ return toString().count(s.toString(), cs); }

inline short CXStringView::toShort(bool *ok, int base) const
{ return toString().toShort(ok, base); }
inline cushort CXStringView::toUShort(bool *ok, int base) const
{ return toString().toUShort(ok, base); }
inline int CXStringView::toInt(bool *ok, int base) const
{ return toString().toInt(ok, base); }
inline cuint CXStringView::toUInt(bool *ok, int base) const
{ return toString().toUInt(ok, base); }
inline long CXStringView::toLong(bool *ok, int base) const
{ return toString().toLong(ok, base); }
inline culong CXStringView::toULong(bool *ok, int base) const
{ return toString().toULong(ok, base); }
inline clonglong CXStringView::toLongLong(bool *ok, int base) const
{ return toString().toLongLong(ok, base); }
inline culonglong CXStringView::toULongLong(bool *ok, int base) const
{ return toString().toULongLong(ok, base); }
inline float CXStringView::toFloat(bool *ok) const
{ return toString().toFloat(ok); }
inline double CXStringView::toDouble(bool *ok) const
{ return toString().toDouble(ok); }

CX_END_NAMESPACE

#endif // clibrary_CX_STRING_H
