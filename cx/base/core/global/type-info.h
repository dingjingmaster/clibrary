//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_CX_TYPE_INFO_H
#define clibrary_CX_TYPE_INFO_H
#include "define.h"

#include "3thrd/macros/macros.h"

#include <type_traits>

CX_BEGIN_NAMESPACE

template <typename T>
static constexpr bool isRelocatable()
{
    return std::is_enum<T>::value || std::is_integral<T>::value;
}

template <typename T>
static constexpr bool isTrivial()
{
    return std::is_enum<T>::value || std::is_integral<T>::value;
}

template <typename T>
class CXTypeInfo
{
public:
    enum
    {
        isSpecialized = std::is_enum<T>::value, // don't require every enum to be marked manually
        isPointer = false,
        isIntegral = std::is_integral<T>::value,
        isComplex = !isTrivial<T>(),
        isStatic = true,
        isRelocatable = isRelocatable<T>(),
        isLarge = (sizeof(T)>sizeof(void*)),
        isDummy = false,
        sizeOf = sizeof(T)
    };
};

template<>
class CXTypeInfo<void>
{
public:
    enum
    {
        isSpecialized = true,
        isPointer = false,
        isIntegral = false,
        isComplex = false,
        isStatic = false,
        isRelocatable = false,
        isLarge = false,
        isDummy = false,
        sizeOf = 0
    };
};

template <typename T>
class CXTypeInfo<T*>
{
public:
    enum
    {
        isSpecialized = true,
        isPointer = true,
        isIntegral = false,
        isComplex = false,
        isStatic = false,
        isRelocatable = true,
        isLarge = false,
        isDummy = false,
        sizeOf = sizeof(T*)
    };
};

template <typename T, typename = void>
struct CXTypeInfoQuery : public CXTypeInfo<T>
{
    enum { isRelocatable = !CXTypeInfo<T>::isStatic };
};

template <typename T>
struct CXTypeInfoQuery<T, typename std::enable_if<CXTypeInfo<T>::isRelocatable || true>::type> : public CXTypeInfo<T>
{

};

template <class T, class T1, class T2 = T1, class T3 = T1, class T4 = T1>
class CXTypeInfoMerger
{
public:
    enum
    {
        isSpecialized = true,
        isComplex = CXTypeInfoQuery<T1>::isComplex
                    || CXTypeInfoQuery<T2>::isComplex
                    || CXTypeInfoQuery<T3>::isComplex
                    || CXTypeInfoQuery<T4>::isComplex,
        isStatic = CXTypeInfoQuery<T1>::isStatic
                    || CXTypeInfoQuery<T2>::isStatic
                    || CXTypeInfoQuery<T3>::isStatic
                    || CXTypeInfoQuery<T4>::isStatic,
        isRelocatable = CXTypeInfoQuery<T1>::isRelocatable
                    && CXTypeInfoQuery<T2>::isRelocatable
                    && CXTypeInfoQuery<T3>::isRelocatable
                    && CXTypeInfoQuery<T4>::isRelocatable,
        isLarge = sizeof(T) > sizeof(void*),
        isPointer = false,
        isIntegral = false,
        isDummy = false,
        sizeOf = sizeof(T)
    };
};

#define CX_DECLARE_MOVABLE_CONTAINER(CONTAINER) \
    template <typename T> class CONTAINER; \
    template <typename T> \
    class CXTypeInfo< CONTAINER<T> > \
    { \
    public: \
        enum { \
            isSpecialized = true, \
            isPointer = false, \
            isIntegral = false, \
            isComplex = true, \
            isRelocatable = true, \
            isStatic = false, \
            isLarge = (sizeof(CONTAINER<T>) > sizeof(void*)), \
            isDummy = false, \
            sizeOf = sizeof(CONTAINER<T>) \
        }; \
    }

CX_DECLARE_MOVABLE_CONTAINER(CXList);
CX_DECLARE_MOVABLE_CONTAINER(CXVector);
CX_DECLARE_MOVABLE_CONTAINER(CXQueue);
CX_DECLARE_MOVABLE_CONTAINER(CXStack);
CX_DECLARE_MOVABLE_CONTAINER(CXSet);

#undef CX_DECLARE_MOVABLE_CONTAINER

#define CX_DECLARE_MOVABLE_CONTAINER(CONTAINER) \
    template <typename K, typename V> class CONTAINER; \
    template <typename K, typename V> \
    class CXTypeInfo< CONTAINER<K, V> > \
    { \
    public: \
        enum { \
            isSpecialized = true, \
            isPointer = false, \
            isIntegral = false, \
            isComplex = true, \
            isStatic = true, \
            isRelocatable = true, \
            isLarge = (sizeof(CONTAINER<K, V>) > sizeof(void*)), \
            isDummy = false, \
            sizeOf = sizeof(CONTAINER<K, V>) \
        }; \
    }

CX_DECLARE_MOVABLE_CONTAINER(CXMap);
CX_DECLARE_MOVABLE_CONTAINER(CXMultiMap);
CX_DECLARE_MOVABLE_CONTAINER(CXHash);
CX_DECLARE_MOVABLE_CONTAINER(CXMultiHash);

#undef CX_DECLARE_MOVABLE_CONTAINER

enum
{
    /* TYPEINFO flags */
    CX_COMPLEX_TYPE = 0,
    CX_PRIMITIVE_TYPE = 0x1,
    CX_STATIC_TYPE = 0,
    CX_MOVABLE_TYPE = 0x2,
    CX_DUMMY_TYPE = 0x4,
    CX_RELOCATABLE_TYPE = 0x8
};

#define CX_DECLARE_TYPEINFO_BODY(TYPE, FLAGS) \
    class CXTypeInfo<TYPE > \
    { \
    public: \
        enum { \
            isSpecialized = true, \
            isComplex = (((FLAGS) & CX_PRIMITIVE_TYPE) == 0) && !isTrivial<TYPE>(), \
            isStatic = (((FLAGS) & (CX_MOVABLE_TYPE | CX_PRIMITIVE_TYPE)) == 0), \
            isRelocatable = !isStatic || ((FLAGS) & CX_RELOCATABLE_TYPE) || isRelocatable<TYPE>(), \
            isLarge = (sizeof(TYPE)>sizeof(void*)), \
            isPointer = false, \
            isIntegral = std::is_integral< TYPE >::value, \
            isDummy = (((FLAGS) & CX_DUMMY_TYPE) != 0), \
            sizeOf = sizeof(TYPE) \
        }; \
        static inline const char *name() { return #TYPE; } \
    }

#define CX_DECLARE_TYPEINFO(TYPE, FLAGS) \
template<> \
CX_DECLARE_TYPEINFO_BODY(TYPE, FLAGS)

/* Specialize QTypeInfo for QFlags<T> */
template<typename T> class CXFlags;
template<typename T>
CX_DECLARE_TYPEINFO_BODY(CXFlags<T>, CX_PRIMITIVE_TYPE);

#define CX_DECLARE_SHARED_IMPL(TYPE, FLAGS) \
    CX_DECLARE_TYPEINFO(TYPE, FLAGS); \
    inline void swap(TYPE &value1, TYPE &value2) \
    noexcept(noexcept(value1.swap(value2))) \
    { value1.swap(value2); }
#define CX_DECLARE_SHARED(TYPE) CX_DECLARE_SHARED_IMPL(TYPE, CX_MOVABLE_TYPE)

CX_DECLARE_TYPEINFO(bool, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(char, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(signed char, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(cuchar, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(cshort, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(cushort, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(cint, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(cuint, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(clong, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(culong, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(cint64, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(cuint64, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(cfloat, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(cdouble, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(long double, CX_PRIMITIVE_TYPE);
CX_DECLARE_TYPEINFO(char16_t, CX_RELOCATABLE_TYPE);
CX_DECLARE_TYPEINFO(char32_t, CX_RELOCATABLE_TYPE);
CX_DECLARE_TYPEINFO(wchar_t, CX_RELOCATABLE_TYPE);

CX_END_NAMESPACE

#endif // clibrary_CX_TYPE_INFO_H
