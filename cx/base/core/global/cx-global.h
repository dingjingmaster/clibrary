//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_CX_GLOBAL_H
#define clibrary_CX_GLOBAL_H
#include "3thrd/macros/macros.h"

#ifdef __cplusplus
#include <stddef.h>
#include <utility>
#endif

#ifndef __ASSEMBLER__
#include <assert.h>
#include <stddef.h>
#endif

#ifdef CX_BOOTSTRAPPED
#include "cx-config-bootstrapped.h"
#else
#include <CxCore/cx-config.h>
#include <CxCoew/cxcore-config.h>
#endif



#ifdef _MSC_VER
#define CX_SUPPORTS(FEATURE) (!defined CX_NO_##FEATURE)
#else
#define CX_SUPPORTS(FEATURE) (!defined(CX_NO_##FEATURE))
#endif

#define CX_CONFIG(feature) (1/CX_FEATURE_##feature == 1)
#define CX_REQUIRE_CONFIG(feature) C_STATIC_ASSERT_X(CX_FEATURE_##feature == 1, "Required feature " \
    #feature " for file " __FILE__ " not available.")

#define CX_STRINGIFY2(x)    #x
#define CX_STRINGIFY(x)     CX_STRINGIFY2(x)

#if defined (__ELF__)
#  define CX_OF_ELF
#endif
#if defined (__MACH__) && defined (__APPLE__)
#  define CX_OF_MACH_O
#endif

#define CX_UNUSED(x) (void)x;

#ifdef __cplusplus

#include <algorithm>

#if !defined(CX_NAMESPACE) || defined(CX_MOC_RUN) /* user namespace */

# define CX_PREPEND_NAMESPACE(name) ::name
# define CX_USE_NAMESPACE
# define CX_BEGIN_NAMESPACE
# define CX_END_NAMESPACE
# define CX_BEGIN_INCLUDE_NAMESPACE
# define CX_END_INCLUDE_NAMESPACE
#ifndef CX_BEGIN_MOC_NAMESPACE
# define CX_BEGIN_MOC_NAMESPACE
#endif
#ifndef CX_END_MOC_NAMESPACE
# define CX_END_MOC_NAMESPACE
#endif
# define CX_FORWARD_DECLARE_CLASS(name) class name;
# define CX_FORWARD_DECLARE_STRUCT(name) struct name;
# define CX_MANGLE_NAMESPACE(name) name

#else /* user namespace */

# define CX_PREPEND_NAMESPACE(name) ::CX_NAMESPACE::name
# define CX_USE_NAMESPACE using namespace ::CX_NAMESPACE;
# define CX_BEGIN_NAMESPACE namespace CX_NAMESPACE {
# define CX_END_NAMESPACE }
# define CX_BEGIN_INCLUDE_NAMESPACE }
# define CX_END_INCLUDE_NAMESPACE namespace CX_NAMESPACE {
#ifndef CX_BEGIN_MOC_NAMESPACE
# define CX_BEGIN_MOC_NAMESPACE CX_USE_NAMESPACE
#endif
#ifndef CX_END_MOC_NAMESPACE
# define CX_END_MOC_NAMESPACE
#endif
# define CX_FORWARD_DECLARE_CLASS(name) \
    CX_BEGIN_NAMESPACE class name; CX_END_NAMESPACE \
    using CX_PREPEND_NAMESPACE(name);

# define CX_FORWARD_DECLARE_STRUCT(name) \
    CX_BEGIN_NAMESPACE struct name; CX_END_NAMESPACE \
    using CX_PREPEND_NAMESPACE(name);

# define CX_MANGLE_NAMESPACE0(x) x
# define CX_MANGLE_NAMESPACE1(a, b) a##_##b
# define CX_MANGLE_NAMESPACE2(a, b) CX_MANGLE_NAMESPACE1(a,b)
# define CX_MANGLE_NAMESPACE(name) CX_MANGLE_NAMESPACE2( \
        CX_MANGLE_NAMESPACE0(name), CX_MANGLE_NAMESPACE0(CX_NAMESPACE))

namespace CX_NAMESPACE {}

# ifndef CX_BOOTSTRAPPED
# ifndef CX_NO_USING_NAMESPACE
   CX_USE_NAMESPACE
# endif
# endif

#endif /* user namespace */

#else /* __cplusplus */

# define CX_BEGIN_NAMESPACE
# define CX_END_NAMESPACE
# define CX_USE_NAMESPACE
# define CX_BEGIN_INCLUDE_NAMESPACE
# define CX_END_INCLUDE_NAMESPACE

#endif /* __cplusplus */

#define CX_BEGIN_HEADER
#define CX_END_HEADER

#if defined(CX_OS_DARWIN) && !defined(CX_LARGEFILE_SUPPORT)
#  define CX_LARGEFILE_SUPPORT 64
#endif

#ifndef __ASSEMBLER__
CX_BEGIN_NAMESPACE

#if defined(CX_OS_WIN) && !defined(CX_CC_GNU)
#  define CX_INT64_C(c) c ## i64    /* signed 64 bit constant */
#  define CX_UINT64_C(c) c ## ui64   /* unsigned 64 bit constant */
#else
#ifdef __cplusplus
#  define CX_INT64_C(c) static_cast<long long>(c ## LL)     /* signed 64 bit constant */
#  define CX_UINT64_C(c) static_cast<unsigned long long>(c ## ULL) /* unsigned 64 bit constant */
#else
#  define CX_INT64_C(c) ((long long)(c ## LL))               /* signed 64 bit constant */
#  define CX_UINT64_C(c) ((unsigned long long)(c ## ULL))    /* unsigned 64 bit constant */
#endif
#endif

typedef cint64  clonglong;
typedef cuint64 culonglong;



/*
   Useful type definitions for Qt
*/

CX_BEGIN_INCLUDE_NAMESPACE
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
CX_END_INCLUDE_NAMESPACE

#if defined(CX_COORD_TYPE)
typedef CX_COORD_TYPE cxreal;
#else
typedef double cxreal;
#endif

#if defined(CX_NO_DEPRECATED)
#  undef CX_DEPRECATED
#  undef CX_DEPRECATED_X
#  undef CX_DEPRECATED_VARIABLE
#  undef CX_DEPRECATED_CONSTRUCTOR
#elif !defined(CX_NO_DEPRECATED_WARNINGS)
#  undef CX_DEPRECATED
#  define CX_DEPRECATED CX_DECL_DEPRECATED
#  undef CX_DEPRECATED_X
#  define CX_DEPRECATED_X(text) CX_DECL_DEPRECATED_X(text)
#  undef CX_DEPRECATED_VARIABLE
#  define CX_DEPRECATED_VARIABLE CX_DECL_VARIABLE_DEPRECATED
#  undef CX_DEPRECATED_CONSTRUCTOR
#  define CX_DEPRECATED_CONSTRUCTOR explicit CX_DECL_CONSTRUCTOR_DEPRECATED
#else
#  undef CX_DEPRECATED
#  define CX_DEPRECATED
#  undef CX_DEPRECATED_X
#  define CX_DEPRECATED_X(text)
#  undef CX_DEPRECATED_VARIABLE
#  define CX_DEPRECATED_VARIABLE
#  undef CX_DEPRECATED_CONSTRUCTOR
#  define CX_DEPRECATED_CONSTRUCTOR
#  undef CX_DECL_ENUMERATOR_DEPRECATED
#  define CX_DECL_ENUMERATOR_DEPRECATED
#  undef CX_DECL_ENUMERATOR_DEPRECATED_X
#  define CX_DECL_ENUMERATOR_DEPRECATED_X(ignored)
#endif

#ifndef CX_DEPRECATED_WARNINGS_SINCE
# ifdef CX_DISABLE_DEPRECATED_BEFORE
#  define CX_DEPRECATED_WARNINGS_SINCE CX_DISABLE_DEPRECATED_BEFORE
# else
#  define CX_DEPRECATED_WARNINGS_SINCE CX_VERSION
# endif
#endif

#ifndef CX_DISABLE_DEPRECATED_BEFORE
#define CX_DISABLE_DEPRECATED_BEFORE CX_VERSION_CHECK(5, 0, 0)
#endif

#ifdef CX_DEPRECATED
#define CX_DEPRECATED_SINCE(major, minor) (CX_VERSION_CHECK(major, minor, 0) > CX_DISABLE_DEPRECATED_BEFORE)
#else
#define CX_DEPRECATED_SINCE(major, minor) 0
#endif

/*
  CX_DEPRECATED_VERSION(major, minor) and CX_DEPRECATED_VERSION_X(major, minor, text)
  outputs a deprecation warning if CX_DEPRECATED_WARNINGS_SINCE is equal or greater
  than the version specified as major, minor. This makes it possible to deprecate a
  function without annoying a user who needs to stick at a specified minimum version
  and therefore can't use the new function.
*/
#if CX_DEPRECATED_WARNINGS_SINCE >= CX_VERSION_CHECK(5, 12, 0)
# define CX_DEPRECATED_VERSION_X_5_12(text) CX_DEPRECATED_X(text)
# define CX_DEPRECATED_VERSION_5_12         CX_DEPRECATED
#else
# define CX_DEPRECATED_VERSION_X_5_12(text)
# define CX_DEPRECATED_VERSION_5_12
#endif

#if CX_DEPRECATED_WARNINGS_SINCE >= CX_VERSION_CHECK(5, 13, 0)
# define CX_DEPRECATED_VERSION_X_5_13(text) CX_DEPRECATED_X(text)
# define CX_DEPRECATED_VERSION_5_13         CX_DEPRECATED
#else
# define CX_DEPRECATED_VERSION_X_5_13(text)
# define CX_DEPRECATED_VERSION_5_13
#endif

#if CX_DEPRECATED_WARNINGS_SINCE >= CX_VERSION_CHECK(5, 14, 0)
# define CX_DEPRECATED_VERSION_X_5_14(text) CX_DEPRECATED_X(text)
# define CX_DEPRECATED_VERSION_5_14         CX_DEPRECATED
#else
# define CX_DEPRECATED_VERSION_X_5_14(text)
# define CX_DEPRECATED_VERSION_5_14
#endif

#if CX_DEPRECATED_WARNINGS_SINCE >= CX_VERSION_CHECK(5, 15, 0)
# define CX_DEPRECATED_VERSION_X_5_15(text) CX_DEPRECATED_X(text)
# define CX_DEPRECATED_VERSION_5_15         CX_DEPRECATED
#else
# define CX_DEPRECATED_VERSION_X_5_15(text)
# define CX_DEPRECATED_VERSION_5_15
#endif

#if CX_DEPRECATED_WARNINGS_SINCE >= CX_VERSION_CHECK(6, 0, 0)
# define CX_DEPRECATED_VERSION_X_6_0(text) CX_DEPRECATED_X(text)
# define CX_DEPRECATED_VERSION_6_0         CX_DEPRECATED
#else
# define CX_DEPRECATED_VERSION_X_6_0(text)
# define CX_DEPRECATED_VERSION_6_0
#endif

#define CX_DEPRECATED_VERSION_X_5(minor, text)      CX_DEPRECATED_VERSION_X_5_##minor(text)
#define CX_DEPRECATED_VERSION_X(major, minor, text) CX_DEPRECATED_VERSION_X_##major##_##minor(text)

#define CX_DEPRECATED_VERSION_5(minor)      CX_DEPRECATED_VERSION_5_##minor
#define CX_DEPRECATED_VERSION(major, minor) CX_DEPRECATED_VERSION_##major##_##minor



#ifdef CX_BOOTSTRAPPED
#  ifdef CX_SHARED
#    error "CX_SHARED and CX_BOOTSTRAPPED together don't make sense. Please fix the build"
#  elif !defined(CX_STATIC)
#    define CX_STATIC
#  endif
#endif

#if defined(CX_SHARED) || !defined(CX_STATIC)
#  ifdef CX_STATIC
#    error "Both CX_SHARED and CX_STATIC defined, please make up your mind"
#  endif
#  ifndef CX_SHARED
#    define CX_SHARED
#  endif
#  if defined(CX_BUILD_CORE_LIB)
#    define CX_CORE_EXPORT CX_DECL_EXPORT
#  else
#    define CX_CORE_EXPORT CX_DECL_IMPORT
#  endif
#else
#  define CX_CORE_EXPORT
#endif

#define CX_DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete;

#define CX_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define CX_DISABLE_COPY_MOVE(Class) \
    CX_DISABLE_COPY(Class) \
    CX_DISABLE_MOVE(Class)

#if defined(CX_BUILD_INTERNAL) && defined(CX_BUILDING_QT) && defined(CX_SHARED)
#    define CX_AUTOTEST_EXPORT CX_DECL_EXPORT
#elif defined(CX_BUILD_INTERNAL) && defined(CX_SHARED)
#    define CX_AUTOTEST_EXPORT CX_DECL_IMPORT
#else
#    define CX_AUTOTEST_EXPORT
#endif

#define CX_INIT_RESOURCE(name) \
    do { extern int CX_MANGLE_NAMESPACE(cxInitResources_ ## name) ();       \
        CX_MANGLE_NAMESPACE(cxInitResources_ ## name) (); } while (false)
#define CX_CLEANUP_RESOURCE(name) \
    do { extern int CX_MANGLE_NAMESPACE(cxCleanupResources_ ## name) ();    \
        CX_MANGLE_NAMESPACE(cxCleanupResources_ ## name) (); } while (false)

#if !defined(CX_NAMESPACE) && defined(__cplusplus) && !defined(CX_QDOC)
extern "C"
#endif
CX_CORE_EXPORT C_DECL_CONST_FUNCTION const char *cxVersion(void) C_DECL_NOEXCEPT;

#if defined(__cplusplus)

#ifndef CX_CONSTRUCTOR_FUNCTION
# define CX_CONSTRUCTOR_FUNCTION0(AFUNC) \
    namespace { \
    static const struct AFUNC ## _ctor_class_ { \
        inline AFUNC ## _ctor_class_() { AFUNC(); } \
    } AFUNC ## _ctor_instance_; \
    }

# define CX_CONSTRUCTOR_FUNCTION(AFUNC) CX_CONSTRUCTOR_FUNCTION0(AFUNC)
#endif

#ifndef CX_DESTRUCTOR_FUNCTION
# define CX_DESTRUCTOR_FUNCTION0(AFUNC) \
    namespace { \
    static const struct AFUNC ## _dtor_class_ { \
        inline AFUNC ## _dtor_class_() { } \
        inline ~ AFUNC ## _dtor_class_() { AFUNC(); } \
    } AFUNC ## _dtor_instance_; \
    }
# define CX_DESTRUCTOR_FUNCTION(AFUNC) CX_DESTRUCTOR_FUNCTION0(AFUNC)
#endif


#define CX_EMULATED_ALIGNOF(T) \
    (size_t(CX_PREPEND_NAMESPACE(CxPrivate)::AlignOf<T>::Value))

#ifndef CX_ALIGNOF
#define CX_ALIGNOF(T) CX_EMULATED_ALIGNOF(T)
#endif


/* moc compats (signals/slots) */
#ifndef CX_MOC_COMPAT
#  define CX_MOC_COMPAT
#else
#  undef CX_MOC_COMPAT
#  define CX_MOC_COMPAT
#endif

#ifdef CX_ASCII_CAST_WARNINGS
#  define CX_ASCII_CAST_WARN CX_DECL_DEPRECATED_X("Use fromUtf8, QStringLiteral, or QLatin1String")
#else
#  define CX_ASCII_CAST_WARN
#endif

#ifdef CX_PROCESSOR_X86_32
#  if defined(CX_CC_GNU)
#    define CX_FASTCALL __attribute__((regparm(3)))
#  elif defined(CX_CC_MSVC)
#    define CX_FASTCALL __fastcall
#  else
#     define CX_FASTCALL
#  endif
#else
#  define CX_FASTCALL
#endif

// enable gcc warnings for printf-style functions
#if defined(CX_CC_GNU) && !defined(__INSURE__)
#  if defined(CX_CC_MINGW) && !defined(CX_CC_CLANG)
#    define CX_ATTRIBUTE_FORMAT_PRINTF(A, B) \
         __attribute__((format(gnu_printf, (A), (B))))
#  else
#    define CX_ATTRIBUTE_FORMAT_PRINTF(A, B) \
         __attribute__((format(printf, (A), (B))))
#  endif
#else
#  define CX_ATTRIBUTE_FORMAT_PRINTF(A, B)
#endif

#ifdef CX_CC_MSVC
#  define CX_NEVER_INLINE __declspec(noinline)
#  define CX_ALWAYS_INLINE __forceinline
#elif defined(CX_CC_GNU)
#  define CX_NEVER_INLINE __attribute__((noinline))
#  define CX_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#  define CX_NEVER_INLINE
#  define CX_ALWAYS_INLINE inline
#endif

#if defined(CX_CC_GNU) && defined(CX_OS_WIN) && !defined(CX_NO_DATA_RELOCATION)
#  define CX_INIT_METAOBJECT __attribute__((init_priority(101)))
#else
#  define CX_INIT_METAOBJECT
#endif

//defines the type for the WNDPROC on windows
//the alignment needs to be forced for sse2 to not crash with mingw
#if defined(CX_OS_WIN)
#  if defined(CX_CC_MINGW) && defined(CX_PROCESSOR_X86_32)
#    define CX_ENSURE_STACK_ALIGNED_FOR_SSE __attribute__ ((force_align_arg_pointer))
#  else
#    define CX_ENSURE_STACK_ALIGNED_FOR_SSE
#  endif
#  define CX_WIN_CALLBACK CALLBACK CX_ENSURE_STACK_ALIGNED_FOR_SSE
#endif

typedef int CXNoImplicitBoolCast;

template <typename T>
C_DECL_CONSTEXPR inline T cAbs(const T &t) { return t >= 0 ? t : -t; }

C_DECL_CONSTEXPR inline int cRound(double d)
{ return d >= 0.0 ? int(d + 0.5) : int(d - double(int(d-1)) + 0.5) + int(d-1); }
C_DECL_CONSTEXPR inline int cRound(float d)
{ return d >= 0.0f ? int(d + 0.5f) : int(d - float(int(d-1)) + 0.5f) + int(d-1); }

C_DECL_CONSTEXPR inline cint64 cRound64(double d)
{ return d >= 0.0 ? cint64(d + 0.5) : cint64(d - double(cint64(d-1)) + 0.5) + cint64(d-1); }
C_DECL_CONSTEXPR inline cint64 cRound64(float d)
{ return d >= 0.0f ? cint64(d + 0.5f) : cint64(d - float(cint64(d-1)) + 0.5f) + cint64(d-1); }

template <typename T>
constexpr inline const T &cMin(const T &a, const T &b) { return (a < b) ? a : b; }
template <typename T>
constexpr inline const T &cMax(const T &a, const T &b) { return (a < b) ? b : a; }
template <typename T>
constexpr inline const T &cBound(const T &min, const T &val, const T &max)
{ return qMax(min, qMin(max, val)); }

#ifndef CX_FORWARD_DECLARE_OBJC_CLASS
#  ifdef __OBJC__
#    define CX_FORWARD_DECLARE_OBJC_CLASS(classname) @class classname
#  else
#    define CX_FORWARD_DECLARE_OBJC_CLASS(classname) typedef struct objc_object classname
#  endif
#endif
#ifndef CX_FORWARD_DECLARE_CF_TYPE
#  define CX_FORWARD_DECLARE_CF_TYPE(type) typedef const struct __ ## type * type ## Ref
#endif
#ifndef CX_FORWARD_DECLARE_MUTABLE_CF_TYPE
#  define CX_FORWARD_DECLARE_MUTABLE_CF_TYPE(type) typedef struct __ ## type * type ## Ref
#endif
#ifndef CX_FORWARD_DECLARE_CG_TYPE
#define CX_FORWARD_DECLARE_CG_TYPE(type) typedef const struct type *type ## Ref;
#endif
#ifndef CX_FORWARD_DECLARE_MUTABLE_CG_TYPE
#define CX_FORWARD_DECLARE_MUTABLE_CG_TYPE(type) typedef struct type *type ## Ref;
#endif

#ifdef CX_OS_DARWIN
#  define CX_DARWIN_PLATFORM_SDK_EQUAL_OR_ABOVE(macos, ios, tvos, watchos) \
    ((defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && macos != __MAC_NA && __MAC_OS_X_VERSION_MAX_ALLOWED >= macos) || \
     (defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && ios != __IPHONE_NA && __IPHONE_OS_VERSION_MAX_ALLOWED >= ios) || \
     (defined(__TV_OS_VERSION_MAX_ALLOWED) && tvos != __TVOS_NA && __TV_OS_VERSION_MAX_ALLOWED >= tvos) || \
     (defined(__WATCH_OS_VERSION_MAX_ALLOWED) && watchos != __WATCHOS_NA && __WATCH_OS_VERSION_MAX_ALLOWED >= watchos))

#  define CX_DARWIN_DEPLOYMENT_TARGET_BELOW(macos, ios, tvos, watchos) \
    ((defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && macos != __MAC_NA && __MAC_OS_X_VERSION_MIN_REQUIRED < macos) || \
     (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && ios != __IPHONE_NA && __IPHONE_OS_VERSION_MIN_REQUIRED < ios) || \
     (defined(__TV_OS_VERSION_MIN_REQUIRED) && tvos != __TVOS_NA && __TV_OS_VERSION_MIN_REQUIRED < tvos) || \
     (defined(__WATCH_OS_VERSION_MIN_REQUIRED) && watchos != __WATCHOS_NA && __WATCH_OS_VERSION_MIN_REQUIRED < watchos))

#  define CX_MACOS_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(macos, ios) \
      CX_DARWIN_PLATFORM_SDK_EQUAL_OR_ABOVE(macos, ios, __TVOS_NA, __WATCHOS_NA)
#  define CX_MACOS_PLATFORM_SDK_EQUAL_OR_ABOVE(macos) \
      CX_DARWIN_PLATFORM_SDK_EQUAL_OR_ABOVE(macos, __IPHONE_NA, __TVOS_NA, __WATCHOS_NA)
#  define CX_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(ios) \
      CX_DARWIN_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_NA, ios, __TVOS_NA, __WATCHOS_NA)
#  define CX_TVOS_PLATFORM_SDK_EQUAL_OR_ABOVE(tvos) \
      CX_DARWIN_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_NA, __IPHONE_NA, tvos, __WATCHOS_NA)
#  define CX_WATCHOS_PLATFORM_SDK_EQUAL_OR_ABOVE(watchos) \
      CX_DARWIN_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_NA, __IPHONE_NA, __TVOS_NA, watchos)

#  define CX_MACOS_IOS_DEPLOYMENT_TARGET_BELOW(macos, ios) \
      CX_DARWIN_DEPLOYMENT_TARGET_BELOW(macos, ios, __TVOS_NA, __WATCHOS_NA)
#  define CX_MACOS_DEPLOYMENT_TARGET_BELOW(macos) \
      CX_DARWIN_DEPLOYMENT_TARGET_BELOW(macos, __IPHONE_NA, __TVOS_NA, __WATCHOS_NA)
#  define CX_IOS_DEPLOYMENT_TARGET_BELOW(ios) \
      CX_DARWIN_DEPLOYMENT_TARGET_BELOW(__MAC_NA, ios, __TVOS_NA, __WATCHOS_NA)
#  define CX_TVOS_DEPLOYMENT_TARGET_BELOW(tvos) \
      CX_DARWIN_DEPLOYMENT_TARGET_BELOW(__MAC_NA, __IPHONE_NA, tvos, __WATCHOS_NA)
#  define CX_WATCHOS_DEPLOYMENT_TARGET_BELOW(watchos) \
      CX_DARWIN_DEPLOYMENT_TARGET_BELOW(__MAC_NA, __IPHONE_NA, __TVOS_NA, watchos)

// Compatibility synonyms, do not use
#  define CX_MAC_PLATFORM_SDK_EQUAL_OR_ABOVE(osx, ios) CX_MACOS_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(osx, ios)
#  define CX_MAC_DEPLOYMENT_TARGET_BELOW(osx, ios) CX_MACOS_IOS_DEPLOYMENT_TARGET_BELOW(osx, ios)
#  define CX_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(osx) CX_MACOS_PLATFORM_SDK_EQUAL_OR_ABOVE(osx)
#  define CX_OSX_DEPLOYMENT_TARGET_BELOW(osx) CX_MACOS_DEPLOYMENT_TARGET_BELOW(osx)

// Implemented in qcore_mac_objc.mm
class CX_CORE_EXPORT CXMacAutoReleasePool
{
public:
    CXMacAutoReleasePool();
    ~CXMacAutoReleasePool();
private:
    CX_DISABLE_COPY(CXMacAutoReleasePool)
    void *pool;
};

#else

#define CX_DARWIN_PLATFORM_SDK_EQUAL_OR_ABOVE(macos, ios, tvos, watchos) (0)
#define CX_MACOS_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(macos, ios) (0)
#define CX_MACOS_PLATFORM_SDK_EQUAL_OR_ABOVE(macos) (0)
#define CX_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(ios) (0)
#define CX_TVOS_PLATFORM_SDK_EQUAL_OR_ABOVE(tvos) (0)
#define CX_WATCHOS_PLATFORM_SDK_EQUAL_OR_ABOVE(watchos) (0)

#define CX_MAC_PLATFORM_SDK_EQUAL_OR_ABOVE(osx, ios) (0)
#define CX_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(osx) (0)

#endif // CX_OS_DARWIN

/*
   Data stream functions are provided by many classes (defined in qdatastream.h)
*/

class CXDataStream;

inline void cx_noop(void) {}


#if !defined(CX_NO_EXCEPTIONS)
#  if !defined(CX_MOC_RUN)
#    if (defined(CX_CC_CLANG) && !defined(CX_CC_INTEL) && !__has_feature(cxx_exceptions)) || \
        (defined(CX_CC_GNU) && !defined(__EXCEPTIONS))
#      define CX_NO_EXCEPTIONS
#    endif
#  elif defined(CX_BOOTSTRAPPED)
#    define CX_NO_EXCEPTIONS
#  endif
#endif

#ifdef CX_NO_EXCEPTIONS
#  define CX_TRY if (true)
#  define CX_CATCH(A) else
#  define CX_THROW(A) qt_noop()
#  define CX_RETHROW qt_noop()
#  define CX_TERMINATE_ON_EXCEPTION(expr) do { expr; } while (false)
#else
#  define CX_TRY try
#  define CX_CATCH(A) catch (A)
#  define CX_THROW(A) throw A
#  define CX_RETHROW throw
CX_NORETURN CX_DECL_COLD_FUNCTION CX_CORE_EXPORT void cxTerminate() noexcept;
#  ifdef CX_COMPILER_NOEXCEPT
#    define CX_TERMINATE_ON_EXCEPTION(expr) do { expr; } while (false)
#  else
#    define CX_TERMINATE_ON_EXCEPTION(expr) do { try { expr; } catch (...) { qTerminate(); } } while (false)
#  endif
#endif

CX_CORE_EXPORT C_DECL_CONST_FUNCTION bool cxSharedBuild() noexcept;

#ifndef CX_OUTOFLINE_TEMPLATE
#  define CX_OUTOFLINE_TEMPLATE
#endif
#ifndef CX_INLINE_TEMPLATE
#  define CX_INLINE_TEMPLATE inline
#endif

#if !defined(CX_NO_DEBUG) && !defined(CX_DEBUG)
#  define CX_DEBUG
#endif

// CxPrivate::asString defined in qstring.h
#ifndef cxPrintable
#  define cxPrintable(string) CxPrivate::asString(string).toLocal8Bit().constData()
#endif

#ifndef cxUtf8Printable
#  define cxUtf8Printable(string) CxPrivate::asString(string).toUtf8().constData()
#endif

#ifndef cxUtf16Printable
#  define cxUtf16Printable(string) \
    static_cast<const wchar_t*>(static_cast<const void*>(CXString(string).utf16()))
#endif

class CXString;
CX_CORE_EXPORT CXString cx_error_string(int errorCode = -1);

#ifndef CX_CC_MSVC
C_NORETURN
#endif
CX_CORE_EXPORT void cx_assert(const char *assertion, const char *file, int line) noexcept;



#ifndef CX_CC_MSVC
C_NORETURN
#endif
CX_CORE_EXPORT void cx_assert_x(const char *where, const char *what, const char *file, int line) noexcept;

#if !defined(CX_ASSERT_X)
#  if defined(CX_NO_DEBUG) && !defined(CX_FORCE_ASSERTS)
#    define CX_ASSERT_X(cond, where, what) static_cast<void>(false && (cond))
#  else
#    define CX_ASSERT_X(cond, where, what) ((cond) ? static_cast<void>(0) : cx_assert_x(where, what, __FILE__, __LINE__))
#  endif
#endif

C_NORETURN CX_CORE_EXPORT void cx_check_pointer(const char *, int) noexcept;
CX_CORE_EXPORT void cxBadAlloc();

#ifdef CX_NO_EXCEPTIONS
#  if defined(CX_NO_DEBUG) && !defined(CX_FORCE_ASSERTS)
#    define CX_CHECK_PTR(p) cx_noop()
#  else
#    define CX_CHECK_PTR(p) do {if (!(p)) cx_check_pointer(__FILE__,__LINE__);} while (false)
#  endif
#else
#  define CX_CHECK_PTR(p) do { if (!(p)) cxBadAlloc(); } while (false)
#endif

template <typename T>
inline T *cx_check_ptr(T *p) { CX_CHECK_PTR(p); return p; }

typedef void (*CXFunctionPointer)();

#if !defined(CX_UNIMPLEMENTED)
#  define CX_UNIMPLEMENTED() C_WARNING("Unimplemented code.")
#endif

C_DECL_CONSTEXPR static inline C_DECL_UNUSED bool cxFuzzyCompare(double p1, double p2)
{
    return (C_ABS(p1 - p2) * 1000000000000. <= C_MIN(C_ABS(p1), C_ABS(p2)));
}

C_DECL_CONSTEXPR static inline C_DECL_UNUSED bool cxFuzzyCompare(float p1, float p2)
{
    return (C_ABS(p1 - p2) * 100000.f <= C_MIN(C_ABS(p1), C_ABS(p2)));
}

C_DECL_CONSTEXPR static inline C_DECL_UNUSED bool cxFuzzyIsNull(double d)
{
    return C_ABS(d) <= 0.000000000001;
}

C_DECL_CONSTEXPR static inline C_DECL_UNUSED bool cxFuzzyIsNull(float f)
{
    return C_ABS(f) <= 0.00001f;
}

C_WARNING_PUSH
C_WARNING_DISABLE_CLANG("-Wfloat-equal")
C_WARNING_DISABLE_GCC("-Wfloat-equal")
C_WARNING_DISABLE_INTEL(1572)

C_DECL_CONSTEXPR static inline C_DECL_UNUSED bool cxIsNull(double d) noexcept
{
    return d == 0.0;
}

C_REQUIRED_RESULT C_DECL_CONSTEXPR static inline C_DECL_UNUSED bool cxIsNull(float f) noexcept
{
    return f == 0.0f;
}

C_WARNING_POP

/*
   Compilers which follow outdated template instantiation rules
   require a class to have a comparison operator to exist when
   a QList of this type is instantiated. It's not actually
   used in the list, though. Hence the dummy implementation.
   Just in case other code relies on it we better trigger a warning
   mandating a real implementation.
*/

#ifdef CX_FULL_TEMPLATE_INSTANTIATION
#  define CX_DUMMY_COMPARISON_OPERATOR(C) \
    bool operator==(const C&) const { \
        C_WARNING(#C"::operator==(const "#C"&) was called"); \
        return false; \
    }
#else

#  define CX_DUMMY_COMPARISON_OPERATOR(C)
#endif

C_WARNING_PUSH
// warning: noexcept-expression evaluates to 'false' because of a call to 'void swap(..., ...)'
C_WARNING_DISABLE_GCC("-Wnoexcept")

namespace CxPrivate
{
namespace SwapExceptionTester { // insulate users from the "using std::swap" below
    using std::swap; // import std::swap
    template <typename T>
    void checkSwap(T &t)
        noexcept(noexcept(swap(t, t)));
    // declared, but not implemented (only to be used in unevaluated contexts (noexcept operator))
}
} // namespace QtPrivate

template <typename T>
inline void cxSwap(T &value1, T &value2)
    noexcept(noexcept(CxPrivate::SwapExceptionTester::checkSwap(value1)))
{
    using std::swap;
    swap(value1, value2);
}

C_WARNING_POP

#if CX_DEPRECATED_SINCE(5, 0)
CX_CORE_EXPORT CX_DEPRECATED void *cxMalloc(size_t size) CX_ALLOC_SIZE(1);
CX_CORE_EXPORT CX_DEPRECATED void cxFree(void *ptr);
CX_CORE_EXPORT CX_DEPRECATED void *cxRealloc(void *ptr, size_t size) CX_ALLOC_SIZE(2);
CX_CORE_EXPORT CX_DEPRECATED void *cxMemCopy(void *dest, const void *src, size_t n);
CX_CORE_EXPORT CX_DEPRECATED void *cxMemSet(void *dest, int c, size_t n);
#endif
CX_CORE_EXPORT void *cxMallocAligned(size_t size, size_t alignment) C_ALLOC_SIZE(1);
CX_CORE_EXPORT void *cxReallocAligned(void *ptr, size_t size, size_t oldsize, size_t alignment) C_ALLOC_SIZE(2);
CX_CORE_EXPORT void cxFreeAligned(void *ptr);


#if !defined(CX_CC_WARNINGS)
#  define CX_NO_WARNINGS
#endif
#if defined(CX_NO_WARNINGS)
#  if defined(CX_CC_MSVC)
CX_WARNING_DISABLE_MSVC(4251) /* class 'type' needs to have dll-interface to be used by clients of class 'type2' */
CX_WARNING_DISABLE_MSVC(4244) /* conversion from 'type1' to 'type2', possible loss of data */
CX_WARNING_DISABLE_MSVC(4275) /* non - DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier' */
CX_WARNING_DISABLE_MSVC(4514) /* unreferenced inline function has been removed */
CX_WARNING_DISABLE_MSVC(4800) /* 'type' : forcing value to bool 'true' or 'false' (performance warning) */
CX_WARNING_DISABLE_MSVC(4097) /* typedef-name 'identifier1' used as synonym for class-name 'identifier2' */
CX_WARNING_DISABLE_MSVC(4706) /* assignment within conditional expression */
CX_WARNING_DISABLE_MSVC(4355) /* 'this' : used in base member initializer list */
CX_WARNING_DISABLE_MSVC(4710) /* function not inlined */
CX_WARNING_DISABLE_MSVC(4530) /* C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc */
#  elif defined(CX_CC_BOR)
#    pragma option -w-inl
#    pragma option -w-aus
#    pragma warn -inl
#    pragma warn -pia
#    pragma warn -ccc
#    pragma warn -rch
#    pragma warn -sig
#  endif
#endif

// Work around MSVC warning about use of 3-arg algorithms
// until we can depend on the C++14 4-arg ones.
//
// These algortithms do NOT check for equal length.
// They need to be treated as if they called the 3-arg version (which they do)!
#ifdef CX_CC_MSVC
# define CX_3ARG_ALG(alg, f1, l1, f2, l2) \
    std::alg(f1, l1, f2, l2)
#else
# define CX_3ARG_ALG(alg, f1, l1, f2, l2)     \
    [&f1, &l1, &f2, &l2]() {                  \
        CX_UNUSED(l2);                         \
        return std::alg(f1, l1, f2);          \
    }()
#endif
template <typename ForwardIterator1, typename ForwardIterator2>
inline bool cx_is_permutation(ForwardIterator1 first1, ForwardIterator1 last1,
                              ForwardIterator2 first2, ForwardIterator2 last2)
{
    return CX_3ARG_ALG(is_permutation, first1, last1, first2, last2);
}
#undef CX_3ARG_ALG

// this adds const to non-const objects (like std::as_const)
template <typename T>
C_DECL_CONSTEXPR typename std::add_const<T>::type &cxAsConst(T &t) noexcept { return t; }
// prevent rvalue arguments:
template <typename T>
void cxAsConst(const T &&) = delete;

// like std::exchange
template <typename T, typename U = T>
C_DECL_RELAXED_CONSTEXPR T cxExchange(T &t, U &&newValue)
{
    T old = std::move(t);
    t = std::forward<U>(newValue);
    return old;
}

#ifndef CX_NO_FOREACH

namespace CxPrivate {

template <typename T>
class CXForeachContainer {
    CX_DISABLE_COPY(CXForeachContainer)
public:
    CXForeachContainer(const T &t) : c(t), i(qAsConst(c).begin()), e(qAsConst(c).end()) {}
    CXForeachContainer(T &&t) : c(std::move(t)), i(qAsConst(c).begin()), e(qAsConst(c).end())  {}

    CXForeachContainer(CXForeachContainer &&other)
        : c(std::move(other.c)),
          i(qAsConst(c).begin()),
          e(qAsConst(c).end()),
          control(std::move(other.control))
    {
    }

    CXForeachContainer &operator=(CXForeachContainer &&other)
    {
        c = std::move(other.c);
        i = qAsConst(c).begin();
        e = qAsConst(c).end();
        control = std::move(other.control);
        return *this;
    }

    T c;
    typename T::const_iterator i, e;
    int control = 1;
};

template<typename T>
CXForeachContainer<typename std::decay<T>::type> cxMakeForeachContainer(T &&t)
{
    return CXForeachContainer<typename std::decay<T>::type>(std::forward<T>(t));
}

}

#define CX_FOREACH_JOIN(A, B) CX_FOREACH_JOIN_IMPL(A, B)
#define CX_FOREACH_JOIN_IMPL(A, B) A ## B

#if __cplusplus >= 201703L
// Use C++17 if statement with initializer. User's code ends up in a else so
// scoping of different ifs is not broken
#define CX_FOREACH_IMPL(variable, name, container)                 \
    for (auto name = CxPrivate::cxMakeForeachContainer(container); \
        name.i != name.e; ++name.i)                               \
        if (variable = *name.i; false) {} else
#else
// Explanation of the control word:
//  - it's initialized to 1
//  - that means both the inner and outer loops start
//  - if there were no breaks, at the end of the inner loop, it's set to 0, which
//    causes it to exit (the inner loop is run exactly once)
//  - at the end of the outer loop, it's inverted, so it becomes 1 again, allowing
//    the outer loop to continue executing
//  - if there was a break inside the inner loop, it will exit with control still
//    set to 1; in that case, the outer loop will invert it to 0 and will exit too
#define CX_FOREACH_IMPL(variable, name, container)             \
for (auto name = CxPrivate::cxMakeForeachContainer(container); \
     name.control && name.i != name.e;                        \
     ++name.i, name.control ^= 1)                             \
    for (variable = *name.i; name.control; name.control = 0)
#endif

#define CX_FOREACH(variable, container) \
    CX_FOREACH_IMPL(variable, CX_FOREACH_JOIN(_container_, __LINE__), container)
#endif // CX_NO_FOREACH

#define CX_FOREVER for(;;)
#ifndef CX_NO_KEYWORDS
# ifndef CX_NO_FOREACH
#  ifndef foreach
#    define foreach CX_FOREACH
#  endif
# endif // CX_NO_FOREACH
#  ifndef forever
#    define forever CX_FOREVER
#  endif
#endif

template <typename T> inline T *cxGetPtrHelper(T *ptr) { return ptr; }
template <typename Ptr> inline auto cxGetPtrHelper(Ptr &ptr) -> decltype(ptr.operator->()) { return ptr.operator->(); }

// The body must be a statement:
#define CX_CAST_IGNORE_ALIGN(body) CX_WARNING_PUSH CX_WARNING_DISABLE_GCC("-Wcast-align") body CX_WARNING_POP
#define CX_DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() \
    { CX_CAST_IGNORE_ALIGN(return reinterpret_cast<Class##Private *>(cxGetPtrHelper(d_ptr));) } \
    inline const Class##Private* d_func() const \
    { CX_CAST_IGNORE_ALIGN(return reinterpret_cast<const Class##Private *>(cxGetPtrHelper(d_ptr));) } \
    friend class Class##Private;

#define CX_DECLARE_PRIVATE_D(Dptr, Class) \
    inline Class##Private* d_func() \
    { CX_CAST_IGNORE_ALIGN(return reinterpret_cast<Class##Private *>(cxGetPtrHelper(Dptr));) } \
    inline const Class##Private* d_func() const \
    { CX_CAST_IGNORE_ALIGN(return reinterpret_cast<const Class##Private *>(cxGetPtrHelper(Dptr));) } \
    friend class Class##Private;

#define CX_DECLARE_PUBLIC(Class)                                    \
    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
    friend class Class;

#define CX_D(Class) Class##Private * const d = d_func()
#define CX_Q(Class) Class * const q = q_func()

#define CX_TR_NOOP(x) x
#define CX_TR_NOOP_UTF8(x) x
#define CX_TRANSLATE_NOOP(scope, x) x
#define CX_TRANSLATE_NOOP_UTF8(scope, x) x
#define CX_TRANSLATE_NOOP3(scope, x, comment) {x, comment}
#define CX_TRANSLATE_NOOP3_UTF8(scope, x, comment) {x, comment}

#ifndef CX_NO_TRANSLATION // ### Qt6: This should enclose the NOOPs above

#define CX_TR_N_NOOP(x) x
#define CX_TRANSLATE_N_NOOP(scope, x) x
#define CX_TRANSLATE_N_NOOP3(scope, x, comment) {x, comment}

// Defined in qcoreapplication.cpp
// The better name qTrId() is reserved for an upcoming function which would
// return a much more powerful QStringFormatter instead of a QString.
CX_CORE_EXPORT CXString cxTrId(const char *id, int n = -1);

#define CX_TRID_NOOP(id) id

#endif // CX_NO_TRANSLATION


#ifdef CX_QDOC

// Just for documentation generation
template<typename T>
auto cxOverload(T functionPointer);
template<typename T>
auto cxConstOverload(T memberFunctionPointer);
template<typename T>
auto cxNonConstOverload(T memberFunctionPointer);

#elif defined(CX_COMPILER_VARIADIC_TEMPLATES)

template <typename... Args>
struct CXNonConstOverload
{
    template <typename R, typename T>
    CX_DECL_CONSTEXPR auto operator()(R (T::*ptr)(Args...)) const noexcept -> decltype(ptr)
    { return ptr; }

    template <typename R, typename T>
    static CX_DECL_CONSTEXPR auto of(R (T::*ptr)(Args...)) noexcept -> decltype(ptr)
    { return ptr; }
};

template <typename... Args>
struct CXConstOverload
{
    template <typename R, typename T>
    CX_DECL_CONSTEXPR auto operator()(R (T::*ptr)(Args...) const) const noexcept -> decltype(ptr)
    { return ptr; }

    template <typename R, typename T>
    static CX_DECL_CONSTEXPR auto of(R (T::*ptr)(Args...) const) noexcept -> decltype(ptr)
    { return ptr; }
};

template <typename... Args>
struct CXOverload : CXConstOverload<Args...>, CXNonConstOverload<Args...>
{
    using CXConstOverload<Args...>::of;
    using CXConstOverload<Args...>::operator();
    using CXNonConstOverload<Args...>::of;
    using CXNonConstOverload<Args...>::operator();

    template <typename R>
    CX_DECL_CONSTEXPR auto operator()(R (*ptr)(Args...)) const noexcept -> decltype(ptr)
    { return ptr; }

    template <typename R>
    static CX_DECL_CONSTEXPR auto of(R (*ptr)(Args...)) noexcept -> decltype(ptr)
    { return ptr; }
};

#if defined(__cpp_variable_templates) && __cpp_variable_templates >= 201304 // C++14
template <typename... Args> CX_CONSTEXPR CX_DECL_UNUSED CXOverload<Args...> cxOverload = {};
template <typename... Args> CX_CONSTEXPR CX_DECL_UNUSED CXConstOverload<Args...> cxConstOverload = {};
template <typename... Args> CX_CONSTEXPR CX_DECL_UNUSED CXNonConstOverload<Args...> cxNonConstOverload = {};
#endif

#endif


class CXByteArray;
CX_CORE_EXPORT CXByteArray cxGetEnv(const char *varName);
CX_CORE_EXPORT CXString cxEnvironmentVariable(const char *varName);
CX_CORE_EXPORT CXString cxEnvironmentVariable(const char *varName, const CXString &defaultValue);
CX_CORE_EXPORT bool cxPutEnv(const char *varName, const CXByteArray& value);
CX_CORE_EXPORT bool cxUnsetEnv(const char *varName);

CX_CORE_EXPORT bool cxEnvironmentVariableIsEmpty(const char *varName) noexcept;
CX_CORE_EXPORT bool cxEnvironmentVariableIsSet(const char *varName) noexcept;
CX_CORE_EXPORT int  cxEnvironmentVariableIntValue(const char *varName, bool *ok=nullptr) noexcept;

inline int cxIntCast(double f) { return int(f); }
inline int cxIntCast(float f) { return int(f); }

#define CX_MODULE(x)

#if !defined(CX_BOOTSTRAPPED) && defined(CX_REDUCE_RELOCATIONS) && defined(__ELF__) && \
    (!defined(__PIC__) || (defined(__PIE__) && defined(CX_CC_GNU) && CX_CC_GNU >= 500))
#  error "You must build your code with position independent code if clibrary-cx was built with -reduce-relocations. "\
         "Compile your code with -fPIC (and not with -fPIE)."
#endif

namespace CxPrivate
{
//like std::enable_if
template <bool B, typename T = void> struct CXEnableIf;
template <typename T> struct CXEnableIf<true, T> { typedef T Type; };
}

CX_END_NAMESPACE

#endif
#endif

#endif // clibrary_CX_GLOBAL_H
