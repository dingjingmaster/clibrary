/*************************************************************************
> FileName: macros.h
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Wed 07 Sep 2022 07:09:31 PM CST
 ************************************************************************/
#ifndef _MACROS_H
#define _MACROS_H
#include <stddef.h>

#ifdef __GNUC__
#define C_GNUC_CHECK_VERSION(major, minor) \
    ((__GNUC__ > (major)) || ((__GNUC__ == (major)) && (__GNUC_MINOR__ >= (minor))))
#else
#define G_GNUC_CHECK_VERSION(major, minor) 0
#endif

#if C_GNUC_CHECK_VERSION(2, 8)
#define C_GNUC_EXTENSION __extension__
#else
#define C_GNUC_EXTENSION
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
#define C_GNUC_PURE             __attribute__((__pure__))
#else
#define C_GNUC_PURE
#endif

/**
 * @brief
 *  void* c_malloc (unsigned long n_bytes) C_GNUC_MALLOC C_GNUC_ALLOC_SIZE(1);
 * @see
 *  [GNU C `malloc` function attribute](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-functions-that-behave-like-malloc)
 */
#if c_macro__has_attribute(__malloc__)
#define C_GNUC_MALLOC           __attribute__ ((__malloc__))
#else
#define C_GNUC_MALLOC
#endif

/**
 * @brief
 *  char* c_strconcat (const char *string1, ...) C_GNUC_NULL_TERMINATED;
 *
 * @see
 *  the [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-sentinel-function-attribute) for more details.
 */
#if c_macro__has_attribute(__sentinel__)
#define C_GNUC_NULL_TERMINATED  __attribute__((__sentinel__))
#else
#define C_GNUC_NULL_TERMINATED
#endif

/**
 * @brief
 *  void* c_malloc_n (int n_blocks, int n_block_bytes) C_GNUC_MALLOC C_GNUC_ALLOC_SIZE2(1, 2);
 * 
 * @see
 *  the [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-alloc_005fsize-function-attribute) for more details.
 */
#if c_macro__has_attribute(__alloc_size__)
#define C_GNUC_ALLOC_SIZE(x)    __attribute__((__alloc_size__(x)))
#define C_GNUC_ALLOC_SIZE2(x,y) __attribute__((__alloc_size__(x,y)))
#else
#define C_GNUC_ALLOC_SIZE(x)
#define C_GNUC_ALLOC_SIZE2(x,y)
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
#define C_GNUC_PRINTF( format_idx, arg_idx )            __attribute__((__format__ (gnu_printf, format_idx, arg_idx)))
#define C_GNUC_SCANF( format_idx, arg_idx )             __attribute__((__format__ (gnu_scanf, format_idx, arg_idx)))
#define C_GNUC_STRFTIME( format_idx )                   __attribute__((__format__ (gnu_strftime, format_idx, 0))) \
#else
#define C_GNUC_PRINTF( format_idx, arg_idx )            __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#define C_GNUC_SCANF( format_idx, arg_idx )             __attribute__((__format__ (__scanf__, format_idx, arg_idx)))
#define C_GNUC_STRFTIME( format_idx )                   __attribute__((__format__ (__strftime__, format_idx, 0))) \
#endif
#else
//
#define C_GNUC_PRINTF( format_idx, arg_idx )
#define C_GNUC_SCANF( format_idx, arg_idx )
#define C_GNUC_STRFTIME( format_idx )
#endif


/**
 * @brief
 *  char* c_dgettext (char *domain_name, char *msgid) G_GNUC_FORMAT (2);
 *
 * @see
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-Wformat-nonliteral-1) for more details.
 */
#if c_macro__has_attribute(__format_arg__)
#define C_GNUC_FORMAT(arg_idx)                          __attribute__ ((__format_arg__ (arg_idx)))
#else
#define C_GNUC_FORMAT( arg_idx )
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
#define C_GNUC_NORETURN                                 __attribute__ ((__noreturn__))
#else
#define C_GNUC_NORETURN
#endif

/**
 * @brief
 *  char c_ascii_tolower (char c) C_GNUC_CONST;
 *
 * @see
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-const-function-attribute) for more details.
 */
#if c_macro__has_attribute(__const__)
#define C_GNUC_CONST                                    __attribute__ ((__const__))
#else
#define C_GNUC_CONST
#endif

/**
 * @brief
 *  void my_unused_function (C_GNUC_UNUSED gint unused_argument, gint other_argument) C_GNUC_UNUSED;
 *
 * @see
 *  [GNU C documentation](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-unused-function-attribute) for more details.
 */
#if c_macro__has_attribute(__unused__)
#define C_GNUC_UNUSED                                   __attribute__ ((__unused__))
#else
#define C_GNUC_UNUSED
#endif








/**
 * @brief 
 *  提供 NULL 定义
 */
#ifndef NULL
#ifdef __cplusplus
#define NULL                                (0L)
#else
#define NULL                                ((void*)0)
#endif
#endif

/**
 * @brief
 *  定义 false
 */
#ifndef false
#ifdef __cplusplus
#define false                               false 
#else
#define false                               (0)
#endif
#endif


/**
 * @brief
 *  定义 true
 */
#ifndef true
#ifdef __cplusplus
#define true                                true
#else
#define true                                (!false)
#endif
#endif


/**
 * @brief
 *  定义: MAX(a, b)
 */
#undef MAX
#define MAX(a, b)                           (((a) > (b)) ? (a) : (b))

#undef MIN
#define MIN(a, b)                           (((a) < (b)) ? (a) : (b))

#undef ABS
#define ABS(a)                              (((a) < 0) ? -(a) : (a))

/**
 * @brief
 *  获取数组容量
 */
#undef C_N_ELEMENTS
#define C_N_ELEMENTS(arr)                   (sizeof(arr) / sizeof((arr)[0]))

/**
 * @brief
 *  指针转整型 && 整型转指针
 */
#undef C_POINTER_TO_SIZE           
#define C_POINTER_TO_SIZE(p)                ((unsigned long)(p))

#undef C_SIZE_TO_POINTER
#define C_SIZE_TO_POINTER(s)                ((void*) (unsigned long) (s))





#endif
