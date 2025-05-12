//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_BYTEARRAY_LIST_H
#define clibrary_BYTEARRAY_LIST_H

#include "bytearray.h"

CX_BEGIN_NAMESPACE

typedef CXListIterator<CXByteArray> QByteArrayListIterator;
typedef CXMutableListIterator<CXByteArray> QMutableByteArrayListIterator;

typedef CXList<CXByteArray> CXByteArrayList;

namespace CXPrivate {
    CXByteArray C_SYMBOL_EXPORT CXByteArrayList_join(const CXByteArrayList *that, const char *separator, int separatorLength);
    int C_SYMBOL_EXPORT CXByteArrayList_indexOf(const CXByteArrayList *that, const char *needle, int from);
}

template <> struct CXListSpecialMethods<CXByteArray>
{
protected:
    ~CXListSpecialMethods() = default;
public:
    inline CXByteArray join() const
    { return CXPrivate::CXByteArrayList_join(self(), nullptr, 0); }
    inline CXByteArray join(const CXByteArray &sep) const
    { return CXPrivate::CXByteArrayList_join(self(), sep.constData(), sep.size()); }
    inline CXByteArray join(char sep) const
    { return CXPrivate::CXByteArrayList_join(self(), &sep, 1); }

    inline int indexOf(const char *needle, int from = 0) const
    { return CXPrivate::CXByteArrayList_indexOf(self(), needle, from); }

private:
    typedef CXList<CXByteArray> Self;
    Self *self() { return static_cast<Self*>(this); }
    const Self *self() const { return static_cast<const Self *>(this); }
};


CX_END_NAMESPACE


#endif // clibrary_BYTEARRAY_LIST_H
