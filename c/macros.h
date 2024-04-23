
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*************************************************************************
> FileName: macros.h
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Wed 07 Sep 2022 21:09:31 PM CST
 ************************************************************************/
#ifndef CLIBRARY_MACROS_H
#define CLIBRARY_MACROS_H
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include <sys/poll.h>

#if !defined (__CLIB_H_INSIDE__) && !defined (CLIB_COMPILATION)
#error "Only <clib.h> can be included directly."
#endif

#define ALIGNOF_CUINT32             4
#define ALIGNOF_CUINT64             8
#define ALIGNOF_UNSIGNED_LONG       8

#define SIZEOF_CHAR                 1
#define SIZEOF_INT                  4
#define SIZEOF_LONG                 8
#define SIZEOF_LONG_LONG            8
#define SIZEOF_SHORT                2
#define SIZEOF_SIZE_T               8
#define SIZEOF_SSIZE_T              8
#define SIZEOF_VOID_P               8
#define SIZEOF_WCHAR_T              4

#define CLIB_SIZEOF_SIZE_T          8


/**************************** 调试相关 ***********************************/
#ifdef DEBUG
#define C_DEBUG_INFO(str)               C_LOG_DEBUG(#str)
#else
#define C_DEBUG_INFO(str)
#endif

/**************************** 基础类型 ***********************************/
/**
 * @brief
 *  提供 NULL 定义
 */
#ifndef NULL
#ifdef __cplusplus
#define NULL                                                    (nullptr)
#else
#define NULL                                                    ((void*)0)
#endif
#endif

/**
 * @brief
 *  定义 false
 */

#ifdef __cplusplus
#define false                                                   false
#else
#undef false
#define false                                                   (0)
#endif

/**
 * @brief bool
 */
#ifdef __cplusplus
#else
#ifndef bool
typedef int                                                     bool;
#endif
#endif

/**
 * @brief
 *  定义 true
 */
#ifdef __cplusplus
#define true                                                    true
#else
#undef true
#define true                                                    (!false)
#endif

/**
 * @brief
 *  一些常用基础类型的简写
 */
typedef signed char                                             cint8;
typedef unsigned char                                           cuint8;

typedef signed short                                            cint16;
typedef unsigned short                                          cuint16;

typedef signed int                                              cint32;
typedef unsigned int                                            cuint32;

typedef signed long                                             cint64;
typedef unsigned long                                           cuint64;

typedef char                                                    cchar;
typedef short                                                   cshort;
typedef long                                                    clong;
typedef int                                                     cint;
typedef cint                                                    cboolean;

typedef unsigned char                                           cuchar;
typedef unsigned short                                          cushort;
typedef unsigned long                                           culong;
typedef unsigned int                                            cuint;

typedef float                                                   cfloat;
typedef double                                                  cdouble;

typedef unsigned long                                           csize;

typedef void*                                                   cvoidptr;
typedef cuint*                                                  cuintptr;
typedef cint                                                    crefcount;
typedef cint                                                    catomicrefcount;

#define C_CINT64_CONSTANT(val)	                                (val##L)
#define C_CUINT64_CONSTANT(val)	                                (val##UL)


#define C_CINT16_MODIFIER                                       "h"
#define C_CINT16_FORMAT                                         "hi"
#define C_CUINT16_FORMAT                                        "hu"

#define C_CINT32_MODIFIER                                       ""
#define C_CINT32_FORMAT                                         "i"
#define C_CUINT32_FORMAT                                        "u"

#define C_CINT64_MODIFIER                                       "l"
#define C_CINT64_FORMAT                                         "li"
#define C_CUINT64_FORMAT                                        "lu"

#define C_MAX_INT8                                              ((cint8) 0x7F)
#define C_MAX_UINT8                                             ((cuint8) 0xFF)
#define C_MIN_INT8                                              ((cint8) (-C_MAX_INT8) - 1)

#define C_MAX_INT16                                             ((cint16) 0x7FFF)
#define C_MAX_UINT16                                            ((cuint16) 0xFFFF)
#define C_MIN_INT16                                             ((cint16) (-C_MAX_INT16) - 1)

#define C_MAX_INT32                                             ((cint32) 0x7FFFFFFF)
#define C_MAX_UINT32                                            ((cuint32) 0xFFFFFFFF)
#define C_MIN_INT32                                             ((cint32) (-C_MAX_INT32) - 1)

#define C_MAX_INT64                                             ((cint64) C_CINT64_CONSTANT(0x7FFFFFFFFFFFFFFF))
#define C_MAX_UINT64                                            ((cuint64) C_CUINT64_CONSTANT(0xFFFFFFFFFFFFFFFF))
#define C_MIN_INT64                                             ((cint64) (-C_MAX_INT64) - C_CINT64_CONSTANT(1))

#define C_MAX_SIZE                                              C_MAX_UINT64
#define C_MAX_UINT                                              C_MAX_UINT32


/**
 * @brief 定义常量
 */
#define C_E                                                     2.7182818284590452353602874713526624977572470937000
#define C_LN2                                                   0.69314718055994530941723212145817656807550013436026
#define C_LN10                                                  2.3025850929940456840179914546843642076011014886288
#define C_PI                                                    3.1415926535897932384626433832795028841971693993751
#define C_PI_2                                                  1.5707963267948966192313216916397514420985846996876
#define C_PI_4                                                  0.78539816339744830961566084581987572104929234984378
#define C_SQRT2                                                 1.4142135623730950488016887242096980785696718753769


/****************************  类型  *************************************/
typedef cint            (*CCompareFunc)         (void* data1, void* data2);
typedef cint            (*CCompareDataFunc)     (void* data1, void* data2, void* udata);
typedef bool            (*CEqualFunc)           (const void* data1, const void* data2);
typedef bool            (*CEqualFuncFull)       (const void* data1, const void* data2, void* udata);

typedef void            (*CDestroyNotify)       (void* data);
typedef void            (*CFunc)                (void* data, void* udata);

typedef cuint           (*CHashFunc)            (const void* key);
typedef void            (*CHFunc)               (void* key, void* value, void* udata);
typedef void*           (*CCopyFunc)            (const void* src, void* udata);
typedef void            (*CFreeFunc)            (void* data);
typedef const char*     (*CTranslateFunc)       (const char* str, void* udata);


typedef enum
{
    C_IO_IN     = POLLIN,
    C_IO_OUT    = POLLOUT,
    C_IO_PRI    = POLLPRI,
    C_IO_ERR    = POLLERR,
    C_IO_HUP    = POLLHUP,
    C_IO_NVAL   = POLLNVAL,
} CIOCondition;


/** @NOTE **/
/********************************* 宏定义 ***************************************/
#ifdef __GNUC__
#define C_GNUC_CHECK_VERSION(major, minor)                      ((__GNUC__ > (major)) || ((__GNUC__ == (major)) && (__GNUC_MINOR__ >= (minor))))
#else
#define C_GNUC_CHECK_VERSION(major, minor)                      0
#endif


#define	C_STRINGIFY_ARG(contents)                               #contents
#define C_STRINGIFY(macro_or_string)                            C_STRINGIFY_ARG (macro_or_string)


#if C_GNUC_CHECK_VERSION(2, 8)
#define C_GNUC_EXTENSION __extension__
#else
#define C_GNUC_EXTENSION
#endif


/**
 * @brief 函数可见性，gcc可见性分为以下几种情况(__attribute__((visibility(""))))：
 *  1. default：默认可见（函数在程序的任何地方可见）
 *  2. hidden：隐藏可见性。函数在链接时候不可见，对于外部链接的符号，将无法从其它目标文件中引用
 *  3. protected：受保护可见。函数在链接时候可见，但只能被其所在的目标文件或具有相同共享库的目标文件引用
 */
#if (defined(_WIN32) || defined(__CYGWIN__))
#define C_SYMBOL_EXPORT __declspec(dllexport)
#define C_SYMBOL_IMPORT __declspec(dllimport)
#define C_SYMBOL_HIDDEN
#define C_SYMBOL_PROTECTED
#elif __GNUC__ >= 4
#define C_SYMBOL_IMPORT
#define C_SYMBOL_HIDDEN     __attribute__((visibility("hidden")))
#define C_SYMBOL_EXPORT     __attribute__((visibility("default")))
#define C_SYMBOL_PROTECTED  __attribute__((visibility("protected")))
#else
#define C_SYMBOL_EXPORT
#define C_SYMBOL_IMPORT
#define C_SYMBOL_HIDDEN
#define C_SYMBOL_PROTECTED
#endif


#if !defined (__cplusplus)
#undef C_CXX_STD_VERSION
#define C_CXX_STD_CHECK_VERSION(version) (0)
#if defined (__STDC_VERSION__)
#define C_C_STD_VERSION __STDC_VERSION__
#else
#define C_C_STD_VERSION 199000L
#endif /* defined (__STDC_VERSION__) */
#define C_C_STD_CHECK_VERSION(version) \
    ( \
        ((version) >= 199000L && (version) <= C_C_STD_VERSION) \
        || ((version) == 89 && C_C_STD_VERSION >= 199000L) \
        || ((version) == 90 && C_C_STD_VERSION >= 199000L) \
        || ((version) == 99 && C_C_STD_VERSION >= 199901L) \
        || ((version) == 11 && C_C_STD_VERSION >= 201112L) \
        || ((version) == 17 && C_C_STD_VERSION >= 201710L) \
        || 0 \
    )
#else /* defined (__cplusplus) */
#undef C_C_STD_VERSION
#define C_C_STD_CHECK_VERSION(version) (0)
#if defined (_MSVC_LANG)
#define C_CXX_STD_VERSION (_MSVC_LANG > __cplusplus ? _MSVC_LANG : __cplusplus)
#else
#define C_CXX_STD_VERSION __cplusplus
#endif /* defined(_MSVC_LANG) */
#define C_CXX_STD_CHECK_VERSION(version) \
    ( \
        ((version) >= 199711L && (version) <= C_CXX_STD_VERSION) \
        || ((version) == 98 && C_CXX_STD_VERSION >= 199711L)     \
        || ((version) == 03 && C_CXX_STD_VERSION >= 199711L)     \
        || ((version) == 11 && C_CXX_STD_VERSION >= 201103L)     \
        || ((version) == 14 && C_CXX_STD_VERSION >= 201402L)     \
        || ((version) == 17 && C_CXX_STD_VERSION >= 201703L)     \
        || ((version) == 20 && C_CXX_STD_VERSION >= 202002L)     \
        || 0 \
    )
#endif /* !defined (__cplusplus) */


#ifdef __has_feature
#define c_macro__has_feature                                    __has_feature
#else
#define c_macro__has_feature(x)                                 0
#endif

#ifdef __has_builtin
#define c_macro__has_builtin                                    __has_builtin
#else
#define c_macro__has_builtin(x)                                 0
#endif

#ifdef __has_extension
#define c_macro__has_extension                                  __has_extension
#else
#define c_macro__has_extension(x)                               0
#endif

#ifdef __has_attribute
#define c_macro__has_attribute                                  __has_attribute
#else
// 针对gcc < 5 或其它不支持 __has_attribute 属性的编译器
#define c_macro__has_attribute(x)                               c_macro__has_attribute_##x
#define c_macro__has_attribute___pure__                         C_GNUC_CHECK_VERSION (2, 96)
#define c_macro__has_attribute___malloc__                       C_GNUC_CHECK_VERSION (2, 96)
#define c_macro__has_attribute___noinline__                     C_GNUC_CHECK_VERSION (2, 96)
#define c_macro__has_attribute___sentinel__                     C_GNUC_CHECK_VERSION (4, 0)
#define c_macro__has_attribute___alloc_size__                   C_GNUC_CHECK_VERSION (4, 3)
#define c_macro__has_attribute___format__                       C_GNUC_CHECK_VERSION (2, 4)
#define c_macro__has_attribute___format_arg__                   C_GNUC_CHECK_VERSION (2, 4)
#define c_macro__has_attribute___noreturn__                     (C_GNUC_CHECK_VERSION (2, 8) || (0x5110 <= __SUNPRO_C))
#define c_macro__has_attribute___const__                        C_GNUC_CHECK_VERSION (2, 4)
#define c_macro__has_attribute___unused__                       C_GNUC_CHECK_VERSION (2, 4)
#define c_macro__has_attribute___no_instrument_function__       C_GNUC_CHECK_VERSION (2, 4)
#define c_macro__has_attribute_fallthrough                      C_GNUC_CHECK_VERSION (6, 0)
#define c_macro__has_attribute___deprecated__                   C_GNUC_CHECK_VERSION (3, 1)
#define c_macro__has_attribute_may_alias                        C_GNUC_CHECK_VERSION (3, 3)
#define c_macro__has_attribute_warn_unused_result               C_GNUC_CHECK_VERSION (3, 4)
#define c_macro__has_attribute_cleanup                          C_GNUC_CHECK_VERSION (3, 3)
#endif

/**
 * @brief
 *  pure 函数返回值仅仅依赖输入参数或全局变量，仅支持gcc编译器
 *  
 *  bool c_type_check_value (const CValue *value) C_GNUC_PURE;
 * 
 * @see
 *  the [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-pure-function-attribute) for more details.
 */
#if c_macro__has_attribute(__pure__)
#define C_PURE                                                  __attribute__((__pure__))
#else
#define C_PURE
#endif

/**
 * @brief
 *  void* c_malloc (unsigned long n_bytes) C_GNUC_MALLOC C_GNUC_ALLOC_SIZE(1);
 * @see
 *  [GNU C `malloc` function attribute](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-functions-that-behave-like-malloc)
 */
#if c_macro__has_attribute(__malloc__)
#define C_MALLOC                                                __attribute__ ((__malloc__))
#else
#define C_MALLOC
#endif

/**
 * @brief
 *  char* c_strconcat (const char *string1, ...) C_GNUC_NULL_TERMINATED;
 *
 * @see
 *  the [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-sentinel-function-attribute) for more details.
 */
#if c_macro__has_attribute(__sentinel__)
#define C_NULL_TERMINATED                                       __attribute__((__sentinel__))
#else
#define C_NULL_TERMINATED
#endif

/**
 * @brief
 *  void* c_malloc_n (int n_blocks, int n_block_bytes) C_GNUC_MALLOC C_GNUC_ALLOC_SIZE2(1, 2);
 * 
 * @see
 *  the [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-alloc_005fsize-function-attribute) for more details.
 */
#if c_macro__has_attribute(__alloc_size__)
#define C_ALLOC_SIZE(x)                                         __attribute__((__alloc_size__(x)))
#define C_ALLOC_SIZE2(x,y)                                      __attribute__((__alloc_size__(x,y)))
#else
#define C_ALLOC_SIZE(x)
#define C_ALLOC_SIZE2(x,y)
#endif

/**
 * @brief
 *  @param format_idx: 表示第几个参数为 "格式化" 参数(index从1开始)
 *  @param arg_idx: 表示第几个参数为 "第一个变参" 参数(index从1开始)，没有则为0
 *  int g_snprintf (char* string, unsigned long n, char const *format, ...) C_GNUC_PRINTF (3, 4);
 * 
 * @see
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-Wformat-3288)
 */
#if c_macro__has_attribute(__format__)
#if !defined (__clang__) && C_GNUC_CHECK_VERSION (4, 4)
#define C_PRINTF( format_idx, arg_idx )                         __attribute__((__format__(printf, format_idx, arg_idx)))
#define C_SCANF( format_idx, arg_idx )                          __attribute__((__format__(scanf, format_idx, arg_idx)))
#define C_STRFTIME( format_idx )                                __attribute__((__format__(strftime, format_idx, 0)))
#else
#define C_PRINTF( format_idx, arg_idx )                         __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#define C_SCANF( format_idx, arg_idx )                          __attribute__((__format__ (__scanf__, format_idx, arg_idx)))
#define C_STRFTIME( format_idx )                                __attribute__((__format__ (__strftime__, format_idx, 0)))
#endif
#else
//
#define C_PRINTF( format_idx, arg_idx )
#define C_SCANF( format_idx, arg_idx )
#define C_STRFTIME( format_idx )
#endif


/**
 * @brief
 *  char* c_dgettext (char *domain_name, char *msgid) G_GNUC_FORMAT (2);
 *
 * @see
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-Wformat-nonliteral-1) for more details.
 */
#if c_macro__has_attribute(__format_arg__)
#define C_FORMAT(arg_idx)                                       __attribute__ ((__format_arg__ (arg_idx)))
#else
#define C_FORMAT( arg_idx )
#endif


/**
 * @brief
 *  告知编译器，函数无返回参数
 *  void c_abort (void) C_GNUC_NORETURN;
 *
 * @see
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-noreturn-function-attribute) for more details.
 *  
 */
#if c_macro__has_attribute(__noreturn__)
#define C_NORETURN                                              __attribute__ ((__noreturn__))
#else
#define C_NORETURN
#endif

/**
 * @brief
 *  char c_ascii_tolower (char c) C_GNUC_CONST;
 *
 * @see
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-const-function-attribute) for more details.
 */
#if c_macro__has_attribute(__const__)
#define C_CONST                                                 __attribute__((__const__))
#else
#define C_CONST
#endif

/**
 * @brief
 *  void my_unused_function (C_GNUC_UNUSED gint unused_argument, gint other_argument) C_GNUC_UNUSED;
 *
 * @see
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-unused-function-attribute) for more details.
 */
#if c_macro__has_attribute(__unused__)
#define C_UNUSED                                                __attribute__ ((__unused__))
#else
#define C_UNUSED
#endif

/**
 * @brief
 *  添加此属性，函数无法被分析
 *  int do_uninteresting_things (void) C_GNUC_NO_INSTRUMENT;
 *
 * @see 
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-no_005finstrument_005ffunction-function-attribute) for more details.
 *
 */
#if c_macro__has_attribute(__no_instrument_function__)
#define C_NO_INSTRUMENT                                         __attribute__ ((__no_instrument_function__))
#else
#define C_NO_INSTRUMENT
#endif

/**
 * @brief
 *  添加此属性，允许 switch - case不中断此 case
 *  
 *  gcc 打开 `-Wimplicit-fallthrough` 属性使用此功能
 *
 *  switch (foo)
 *  {
 *     case 1:
 *       printf ("it's 1\n");
 *       C_GNUC_FALLTHROUGH;
 *     case 2:
 *       printf("it's either 1 or 2\n");
 *       break;
 *  }
 *
 * @see
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Statement-Attributes.html#index-fallthrough-statement-attribute) for more details.
 *
 */
#if c_macro__has_attribute(fallthrough)
#define C_FALLTHROUGH                                           __attribute__((fallthrough))
#else
#define C_FALLTHROUGH 
#endif

/**
 * @brief
 *  指定函数已经过时
 *
 *  gcc 打开 `-Wdeprecated-declarations` 属性使用此功能
 *
 *  int my_mistake (void) C_GNUC_DEPRECATED;
 *
 * @See
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-deprecated-function-attribute) for more details.
 */
#if c_macro__has_attribute(__deprecated__)
#define C_DEPRECATED                                            __attribute__((__deprecated__))
#else
#define C_DEPRECATED
#endif 

/**
 * @brief
 *  当返回值未使用时候，编译器发出警告
 *
 *  CList *c_list_append (CList *list, void* data) C_GNUC_WARN_UNUSED_RESULT;
 *
 * @see 
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-warn_005funused_005fresult-function-attribute) for more details.
 *
 */
#if c_macro__has_attribute(warn_unused_result)
#define C_WARN_UNUSED_RESULT                                    __attribute__((warn_unused_result))
#else
#define C_WARN_UNUSED_RESULT
#endif

/**
 * @brief
 *  当前函数名字符串
 */
#if defined (__GNUC__) && defined (__cplusplus)
#define C_STRFUNC                                               ((const char*) (__PRETTY_FUNCTION__))
#elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define C_STRFUNC                                               ((const char*) (__func__))
#elif defined (__GNUC__) || (defined(_MSC_VER) && (_MSC_VER > 1300))
#define C_STRFUNC                                               ((const char*) (__FUNCTION__))
#else
#define C_STRFUNC                                               ((const char*) ("???"))
#endif

/**
 * @brief
 *  当前行号
 */
#define C_LINE                                                  C_STRINGIFY (__LINE__)

/**
 * @brief
 *  当前文件中位置
 */
#if defined(__GNUC__) && (__GNUC__ < 3) && !defined(__cplusplus)
#define C_STRLOC                                                __FILE__ ":" C_STRINGIFY (__LINE__) ":" __PRETTY_FUNCTION__ "()"
#else
#define C_STRLOC                                                __FILE__ ":" C_STRINGIFY (__LINE__)
#endif

#ifdef  __cplusplus
#define C_BEGIN_EXTERN_C                                        extern "C" {
#define C_END_EXTERN_C                                          }
#else
#define C_BEGIN_EXTERN_C
#define C_END_EXTERN_C
#endif

#ifndef __GI_SCANNER__ /* The static assert macro really confuses the introspection parser */
#define C_PASTE_ARGS(identifier1,identifier2)                   identifier1##identifier2
#define C_PASTE(identifier1,identifier2)                        C_PASTE_ARGS(identifier1, identifier2)
#if !defined(__cplusplus) \
    && defined(__STDC_VERSION__) \
    && (__STDC_VERSION__ >= 201112L || c_macro__has_feature(c_static_assert) || c_macro__has_extension(c_static_assert))
#define C_STATIC_ASSERT(expr)                                   _Static_assert (expr, "Expression evaluates to false")
#elif (defined(__cplusplus) && __cplusplus >= 201103L) \
    || (defined(__cplusplus) && defined (_MSC_VER) && (_MSC_VER >= 1600)) \
    || (defined (_MSC_VER) && (_MSC_VER >= 1800))
#define C_STATIC_ASSERT(expr)                                   static_assert (expr, "Expression evaluates to false")
#else
#ifdef __COUNTER__
#define C_STATIC_ASSERT(expr)                                   typedef char C_PASTE (_GStaticAssertCompileTimeAssertion_, __COUNTER__)[(expr) ? 1 : -1] C_UNUSED
#else
#define C_STATIC_ASSERT(expr)                                   typedef char C_PASTE (_GStaticAssertCompileTimeAssertion_, __LINE__)[(expr) ? 1 : -1] C_UNUSED
#endif
#endif /* __STDC_VERSION__ */
#define C_STATIC_ASSERT_EXPR(expr)                              ((void) sizeof (char[(expr) ? 1 : -1]))
#endif /* !__GI_SCANNER__ */


/**
 * @brief 忽略
 */
#ifdef __ICC
#define C_GNUC_BEGIN_IGNORE_DEPRECATIONS \
    _Pragma ("warning (push)") \
    _Pragma ("warning (disable:1478)")
#define C_GNUC_END_IGNORE_DEPRECATIONS \
    _Pragma ("warning (pop)")
#elif C_GNUC_CHECK_VERSION(4, 6)
#define C_GNUC_BEGIN_IGNORE_DEPRECATIONS \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define C_GNUC_END_IGNORE_DEPRECATIONS \
    _Pragma ("GCC diagnostic pop")
#elif defined (_MSC_VER) && (_MSC_VER >= 1500) && !defined (__clang__)
#define C_GNUC_BEGIN_IGNORE_DEPRECATIONS \
    __pragma (warning (push)) \
    __pragma (warning (disable : 4996))
#define C_GNUC_END_IGNORE_DEPRECATIONS \
    __pragma (warning (pop))
#elif defined (__clang__)
#define C_GNUC_BEGIN_IGNORE_DEPRECATIONS \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define C_GNUC_END_IGNORE_DEPRECATIONS \
    _Pragma("clang diagnostic pop")
#else
#define C_GNUC_BEGIN_IGNORE_DEPRECATIONS
#define C_GNUC_END_IGNORE_DEPRECATIONS
#define C_CANNOT_IGNORE_DEPRECATIONS
#endif


#if C_GNUC_CHECK_VERSION(2, 0) && defined(__OPTIMIZE__)
#define C_LIKELY(expr) (__builtin_expect(!!(expr), 1))
#define C_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
#else
#define C_LIKELY(expr) (expr)
#define C_UNLIKELY(expr) (expr)
#endif

#if !(defined (C_STMT_START) && defined (C_STMT_END))
#define C_STMT_START  do
#if defined (_MSC_VER) && (_MSC_VER >= 1500)
#define C_STMT_END \
    __pragma(warning(push)) \
    __pragma(warning(disable:4127)) \
    while(0) \
    __pragma(warning(pop))
#else
#define C_STMT_END    while (0)
#endif
#endif



/**
 * @brief
 *  定义: MAX(a, b)
 */
#undef C_MAX
#define C_MAX(a, b)                                             (((a) > (b)) ? (a) : (b))

#undef C_MIN
#define C_MIN(a, b)                                             (((a) < (b)) ? (a) : (b))

#undef C_ABS
#define C_ABS(a)                                                (((a) < 0) ? -(a) : (a))

/**
 * @brief
 *  获取数组容量
 */
#undef C_N_ELEMENTS
#define C_N_ELEMENTS(arr)                                       (sizeof(arr) / sizeof((arr)[0]))

/**
 * @brief
 *  指针转整型 && 整型转指针
 */
#undef C_POINTER_TO_SIZE           
#define C_POINTER_TO_SIZE(p)                                    ((unsigned long)(p))

#undef C_SIZE_TO_POINTER
#define C_SIZE_TO_POINTER(s)                                    ((void*) (unsigned long) (s))

#undef C_POINTER_TO_UINT
#define C_POINTER_TO_UINT(p)                                    ((unsigned int) (unsigned long) (p))

#undef C_UINT_TO_POINTER
#define C_UINT_TO_POINTER(u)	                                ((void*) (unsigned long) (u))

#define c_assert(x) \
C_STMT_START \
{ \
    assert(x); \
} \
C_STMT_END

#define c_assert_not_reached() \
C_STMT_START \
{ \
    C_LOG_WARNING_CONSOLE("It's impossible"); \
} \
C_STMT_END

// FIXME:// 替换为 gunlib ?
#define _c_printf    printf
#define _c_fprintf   fprintf
#define _c_sprintf   sprintf
#define _c_snprintf  snprintf
#define _c_vprintf   vprintf
#define _c_vfprintf  vfprintf
#define _c_vsprintf  vsprintf
#define _c_vsnprintf vsnprintf
// FIXME:// -- END

// ASCII 判断
#define ISUPPER(c)              ((c) >= 'A' && (c) <= 'Z')
#define ISLOWER(c)              ((c) >= 'a' && (c) <= 'z')
#define ISALPHA(c)              (ISUPPER (c) || ISLOWER (c))
#define TOUPPER(c)              (ISLOWER (c) ? (c) - 'a' + 'A' : (c))
#define TOLOWER(c)              (ISUPPER (c) ? (c) - 'A' + 'a' : (c))
#define ISSPACE(c)              ((c) == ' ' || (c) == '\f' || (c) == '\n' || (c) == '\r' || (c) == '\t' || (c) == '\v')

/****************** 内存申请与释放 ********************/
#define c_malloc(ptr, size) \
C_STMT_START \
{ \
    if (C_LIKELY(size > 0)) { \
        ptr = malloc (size); \
        c_assert(ptr); \
        memset (ptr, 0, size); \
    } \
    else { \
        c_assert(false); \
    } \
} \
C_STMT_END

#define c_malloc_type(ptr, type, count) \
C_STMT_START \
{ \
    if (C_LIKELY(count > 0)) { \
        ptr = (type*) malloc (sizeof (type) * count); \
        c_assert(ptr); \
        memset (ptr, 0, sizeof (type) * count); \
    } \
    else { \
        exit(-errno); \
    } \
} \
C_STMT_END

/**
 * @brief 计算指针数组元素个数
 */
#define c_ptr_array_count0(ptr, count) \
C_STMT_START \
{ \
    if (C_LIKELY(ptr)) { \
        int i = 0; \
        for (i = 0; ptr[i]; ++i);     \
        count = i; \
    } \
    else { \
        count = 0; \
    } \
} \
C_STMT_END

/**
 * @brief 为指针数组空间加1，并把指针元素ele放入数组中
 */
#define c_ptr_array_add1_0(ptr, type, ele) \
C_STMT_START \
{ \
    cuint c = 0; \
    if (C_LIKELY(ptr)) { \
        c_ptr_array_count0(ptr, c); \
        type* ptrT = NULL; \
        c_malloc(ptrT, sizeof(type) * (c + 2)); \
        memcpy(ptrT, ptr, sizeof(type) * c + 1); \
        ptrT[c] = ele; \
        c_free(ptr); \
        ptr = ptrT; \
    } \
    else { \
        c = 2; \
        c_malloc(ptr, sizeof(type) * c); \
        ptr[0] = ele; \
    } \
} \
C_STMT_END

/**
 * @brief 释放指针数组所有元素，对每个元素执行 freeFunc()
 */
#define c_ptr_array_free_full0(ptr, freeFunc) \
C_STMT_START \
{ \
    if (C_LIKELY(ptr)) { \
        int i = 0; \
        for (i = 0; ptr[i]; ++i) { \
            freeFunc(ptr[i]); \
        } \
        c_free (ptr); \
    } \
} \
C_STMT_END

/**
 * @brief 释放指针数组所有元素，对每个元素执行 c_free()
 */
#define c_ptr_array_free0(ptr) \
C_STMT_START \
{ \
    if (C_LIKELY(ptr)) { \
        int i = 0; \
        for (i = 0; ptr[i]; ++i) { \
            c_free(ptr[i]); \
        } \
        c_free(ptr); \
    } \
} \
C_STMT_END


/**
 * @brief 释放分配的资源
 */
#define c_free(ptr) \
C_STMT_START \
{ \
    if (C_LIKELY(ptr)) { \
        free (ptr); \
        ptr = NULL; \
    } \
} \
C_STMT_END

/* void(*func) (void*)*/
#define c_free_with_func(ptr, func) \
C_STMT_START \
{ \
    if (C_LIKELY(ptr)) { \
        func(ptr); \
        ptr = NULL; \
    } \
} \
C_STMT_END

#define c_return_if_fail(x) \
C_STMT_START \
{ \
    if (C_UNLIKELY(!(x))) { \
        return; \
    } \
} \
C_STMT_END

#define c_return_val_if_fail(x, val) \
C_STMT_START \
{ \
    if (C_UNLIKELY(!(x))) { \
        return val; \
    } \
} \
C_STMT_END


static inline bool _CLIB_CHECKED_ADD_UINT (cuint* dest, cuint a, cuint b)           { *dest = a + b; return *dest >= a; }
static inline bool _CLIB_CHECKED_MUL_UINT (cuint* dest, cuint a, cuint b)           { *dest = a * b; return !a || *dest / a == b; }
static inline bool _CLIB_CHECKED_ADD_UINT64 (cuint64* dest, cuint64 a, cuint64 b)   { *dest = a + b; return *dest >= a; }
static inline bool _CLIB_CHECKED_MUL_UINT64 (cuint64* dest, cuint64 a, cuint64 b)   { *dest = a * b; return !a || *dest / a == b; }
static inline bool _CLIB_CHECKED_ADD_SIZE (csize* dest, csize a, csize b)           { *dest = a + b; return *dest >= a; }
static inline bool _CLIB_CHECKED_MUL_SIZE (csize* dest, csize a, csize b)           { *dest = a * b; return !a || *dest / a == b; }

#define c_uint_checked_add(dest, a, b)      _CLIB_CHECKED_ADD_UINT(dest, a, b)
#define c_uint_checked_mul(dest, a, b)      _CLIB_CHECKED_MUL_UINT(dest, a, b)

#define c_uint64_checked_add(dest, a, b)    _CLIB_CHECKED_ADD_UINT64(dest, a, b)
#define c_uint64_checked_mul(dest, a, b)    _CLIB_CHECKED_MUL_UINT64(dest, a, b)

#define c_size_checked_add(dest, a, b)      _CLIB_CHECKED_ADD_SIZE(dest, a, b)
#define c_size_checked_mul(dest, a, b)      _CLIB_CHECKED_MUL_SIZE(dest, a, b)


//
static inline void* c_steal_pointer (void* pp)
{
    void** ptr = (void**) pp;
    void* ref = *ptr;
    *ptr = NULL;

    return ref;
}

static void c_free0 (void* p)
{
    c_free(p);
}

static inline void* c_malloc0 (cuint64 size)
{
    void* ptr = NULL;

    c_malloc(ptr, size);

    return ptr;
}

static inline void* c_memdup (const void* mem, csize byteSize)
{
    void* newMem = NULL;

    if (mem && (0 != byteSize)) {
        newMem = c_malloc0 (byteSize);
        memcpy (newMem, mem, byteSize);
    }

    return newMem;
}

static inline void* c_realloc (void* ptr, csize size)
{
    c_return_val_if_fail(size > 0, NULL);

    void* ret = realloc(ptr, size);

    c_assert(ret);

    return ret;
}

static inline csize c_nearest_pow (csize num)
{
    csize n = num - 1;

    c_assert (num > 0 && num <= C_MAX_SIZE / 2);

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;

#if CLIB_SIZEOF_SIZE_T == 8
    n |= n >> 32;
#endif

    return n + 1;
}


void c_abort (void);
void c_qsort_with_data (const void* pBase, cint totalElems, csize size, CCompareDataFunc compareFunc, void* udata);

bool c_direct_equal (const void* p1, const void* p2);
bool c_str_equal (const void* p1, const void* p2);
bool c_int_equal (const void* p1, const void* p2);
bool c_int64_equal (const void* p1, const void* p2);
bool c_double_equal (const void* p1, const void* p2);


// random
typedef struct _CRand CRand;
#define c_rand_boolean(rand_)   ((c_rand_int (rand_) & (1 << 15)) != 0)
#define c_random_boolean()      ((c_random_int () & (1 << 15)) != 0)

CRand*  c_rand_new_with_seed  (cuint32  seed);
CRand*  c_rand_new_with_seed_array (const cuint32 *seed, cuint seedLength);
CRand*  c_rand_new            (void);
void    c_rand_free           (CRand* rand);
CRand*  c_rand_copy           (CRand* rand);
void    c_rand_set_seed       (CRand* rand, cuint32 seed);
void    c_rand_set_seed_array (CRand* rand, const cuint32* seed, cuint seedLength);
cuint32 c_rand_int            (CRand* rand);
cint32  c_rand_int_range      (CRand* rand, cint32 begin, cint32 end);
cdouble c_rand_double         (CRand* rand);
cdouble c_rand_double_range   (CRand* rand, cdouble begin, cdouble end);
void    c_random_set_seed     (cuint32 seed);
cuint32 c_random_int          (void);
cint32  c_random_int_range    (cint32 begin, cint32 end);
cdouble c_random_double       (void);
cdouble c_random_double_range (cdouble begin, cdouble end);

/* env start */
const char* c_getenv          (const char* variable);
bool        c_setenv          (const char* variable, const char* value, bool overwrite);
void        c_unsetenv        (const char* variable);
char**      c_listenv         (void);
char**      c_get_environ     (void);
const char* c_environ_getenv  (char** envp, const char* variable);
char**      c_environ_setenv  (char** envp, const char* variable, const char* value, bool overwrite) C_WARN_UNUSED_RESULT;
char**      c_environ_unsetenv(char** envp, const char* variable) C_WARN_UNUSED_RESULT;

/* env end */
#endif
