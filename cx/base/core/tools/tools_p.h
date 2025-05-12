//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_TOOLS_P_H
#define clibrary_TOOLS_P_H
#include "3thrd/macros/macros.h"
#include "c/macros.h"
#include "cx/base/core/global/global.h"

CX_BEGIN_NAMESPACE

namespace QtMiscUtils
{
    C_DECL_CONSTEXPR inline char toHexUpper(cuint value) noexcept
    {
        return "0123456789ABCDEF"[value & 0xF];
    }

    C_DECL_CONSTEXPR inline char toHexLower(cuint value) noexcept
    {
        return "0123456789abcdef"[value & 0xF];
    }

    C_DECL_CONSTEXPR inline int fromHex(cuint c) noexcept
    {
        return ((c >= '0') && (c <= '9')) ? int(c - '0') :
               ((c >= 'A') && (c <= 'F')) ? int(c - 'A' + 10) :
               ((c >= 'a') && (c <= 'f')) ? int(c - 'a' + 10) :
               /* otherwise */              -1;
    }

    C_DECL_CONSTEXPR inline char toOct(cuint value) noexcept
    {
        return '0' + char(value & 0x7);
    }

    C_DECL_CONSTEXPR inline int fromOct(cuint c) noexcept
    {
        return ((c >= '0') && (c <= '7')) ? int(c - '0') : -1;
    }

    constexpr inline char toAsciiLower(char ch) noexcept
    {
        return (ch >= 'A' && ch <= 'Z') ? ch - 'A' + 'a' : ch;
    }

    constexpr inline char toAsciiUpper(char ch) noexcept
    {
        return (ch >= 'a' && ch <= 'z') ? ch - 'a' + 'A' : ch;
    }

}

// We typically need an extra bit for qNextPowerOfTwo when determining the next allocation size.
enum {
    MaxAllocSize = C_MAX_INT32
};

struct CalculateGrowingBlockSizeResult {
    size_t size;
    size_t elementCount;
};

size_t C_SYMBOL_EXPORT C_DECL_CONST_FUNCTION
cxCalculateBlockSize(size_t elementCount, size_t elementSize, size_t headerSize = 0) noexcept;
CalculateGrowingBlockSizeResult C_SYMBOL_EXPORT C_DECL_CONST_FUNCTION
cxCalculateGrowingBlockSize(size_t elementCount, size_t elementSize, size_t headerSize = 0) noexcept ;

CX_END_NAMESPACE

#endif // clibrary_TOOLS_P_H
