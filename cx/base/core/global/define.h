//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_DEFINE_H
#define clibrary_DEFINE_H

#ifdef _MSC_VER
#define CX_SUPPORTS(FEATURE) (!defined CX_NO_##FEATURE)
#else
#define CX_SUPPORTS(FEATURE) (!defined(CX_NO_##FEATURE))
#endif

#define CX_VERSION                                  CX_VERSION_CHECK(PROJECT_VERSION_MAJOR,PROJECT_VERSION_MINOR,PROJECT_VERSION_PATCH)
#define CX_VERSION_CHECK(major, minor, patch)       ((major<<16)|(minor<<8)|(patch))

#define CX_BEGIN_NAMESPACE namespace cx {
#define CX_END_NAMESPACE }

#endif // clibrary_DEFINE_H
