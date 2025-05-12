//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_BYTEARRAY_H
#define clibrary_BYTEARRAY_H
#include "../global/global.h"
#include "3thrd/macros/macros.h"
#include "../global/name-space.h"

#include <string.h>

#include <string>

#ifdef truncate
#error qbytearray.h must be included before any header file that defines truncate
#endif

CX_BEGIN_NAMESPACE

C_SYMBOL_EXPORT char *cx_strdup(const char *);

inline cuint cx_strlen(const char *str)
{ return str ? static_cast<cuint>(strlen(str)) : 0; }

inline cuint cx_strnlen(const char *str, cuint maxlen)
{
    cuint length = 0;
    if (str) {
        while (length < maxlen && *str++) {
            length++;
        }
    }
    return length;
}

C_SYMBOL_EXPORT char *cx_strcpy(char *dst, const char *src);
C_SYMBOL_EXPORT char *cx_strncpy(char *dst, const char *src, cuint len);

C_SYMBOL_EXPORT int cx_strcmp(const char *str1, const char *str2);
C_SYMBOL_EXPORT int cx_strcmp(const CXByteArray &str1, const CXByteArray &str2);
C_SYMBOL_EXPORT int cx_strcmp(const CXByteArray &str1, const char *str2);
static inline int cx_strcmp(const char *str1, const CXByteArray &str2)
{ return -cx_strcmp(str2, str1); }

inline int cx_strncmp(const char *str1, const char *str2, cuint len)
{
    return (str1 && str2) ? strncmp(str1, str2, len)
        : (str1 ? 1 : (str2 ? -1 : 0));
}
C_SYMBOL_EXPORT int cx_stricmp(const char *, const char *);
C_SYMBOL_EXPORT int cx_strnicmp(const char *, const char *, cuint len);
C_SYMBOL_EXPORT int cx_strnicmp(const char *, csizetype, const char *, csizetype = -1);

// implemented in qvsnprintf.cpp
C_SYMBOL_EXPORT int cx_vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
C_SYMBOL_EXPORT int cx_snprintf(char *str, size_t n, const char *fmt, ...);

// qChecksum: Internet checksum
C_SYMBOL_EXPORT cuint16 qChecksum(const char *s, cuint len);
C_SYMBOL_EXPORT cuint16 qChecksum(const char *s, cuint len, cx::ChecksumType standard);

class CXString;
class CXByteRef;
class QDataStream;
template <typename T> class QList;

typedef CXArrayData CXByteArrayData;

template<int N> struct CXStaticByteArrayData
{
    CXByteArrayData ba;
    char data[N + 1];

    CXByteArrayData *data_ptr() const
    {
        C_ASSERT(ba.ref.isStatic());
        return const_cast<CXByteArrayData *>(&ba);
    }
};

struct CXByteArrayDataPtr
{
    CXByteArrayData *ptr;
};

#define CX_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(size, offset) \
    CX_STATIC_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(size, offset)
    /**/

#define CX_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER(size) \
    CX_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(size, sizeof(CXByteArrayData)) \
    /**/

#  define CXByteArrayLiteral(str) \
    ([]() -> CXByteArray { \
        enum { Size = sizeof(str) - 1 }; \
        static const CXStaticByteArrayData<Size> qbytearray_literal = { \
            CX_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER(Size), \
            str }; \
        CXByteArrayDataPtr holder = { qbytearray_literal.data_ptr() }; \
        return CXByteArray(holder); \
    }()) \
    /**/

class C_SYMBOL_EXPORT CXByteArray
{
private:
    typedef CXTypedArrayData<char> Data;

public:
    enum Base64Option {
        Base64Encoding = 0,
        Base64UrlEncoding = 1,

        KeepTrailingEquals = 0,
        OmitTrailingEquals = 2,

        IgnoreBase64DecodingErrors = 0,
        AbortOnBase64DecodingErrors = 4,
    };
    CX_DECLARE_FLAGS(Base64Options, Base64Option)

    enum class Base64DecodingStatus {
        Ok,
        IllegalInputLength,
        IllegalCharacter,
        IllegalPadding,
    };

    inline CXByteArray() noexcept;
    CXByteArray(const char *, int size = -1);
    CXByteArray(int size, char c);
    CXByteArray(int size, cx::Initialization);
    inline CXByteArray(const CXByteArray &) noexcept;
    inline ~CXByteArray();

    CXByteArray &operator=(const CXByteArray &) noexcept;
    CXByteArray &operator=(const char *str);
    inline CXByteArray(CXByteArray && other) noexcept : d(other.d) { other.d = Data::sharedNull(); }
    inline CXByteArray &operator=(CXByteArray &&other) noexcept
    { C_SWAP(d, other.d); return *this; }

    inline void swap(CXByteArray &other) noexcept
    { C_SWAP(d, other.d); }

    inline int size() const;
    inline bool isEmpty() const;
    void resize(int size);

    CXByteArray &fill(char c, int size = -1);

    inline int capacity() const;
    inline void reserve(int size);
    inline void squeeze();

    inline operator const char *() const;
    inline operator const void *() const;
    inline char *data();
    inline const char *data() const;
    inline const char *constData() const;
    inline void detach();
    inline bool isDetached() const;
    inline bool isSharedWith(const CXByteArray &other) const { return d == other.d; }
    void clear();

    inline char at(int i) const;
    inline char operator[](int i) const;
    inline char operator[](uint i) const;
    inline CXByteRef operator[](int i);
    inline CXByteRef operator[](uint i);
    char front() const { return at(0); }
    inline CXByteRef front();
    char back() const { return at(size() - 1); }
    inline CXByteRef back();

    int indexOf(char c, int from = 0) const;
    int indexOf(const char *c, int from = 0) const;
    int indexOf(const CXByteArray &a, int from = 0) const;
    int lastIndexOf(char c, int from = -1) const;
    int lastIndexOf(const char *c, int from = -1) const;
    int lastIndexOf(const CXByteArray &a, int from = -1) const;

    inline bool contains(char c) const;
    inline bool contains(const char *a) const;
    inline bool contains(const CXByteArray &a) const;
    int count(char c) const;
    int count(const char *a) const;
    int count(const CXByteArray &a) const;

    inline int compare(const char *c, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;
    inline int compare(const CXByteArray &a, cx::CaseSensitivity cs = cx::CaseSensitive) const noexcept;

    CXByteArray left(int len) const;
    CXByteArray right(int len) const;
    CXByteArray mid(int index, int len = -1) const;
    CXByteArray chopped(int len) const
    { C_ASSERT(len >= 0); C_ASSERT(len <= size()); return left(size() - len); }

    bool startsWith(const CXByteArray &a) const;
    bool startsWith(char c) const;
    bool startsWith(const char *c) const;

    bool endsWith(const CXByteArray &a) const;
    bool endsWith(char c) const;
    bool endsWith(const char *c) const;

    bool isUpper() const;
    bool isLower() const;

    void truncate(int pos);
    void chop(int n);

    CXByteArray toLower() const;
    CXByteArray toUpper() const;
    CXByteArray trimmed() const;
    CXByteArray simplified() const;

    CXByteArray leftJustified(int width, char fill = ' ', bool truncate = false) const;
    CXByteArray rightJustified(int width, char fill = ' ', bool truncate = false) const;

    CXByteArray &prepend(char c);
    inline CXByteArray &prepend(int count, char c);
    CXByteArray &prepend(const char *s);
    CXByteArray &prepend(const char *s, int len);
    CXByteArray &prepend(const CXByteArray &a);
    CXByteArray &append(char c);
    inline CXByteArray &append(int count, char c);
    CXByteArray &append(const char *s);
    CXByteArray &append(const char *s, int len);
    CXByteArray &append(const CXByteArray &a);
    CXByteArray &insert(int i, char c);
    CXByteArray &insert(int i, int count, char c);
    CXByteArray &insert(int i, const char *s);
    CXByteArray &insert(int i, const char *s, int len);
    CXByteArray &insert(int i, const CXByteArray &a);
    CXByteArray &remove(int index, int len);
    CXByteArray &replace(int index, int len, const char *s);
    CXByteArray &replace(int index, int len, const char *s, int alen);
    CXByteArray &replace(int index, int len, const CXByteArray &s);
    inline CXByteArray &replace(char before, const char *after);
    CXByteArray &replace(char before, const CXByteArray &after);
    inline CXByteArray &replace(const char *before, const char *after);
    CXByteArray &replace(const char *before, int bsize, const char *after, int asize);
    CXByteArray &replace(const CXByteArray &before, const CXByteArray &after);
    inline CXByteArray &replace(const CXByteArray &before, const char *after);
    CXByteArray &replace(const char *before, const CXByteArray &after);
    CXByteArray &replace(char before, char after);
    inline CXByteArray &operator+=(char c);
    inline CXByteArray &operator+=(const char *s);
    inline CXByteArray &operator+=(const CXByteArray &a);

    CXList<CXByteArray> split(char sep) const;

    CXByteArray repeated(int times) const;

    CXByteArray &append(const CXString &s);
    CXByteArray &insert(int i, const CXString &s);
    CXByteArray &replace(const CXString &before, const char *after);
    CXByteArray &replace(char c, const CXString &after);
    CXByteArray &replace(const CXString &before, const CXByteArray &after);

    CXByteArray &operator+=(const CXString &s);
    int indexOf(const CXString &s, int from = 0) const;
    int lastIndexOf(const CXString &s, int from = -1) const;
    inline bool operator==(const CXString &s2) const;
    inline bool operator!=(const CXString &s2) const;
    inline bool operator<(const CXString &s2) const;
    inline bool operator>(const CXString &s2) const;
    inline bool operator<=(const CXString &s2) const;
    inline bool operator>=(const CXString &s2) const;

    short toShort(bool *ok = nullptr, int base = 10) const;
    cushort toUShort(bool *ok = nullptr, int base = 10) const;
    int toInt(bool *ok = nullptr, int base = 10) const;
    cuint toUInt(bool *ok = nullptr, int base = 10) const;
    long toLong(bool *ok = nullptr, int base = 10) const;
    culong toULong(bool *ok = nullptr, int base = 10) const;
    clonglong toLongLong(bool *ok = nullptr, int base = 10) const;
    culonglong toULongLong(bool *ok = nullptr, int base = 10) const;
    float toFloat(bool *ok = nullptr) const;
    double toDouble(bool *ok = nullptr) const;
    CXByteArray toBase64(Base64Options options) const;
    CXByteArray toBase64() const;
    CXByteArray toHex() const;
    CXByteArray toHex(char separator) const;
    CXByteArray toPercentEncoding(const CXByteArray &exclude = CXByteArray(),
                                 const CXByteArray &include = CXByteArray(),
                                 char percent = '%') const;

    inline CXByteArray &setNum(short, int base = 10);
    inline CXByteArray &setNum(cushort, int base = 10);
    inline CXByteArray &setNum(int, int base = 10);
    inline CXByteArray &setNum(cuint, int base = 10);
    CXByteArray &setNum(clonglong, int base = 10);
    CXByteArray &setNum(culonglong, int base = 10);
    inline CXByteArray &setNum(float, char f = 'g', int prec = 6);
    CXByteArray &setNum(double, char f = 'g', int prec = 6);
    CXByteArray &setRawData(const char *a, cuint n); // ### Qt 6: use an int

    static CXByteArray number(int, int base = 10);
    static CXByteArray number(cuint, int base = 10);
    static CXByteArray number(clonglong, int base = 10);
    static CXByteArray number(culonglong, int base = 10);
    static CXByteArray number(double, char f = 'g', int prec = 6);
    static CXByteArray fromRawData(const char *, int size);

    class FromBase64Result;
    static FromBase64Result fromBase64Encoding(CXByteArray &&base64, Base64Options options = Base64Encoding);
    static FromBase64Result fromBase64Encoding(const CXByteArray &base64, Base64Options options = Base64Encoding);
    static CXByteArray fromBase64(const CXByteArray &base64, Base64Options options);
    static CXByteArray fromBase64(const CXByteArray &base64);
    static CXByteArray fromHex(const CXByteArray &hexEncoded);
    static CXByteArray fromPercentEncoding(const CXByteArray &pctEncoded, char percent = '%');

    typedef char *iterator;
    typedef const char *const_iterator;
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

    typedef int size_type;
    typedef cptrdiff difference_type;
    typedef const char & const_reference;
    typedef char & reference;
    typedef char *pointer;
    typedef const char *const_pointer;
    typedef char value_type;
    inline void push_back(char c);
    inline void push_back(const char *c);
    inline void push_back(const CXByteArray &a);
    inline void push_front(char c);
    inline void push_front(const char *c);
    inline void push_front(const CXByteArray &a);
    void shrink_to_fit() { squeeze(); }

    static inline CXByteArray fromStdString(const std::string &s);
    inline std::string toStdString() const;

    inline int count() const { return d->size; }
    int length() const { return d->size; }
    bool isNull() const;

    inline CXByteArray(CXByteArrayDataPtr dd)
        : d(static_cast<Data *>(dd.ptr))
    {
    }

private:
    operator CXNoImplicitBoolCast() const;
    Data *d;
    void reallocData(cuint alloc, Data::AllocationOptions options);
    void expand(int i);
    CXByteArray nulTerminated() const;

    static CXByteArray toLower_helper(const CXByteArray &a);
    static CXByteArray toLower_helper(CXByteArray &a);
    static CXByteArray toUpper_helper(const CXByteArray &a);
    static CXByteArray toUpper_helper(CXByteArray &a);
    static CXByteArray trimmed_helper(const CXByteArray &a);
    static CXByteArray trimmed_helper(CXByteArray &a);
    static CXByteArray simplified_helper(const CXByteArray &a);
    static CXByteArray simplified_helper(CXByteArray &a);

    friend class CXByteRef;
    friend class CXString;
    friend C_SYMBOL_EXPORT CXByteArray qUncompress(const cuchar *data, int nbytes);
public:
    typedef Data * DataPtr;
    inline DataPtr &data_ptr() { return d; }
};

CX_DECLARE_OPERATORS_FOR_FLAGS(CXByteArray::Base64Options)

inline CXByteArray::CXByteArray() noexcept : d(Data::sharedNull()) { }
inline CXByteArray::~CXByteArray() { if (!d->ref.deref()) Data::deallocate(d); }
inline int CXByteArray::size() const
{ return d->size; }

inline char CXByteArray::at(int i) const
{ C_ASSERT(cuint(i) < cuint(size())); return d->data()[i]; }
inline char CXByteArray::operator[](int i) const
{ C_ASSERT(cuint(i) < cuint(size())); return d->data()[i]; }
inline char CXByteArray::operator[](uint i) const
{ C_ASSERT(i < cuint(size())); return d->data()[i]; }

inline bool CXByteArray::isEmpty() const
{ return d->size == 0; }
inline CXByteArray::operator const char *() const
{ return d->data(); }
inline CXByteArray::operator const void *() const
{ return d->data(); }
inline char *CXByteArray::data()
{ detach(); return d->data(); }
inline const char *CXByteArray::data() const
{ return d->data(); }
inline const char *CXByteArray::constData() const
{ return d->data(); }
inline void CXByteArray::detach()
{ if (d->ref.isShared() || (d->offset != sizeof(CXByteArrayData))) reallocData(uint(d->size) + 1u, d->detachFlags()); }
inline bool CXByteArray::isDetached() const
{ return !d->ref.isShared(); }
inline CXByteArray::CXByteArray(const CXByteArray &a) noexcept : d(a.d)
{ d->ref.ref(); }

inline int CXByteArray::capacity() const
{ return d->alloc ? d->alloc - 1 : 0; }

inline void CXByteArray::reserve(int asize)
{
    if (d->ref.isShared() || cuint(asize) + 1u > d->alloc) {
        reallocData(C_MAX(cuint(size()), cuint(asize)) + 1u, d->detachFlags() | Data::CapacityReserved);
    }
    else {
        // cannot set unconditionally, since d could be the shared_null or
        // otherwise static
        d->capacityReserved = true;
    }
}

inline void CXByteArray::squeeze()
{
    if (d->ref.isShared() || cuint(d->size) + 1u < d->alloc) {
        reallocData(cuint(d->size) + 1u, d->detachFlags() & ~Data::CapacityReserved);
    } else {
        // cannot set unconditionally, since d could be shared_null or
        // otherwise static.
        d->capacityReserved = false;
    }
}

namespace CXPrivate {
namespace DeprecatedRefClassBehavior {
    enum class EmittingClass {
        CXByteRef,
        QCharRef,
    };

    enum class WarningType {
        OutOfRange,
        DelayedDetach,
    };

    C_SYMBOL_EXPORT void warn(WarningType w, EmittingClass c);
}
}

class
C_SYMBOL_EXPORT
CXByteRef
{
    CXByteArray &a;
    int i;
    inline CXByteRef(CXByteArray &array, int idx)
        : a(array),i(idx) {}
    friend class CXByteArray;
public:
    CXByteRef(const CXByteRef &) = default;
    inline operator char() const
    {
        using namespace CXPrivate::DeprecatedRefClassBehavior;
        if (C_LIKELY(i < a.d->size))
            return a.d->data()[i];
        return char(0);
    }
    inline CXByteRef &operator=(char c)
    {
        using namespace CXPrivate::DeprecatedRefClassBehavior;
        if (C_UNLIKELY(i >= a.d->size)) {
            a.expand(i);
        } else {
            a.detach();
        }
        a.d->data()[i] = c;
        return *this;
    }
    inline CXByteRef &operator=(const CXByteRef &c)
    {
        return operator=(char(c));
    }
    inline bool operator==(char c) const
    { return a.d->data()[i] == c; }
    inline bool operator!=(char c) const
    { return a.d->data()[i] != c; }
    inline bool operator>(char c) const
    { return a.d->data()[i] > c; }
    inline bool operator>=(char c) const
    { return a.d->data()[i] >= c; }
    inline bool operator<(char c) const
    { return a.d->data()[i] < c; }
    inline bool operator<=(char c) const
    { return a.d->data()[i] <= c; }
};

inline CXByteRef CXByteArray::operator[](int i)
{ C_ASSERT(i >= 0); detach(); return CXByteRef(*this, i); }
inline CXByteRef CXByteArray::operator[](uint i)
{  detach(); return CXByteRef(*this, i); }
inline CXByteRef CXByteArray::front() { return operator[](0); }
inline CXByteRef CXByteArray::back() { return operator[](size() - 1); }
inline CXByteArray::iterator CXByteArray::begin()
{ detach(); return d->data(); }
inline CXByteArray::const_iterator CXByteArray::begin() const
{ return d->data(); }
inline CXByteArray::const_iterator CXByteArray::cbegin() const
{ return d->data(); }
inline CXByteArray::const_iterator CXByteArray::constBegin() const
{ return d->data(); }
inline CXByteArray::iterator CXByteArray::end()
{ detach(); return d->data() + d->size; }
inline CXByteArray::const_iterator CXByteArray::end() const
{ return d->data() + d->size; }
inline CXByteArray::const_iterator CXByteArray::cend() const
{ return d->data() + d->size; }
inline CXByteArray::const_iterator CXByteArray::constEnd() const
{ return d->data() + d->size; }
inline CXByteArray &CXByteArray::append(int n, char ch)
{ return insert(d->size, n, ch); }
inline CXByteArray &CXByteArray::prepend(int n, char ch)
{ return insert(0, n, ch); }
inline CXByteArray &CXByteArray::operator+=(char c)
{ return append(c); }
inline CXByteArray &CXByteArray::operator+=(const char *s)
{ return append(s); }
inline CXByteArray &CXByteArray::operator+=(const CXByteArray &a)
{ return append(a); }
inline void CXByteArray::push_back(char c)
{ append(c); }
inline void CXByteArray::push_back(const char *c)
{ append(c); }
inline void CXByteArray::push_back(const CXByteArray &a)
{ append(a); }
inline void CXByteArray::push_front(char c)
{ prepend(c); }
inline void CXByteArray::push_front(const char *c)
{ prepend(c); }
inline void CXByteArray::push_front(const CXByteArray &a)
{ prepend(a); }
inline bool CXByteArray::contains(const CXByteArray &a) const
{ return indexOf(a) != -1; }
inline bool CXByteArray::contains(char c) const
{ return indexOf(c) != -1; }
inline int CXByteArray::compare(const char *c, cx::CaseSensitivity cs) const noexcept
{
    return cs == cx::CaseSensitive ? cx_strcmp(*this, c) :
                                     cx_strnicmp(data(), size(), c, -1);
}
inline int CXByteArray::compare(const CXByteArray &a, cx::CaseSensitivity cs) const noexcept
{
    return cs == cx::CaseSensitive ? cx_strcmp(*this, a) :
                                     cx_strnicmp(data(), size(), a.data(), a.size());
}
inline bool operator==(const CXByteArray &a1, const CXByteArray &a2) noexcept
{ return (a1.size() == a2.size()) && (memcmp(a1.constData(), a2.constData(), a1.size())==0); }
inline bool operator==(const CXByteArray &a1, const char *a2) noexcept
{ return a2 ? cx_strcmp(a1,a2) == 0 : a1.isEmpty(); }
inline bool operator==(const char *a1, const CXByteArray &a2) noexcept
{ return a1 ? cx_strcmp(a1,a2) == 0 : a2.isEmpty(); }
inline bool operator!=(const CXByteArray &a1, const CXByteArray &a2) noexcept
{ return !(a1==a2); }
inline bool operator!=(const CXByteArray &a1, const char *a2) noexcept
{ return a2 ? cx_strcmp(a1,a2) != 0 : !a1.isEmpty(); }
inline bool operator!=(const char *a1, const CXByteArray &a2) noexcept
{ return a1 ? cx_strcmp(a1,a2) != 0 : !a2.isEmpty(); }
inline bool operator<(const CXByteArray &a1, const CXByteArray &a2) noexcept
{ return cx_strcmp(a1, a2) < 0; }
 inline bool operator<(const CXByteArray &a1, const char *a2) noexcept
{ return cx_strcmp(a1, a2) < 0; }
inline bool operator<(const char *a1, const CXByteArray &a2) noexcept
{ return cx_strcmp(a1, a2) < 0; }
inline bool operator<=(const CXByteArray &a1, const CXByteArray &a2) noexcept
{ return cx_strcmp(a1, a2) <= 0; }
inline bool operator<=(const CXByteArray &a1, const char *a2) noexcept
{ return cx_strcmp(a1, a2) <= 0; }
inline bool operator<=(const char *a1, const CXByteArray &a2) noexcept
{ return cx_strcmp(a1, a2) <= 0; }
inline bool operator>(const CXByteArray &a1, const CXByteArray &a2) noexcept
{ return cx_strcmp(a1, a2) > 0; }
inline bool operator>(const CXByteArray &a1, const char *a2) noexcept
{ return cx_strcmp(a1, a2) > 0; }
inline bool operator>(const char *a1, const CXByteArray &a2) noexcept
{ return cx_strcmp(a1, a2) > 0; }
inline bool operator>=(const CXByteArray &a1, const CXByteArray &a2) noexcept
{ return cx_strcmp(a1, a2) >= 0; }
inline bool operator>=(const CXByteArray &a1, const char *a2) noexcept
{ return cx_strcmp(a1, a2) >= 0; }
inline bool operator>=(const char *a1, const CXByteArray &a2) noexcept
{ return cx_strcmp(a1, a2) >= 0; }
inline const CXByteArray operator+(const CXByteArray &a1, const CXByteArray &a2)
{ return CXByteArray(a1) += a2; }
inline const CXByteArray operator+(const CXByteArray &a1, const char *a2)
{ return CXByteArray(a1) += a2; }
inline const CXByteArray operator+(const CXByteArray &a1, char a2)
{ return CXByteArray(a1) += a2; }
inline const CXByteArray operator+(const char *a1, const CXByteArray &a2)
{ return CXByteArray(a1) += a2; }
inline const CXByteArray operator+(char a1, const CXByteArray &a2)
{ return CXByteArray(&a1, 1) += a2; }
inline bool CXByteArray::contains(const char *c) const
{ return indexOf(c) != -1; }
inline CXByteArray &CXByteArray::replace(char before, const char *c)
{ return replace(&before, 1, c, cx_strlen(c)); }
inline CXByteArray &CXByteArray::replace(const CXByteArray &before, const char *c)
{ return replace(before.constData(), before.size(), c, cx_strlen(c)); }
inline CXByteArray &CXByteArray::replace(const char *before, const char *after)
{ return replace(before, cx_strlen(before), after, cx_strlen(after)); }

inline CXByteArray &CXByteArray::setNum(short n, int base)
{ return base == 10 ? setNum(clonglong(n), base) : setNum(culonglong(cushort(n)), base); }
inline CXByteArray &CXByteArray::setNum(cushort n, int base)
{ return setNum(culonglong(n), base); }
inline CXByteArray &CXByteArray::setNum(int n, int base)
{ return base == 10 ? setNum(clonglong(n), base) : setNum(culonglong(cuint(n)), base); }
inline CXByteArray &CXByteArray::setNum(cuint n, int base)
{ return setNum(culonglong(n), base); }
inline CXByteArray &CXByteArray::setNum(float n, char f, int prec)
{ return setNum(double(n),f,prec); }

inline std::string CXByteArray::toStdString() const
{ return std::string(constData(), length()); }

inline CXByteArray CXByteArray::fromStdString(const std::string &s)
{ return CXByteArray(s.data(), int(s.size())); }

C_SYMBOL_EXPORT CXDataStream &operator<<(CXDataStream &, const CXByteArray &);
C_SYMBOL_EXPORT CXDataStream &operator>>(CXDataStream &, CXByteArray &);

C_SYMBOL_EXPORT CXByteArray cxCompress(const cuchar* data, int nbytes, int compressionLevel = -1);
C_SYMBOL_EXPORT CXByteArray cxUncompress(const cuchar* data, int nbytes);
inline CXByteArray cxCompress(const CXByteArray& data, int compressionLevel = -1)
{ return cxCompress(reinterpret_cast<const cuchar *>(data.constData()), data.size(), compressionLevel); }
inline CXByteArray cxUncompress(const CXByteArray& data)
{ return cxUncompress(reinterpret_cast<const cuchar*>(data.constData()), data.size()); }

CX_DECLARE_SHARED(CXByteArray)

class CXByteArray::FromBase64Result
{
public:
    CXByteArray decoded;
    CXByteArray::Base64DecodingStatus decodingStatus;

    void swap(CXByteArray::FromBase64Result &other) noexcept
    {
        C_SWAP(decoded, other.decoded);
        C_SWAP(decodingStatus, other.decodingStatus);
    }

    explicit operator bool() const noexcept { return decodingStatus == CXByteArray::Base64DecodingStatus::Ok; }

    CXByteArray &operator*() noexcept { return decoded; }
    const CXByteArray &operator*() const noexcept { return decoded; }
};

CX_DECLARE_SHARED(CXByteArray::FromBase64Result)

inline bool operator==(const CXByteArray::FromBase64Result &lhs, const CXByteArray::FromBase64Result &rhs) noexcept
{
    if (lhs.decodingStatus != rhs.decodingStatus)
        return false;

    if (lhs.decodingStatus == CXByteArray::Base64DecodingStatus::Ok && lhs.decoded != rhs.decoded)
        return false;

    return true;
}

inline bool operator!=(const CXByteArray::FromBase64Result &lhs, const CXByteArray::FromBase64Result &rhs) noexcept
{
    return !operator==(lhs, rhs);
}

C_SYMBOL_EXPORT cuint qHash(const CXByteArray::FromBase64Result &key, cuint seed = 0) noexcept;


CX_END_NAMESPACE

#endif // clibrary_BYTEARRAY_H
