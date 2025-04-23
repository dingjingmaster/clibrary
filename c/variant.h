//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-4-23.
//

#ifndef clibrary_VARIANT_H
#define clibrary_VARIANT_H

#if !defined (__CLIB_H_INSIDE__) && !defined (CLIB_COMPILATION)
#error "Only <clib.h> can be included directly."
#endif
#include <c/macros.h>

C_BEGIN_EXTERN_C

#define C_VARIANT_TYPE_INFO_CHAR_MAYBE          'm'
#define C_VARIANT_TYPE_INFO_CHAR_ARRAY          'a'
#define C_VARIANT_TYPE_INFO_CHAR_TUPLE          '('
#define C_VARIANT_TYPE_INFO_CHAR_DICT_ENTRY     '{'
#define C_VARIANT_TYPE_INFO_CHAR_VARIANT        'v'
#define c_variant_type_info_get_type_char(info) (c_variant_type_info_get_type_string(info)[0])

#define C_VARIANT_MEMBER_ENDING_FIXED   0
#define C_VARIANT_MEMBER_ENDING_LAST    1
#define C_VARIANT_MEMBER_ENDING_OFFSET  2


typedef struct _CVariantType CVariantType;
typedef struct _CVariantTypeInfo CVariantTypeInfo;
typedef struct _CVariantMemberInfo CVariantMemberInfo;

struct _CVariantMemberInfo
{
    CVariantTypeInfo *type_info;
    csize i, a;
    cint8 b, c;
    cuint8 ending_type;
};


/**
 * @brief
 * C_VARIANT_TYPE_BOOLEAN:
 */
#define C_VARIANT_TYPE_BOOLEAN              ((const CVariantType *) "b")

/**
 * @brief
 * C_VARIANT_TYPE_BYTE: [0, 255]
 */
#define C_VARIANT_TYPE_BYTE                 ((const CVariantType *) "y")

/**
 * C_VARIANT_TYPE_INT16: [-32768, +32767]
 */
#define C_VARIANT_TYPE_INT16                ((const CVariantType *) "n")

/**
 * @brief
 * C_VARIANT_TYPE_UINT16: [0, 65535]
 */
#define C_VARIANT_TYPE_UINT16               ((const CVariantType *) "q")

/**
 * C_VARIANT_TYPE_INT32: [-2147483648, 2147483647]
 */
#define C_VARIANT_TYPE_INT32                ((const CVariantType *) "i")

/**
 * C_VARIANT_TYPE_UINT32: [0, 4294967295]
 */
#define C_VARIANT_TYPE_UINT32               ((const CVariantType *) "u")

/**
 * C_VARIANT_TYPE_INT64: [-9223372036854775808, 9223372036854775807]
 */
#define C_VARIANT_TYPE_INT64                ((const CVariantType *) "x")

/**
 * C_VARIANT_TYPE_UINT64: [0, 18446744073709551615]
 */
#define C_VARIANT_TYPE_UINT64               ((const CVariantType *) "t")

/**
 * C_VARIANT_TYPE_DOUBLE:
 * [IEEE 754 floating point number](https://en.wikipedia.org/wiki/IEEE_754).
 */
#define C_VARIANT_TYPE_DOUBLE               ((const CVariantType *) "d")

/**
 * C_VARIANT_TYPE_STRING:
 * `""` is a string.  `NULL` is not a string.
 */
#define C_VARIANT_TYPE_STRING               ((const CVariantType *) "s")

/**
 * C_VARIANT_TYPE_OBJECT_PATH:
 */
#define C_VARIANT_TYPE_OBJECT_PATH          ((const CVariantType *) "o")

/**
 * C_VARIANT_TYPE_SIGNATURE:
 */
#define C_VARIANT_TYPE_SIGNATURE            ((const CVariantType *) "g")

/**
 * C_VARIANT_TYPE_VARIANT:
 */
#define C_VARIANT_TYPE_VARIANT              ((const CVariantType *) "v")

/**
 * C_VARIANT_TYPE_HANDLE:
 */
#define C_VARIANT_TYPE_HANDLE               ((const CVariantType *) "h")

/**
 * C_VARIANT_TYPE_UNIT:
 */
#define C_VARIANT_TYPE_UNIT                 ((const CVariantType *) "()")

/**
 * C_VARIANT_TYPE_ANY:
 */
#define C_VARIANT_TYPE_ANY                  ((const CVariantType *) "*")

/**
 * C_VARIANT_TYPE_BASIC:
 */
#define C_VARIANT_TYPE_BASIC                ((const CVariantType *) "?")

/**
 * C_VARIANT_TYPE_MAYBE:
 */
#define C_VARIANT_TYPE_MAYBE                ((const CVariantType *) "m*")

/**
 * C_VARIANT_TYPE_ARRAY:
 */
#define C_VARIANT_TYPE_ARRAY                ((const CVariantType *) "a*")

/**
 * C_VARIANT_TYPE_TUPLE:
 */
#define C_VARIANT_TYPE_TUPLE                ((const CVariantType *) "r")

/**
 * C_VARIANT_TYPE_DICT_ENTRY:
 */
#define C_VARIANT_TYPE_DICT_ENTRY           ((const CVariantType *) "{?*}")

/**
 * C_VARIANT_TYPE_DICTIONARY:
 */
#define C_VARIANT_TYPE_DICTIONARY           ((const CVariantType *) "a{?*}")

/**
 * C_VARIANT_TYPE_STRING_ARRAY:
 */
#define C_VARIANT_TYPE_STRING_ARRAY         ((const CVariantType *) "as")

/**
 * C_VARIANT_TYPE_OBJECT_PATH_ARRAY:
 */
#define C_VARIANT_TYPE_OBJECT_PATH_ARRAY    ((const CVariantType *) "ao")

/**
 * C_VARIANT_TYPE_BYTESTRING:
 */
#define C_VARIANT_TYPE_BYTESTRING           ((const CVariantType *) "ay")

/**
 * C_VARIANT_TYPE_BYTESTRING_ARRAY:
 */
#define C_VARIANT_TYPE_BYTESTRING_ARRAY     ((const CVariantType *) "aay")

/**
 * C_VARIANT_TYPE_VARDICT:
 */
#define C_VARIANT_TYPE_VARDICT              ((const CVariantType *) "a{sv}")

#ifndef C_DISABLE_CAST_CHECKS
# define C_VARIANT_TYPE(typeString)         (c_variant_type_checked_ ((typeString)))
#else
# define C_VARIANT_TYPE(typeString)         ((const CVariantType *) (typeString))
#endif

#define C_VARIANT_MAX_RECURSION_DEPTH       ((csize) 128)

bool                        c_variant_type_string_is_valid          (const cchar* typeStr) C_CONST;
bool                        c_variant_type_string_scan              (const cchar* str, const cchar* limit, const cchar** endPtr);
void                        c_variant_type_free                     (CVariantType* type);
CVariantType*               c_variant_type_copy                     (const CVariantType* type);
CVariantType*               c_variant_type_new                      (const cchar* typeStr);

csize                       c_variant_type_get_string_length        (const CVariantType* type);
const cchar*                c_variant_type_peek_string              (const CVariantType* type);
cchar*                      c_variant_type_dup_string               (const CVariantType* type);

bool                        c_variant_type_is_definite              (const CVariantType* type) C_CONST;
bool                        c_variant_type_is_container             (const CVariantType* type) C_CONST;
bool                        c_variant_type_is_basic                 (const CVariantType* type) C_CONST;
bool                        c_variant_type_is_maybe                 (const CVariantType* type) C_CONST;
bool                        c_variant_type_is_array                 (const CVariantType* type) C_CONST;
bool                        c_variant_type_is_tuple                 (const CVariantType* type) C_CONST;
bool                        c_variant_type_is_dict_entry            (const CVariantType* type) C_CONST;
bool                        c_variant_type_is_variant               (const CVariantType* type) C_CONST;

cuint                       c_variant_type_hash                     (void const* type);
bool                        c_variant_type_equal                    (void const* type1, void const* type2);

/* subtypes */
bool                        c_variant_type_is_subtype_of            (const CVariantType* type, const CVariantType* supertype) C_CONST;

/* type iterator interface */
const CVariantType*         c_variant_type_element                  (const CVariantType* type) C_CONST;
const CVariantType*         c_variant_type_first                    (const CVariantType* type) C_CONST;
const CVariantType*         c_variant_type_next                     (const CVariantType* type) C_CONST;
csize                       c_variant_type_n_items                  (const CVariantType* type) C_CONST;
const CVariantType*         c_variant_type_key                      (const CVariantType* type) C_CONST;
const CVariantType*         c_variant_type_value                    (const CVariantType* type) C_CONST;

/* constructors */
CVariantType*               c_variant_type_new_array                (const CVariantType* element);
CVariantType*               c_variant_type_new_maybe                (const CVariantType* element);
CVariantType*               c_variant_type_new_tuple                (const CVariantType* const *items, cint length);
CVariantType*               c_variant_type_new_dict_entry           (const CVariantType* key, const CVariantType* value);

/*< private >*/
const CVariantType*         c_variant_type_checked_                 (const cchar* typeStr);
csize                       c_variant_type_string_get_depth_        (const cchar* typeStr);


const cchar*                c_variant_type_info_get_type_string     (CVariantTypeInfo* typeinfo);
void                        c_variant_type_info_query               (CVariantTypeInfo* typeinfo, cuint* alignment, csize* size);
csize                       c_variant_type_info_query_depth         (CVariantTypeInfo* typeinfo);

/* array */
CVariantTypeInfo*           c_variant_type_info_element             (CVariantTypeInfo* typeinfo);
void                        c_variant_type_info_query_element       (CVariantTypeInfo* typeinfo, cuint* alignment, csize* size);

/* structure */
csize                       c_variant_type_info_n_members           (CVariantTypeInfo* typeinfo);
const CVariantMemberInfo*   c_variant_type_info_member_info         (CVariantTypeInfo* typeinfo, csize index);

/* new/ref/unref */
CVariantTypeInfo*           c_variant_type_info_get                 (const CVariantType* type);
CVariantTypeInfo*           c_variant_type_info_ref                 (CVariantTypeInfo* typeinfo);
void                        c_variant_type_info_unref               (CVariantTypeInfo* typeinfo);
void                        c_variant_type_info_assert_no_infos     (void);



C_END_EXTERN_C

#endif // clibrary_VARIANT_H
