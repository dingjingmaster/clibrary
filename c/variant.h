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

#define C_VARIANT_MAX_RECURSION_DEPTH ((csize) 128)

#if SIZEOF_VOID_P == 8
# define C_VARIANT_MAX_PREALLOCATED 64
#else
# define C_VARIANT_MAX_PREALLOCATED 32
#endif


typedef struct _CVariant CVariant;
typedef struct _CVariantType CVariantType;
typedef struct _CVariantTypeInfo CVariantTypeInfo;
typedef struct _CVariantMemberInfo CVariantMemberInfo;

typedef enum
{
    C_VARIANT_CLASS_BOOLEAN       = 'b',
    C_VARIANT_CLASS_BYTE          = 'y',
    C_VARIANT_CLASS_INT16         = 'n',
    C_VARIANT_CLASS_UINT16        = 'q',
    C_VARIANT_CLASS_INT32         = 'i',
    C_VARIANT_CLASS_UINT32        = 'u',
    C_VARIANT_CLASS_INT64         = 'x',
    C_VARIANT_CLASS_UINT64        = 't',
    C_VARIANT_CLASS_HANDLE        = 'h',
    C_VARIANT_CLASS_DOUBLE        = 'd',
    C_VARIANT_CLASS_STRING        = 's',
    C_VARIANT_CLASS_OBJECT_PATH   = 'o',
    C_VARIANT_CLASS_SIGNATURE     = 'g',
    C_VARIANT_CLASS_VARIANT       = 'v',
    C_VARIANT_CLASS_MAYBE         = 'm',
    C_VARIANT_CLASS_ARRAY         = 'a',
    C_VARIANT_CLASS_TUPLE         = '(',
    C_VARIANT_CLASS_DICT_ENTRY    = '{'
} CVariantClass;

struct _CVariantMemberInfo
{
    CVariantTypeInfo *type_info;
    csize i, a;
    cint8 b, c;
    cuint8 ending_type;
};

typedef struct
{
    CVariantTypeInfo *type_info;
    cuchar           *data;
    csize             size;
    csize             depth;  /* same semantics as GVariant.depth */
    csize             ordered_offsets_up_to;
    csize             checked_offsets_up_to;
} CVariantSerialised;


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


typedef void                (*CVariantSerialisedFiller)             (CVariantSerialised* serialised, void* data);


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


void                    c_variant_unref                     (CVariant* value);
CVariant*               c_variant_ref                       (CVariant* value);
CVariant*               c_variant_ref_sink                  (CVariant* value);
bool                    c_variant_is_floating               (CVariant* value);
CVariant*               c_variant_take_ref                  (CVariant* value);
const CVariantType*     c_variant_get_type                  (CVariant* value);
const cchar*            c_variant_get_type_string           (CVariant* value);
bool                    c_variant_is_of_type                (CVariant* value, const CVariantType* type);
bool                    c_variant_is_container              (CVariant* value);
CVariantClass           c_variant_classify                  (CVariant* value);
CVariant*               c_variant_new_boolean               (bool value);
CVariant*               c_variant_new_byte                  (cuint8 value);
CVariant*               c_variant_new_int16                 (cint16 value);
CVariant*               c_variant_new_uint16                (cuint16 value);
CVariant*               c_variant_new_int32                 (cint32 value);
CVariant*               c_variant_new_uint32                (cuint32 value);
CVariant*               c_variant_new_int64                 (cint64 value);
CVariant*               c_variant_new_uint64                (cuint64 value);
CVariant*               c_variant_new_handle                (cint32 value);
CVariant*               c_variant_new_double                (cdouble value);
CVariant*               c_variant_new_string                (const cchar* str);
CVariant*               c_variant_new_take_string           (cchar* str);
CVariant*               c_variant_new_printf                (const cchar* formatStr, ...) C_PRINTF (1, 2);
CVariant*               c_variant_new_object_path           (const cchar* objectPath);
bool                    c_variant_is_object_path            (const cchar* str);
CVariant*               c_variant_new_signature             (const cchar* signature);
bool                    c_variant_is_signature              (const cchar* str);
CVariant*               c_variant_new_variant               (CVariant* value);
CVariant*               c_variant_new_strv                  (const cchar* const* strv, cssize len);
CVariant*               c_variant_new_objv                  (const cchar* const* strv, cssize len);
CVariant*               c_variant_new_bytestring            (const cchar* str);
CVariant*               c_variant_new_bytestring_array      (const cchar* const* strv, cssize len);
CVariant*               c_variant_new_fixed_array           (const CVariantType* eleType, const void* ele, csize nEle, csize eleSize);
bool                    c_variant_get_boolean               (CVariant* value);
cuint8                  c_variant_get_byte                  (CVariant* value);
cint16                  c_variant_get_int16                 (CVariant* value);
cuint16                 c_variant_get_uint16                (CVariant* value);
cint32                  c_variant_get_int32                 (CVariant* value);
cuint32                 c_variant_get_uint32                (CVariant* value);
cint64                  c_variant_get_int64                 (CVariant* value);
cuint64                 c_variant_get_uint64                (CVariant* value);
cint32                  c_variant_get_handle                (CVariant* value);
cdouble                 c_variant_get_double                (CVariant* value);
CVariant*               c_variant_get_variant               (CVariant* value);
const cchar*            c_variant_get_string                (CVariant* value, csize* length);
cchar*                  c_variant_dup_string                (CVariant* value, csize* length);
const cchar**           c_variant_get_strv                  (CVariant* value, csize* length);
cchar**                 c_variant_dup_strv                  (CVariant* value, csize* length);
const cchar**           c_variant_get_objv                  (CVariant* value, csize* length);
cchar**                 c_variant_dup_objv                  (CVariant* value, csize* length);
const cchar*            c_variant_get_bytestring            (CVariant* value);
cchar*                  c_variant_dup_bytestring            (CVariant* value, csize* length);
const cchar**           c_variant_get_bytestring_array      (CVariant* value, csize* length);
cchar**                 c_variant_dup_bytestring_array      (CVariant* value, csize* length);

CVariant*               c_variant_new_maybe                 (const CVariantType* childType, CVariant* child);
CVariant*               c_variant_new_array                 (const CVariantType* childType, CVariant* const* child, csize nChild);
CVariant*               c_variant_new_tuple                 (CVariant* const* children, csize nChild);
CVariant*               c_variant_new_dict_entry            (CVariant* key, CVariant* value);

CVariant*               c_variant_get_maybe                 (CVariant* value);
csize                   c_variant_n_children                (CVariant* value);
void                    c_variant_get_child                 (CVariant* value, csize idx, const cchar* formatStr, ...);
CVariant*               c_variant_get_child_value           (CVariant* value, csize idx);
bool                    c_variant_lookup                    (CVariant* dictionary, const cchar* key, const cchar *formatStr, ...);
CVariant*               c_variant_lookup_value              (CVariant* dictionary, const cchar* key, const CVariantType* expectedType);
const void*             c_variant_get_fixed_array           (CVariant* value, csize* nEles, csize elementSize);
csize                   c_variant_get_size                  (CVariant* value);
const void*             c_variant_get_data                  (CVariant* value);
CBytes*                 c_variant_get_data_as_bytes         (CVariant* value);
void                    c_variant_store                     (CVariant* value, void* data);
cchar*                  c_variant_print                     (CVariant* value, bool type_annotate);
CString*                c_variant_print_string              (CVariant* value, CString* string, bool type_annotate);
cuint                   c_variant_hash                      (const void* value);
bool                    c_variant_equal                     (const void* one, const void* two);
CVariant*               c_variant_get_normal_form           (CVariant* value);
bool                    c_variant_is_normal_form            (CVariant* value);
CVariant*               c_variant_byteswap                  (CVariant* value);
CVariant*               c_variant_new_from_bytes            (const CVariantType* type, CBytes* bytes, bool trusted);
CVariant*               c_variant_new_from_data             (const CVariantType* type, const void* data, csize size, bool trusted, CDestroyNotify notify, void* user_data);

typedef struct _CVariantIter CVariantIter;
struct _CVariantIter
{
    /*< private >*/
    cuintptr x[16];
};

CVariantIter*           c_variant_iter_new                  (CVariant* value);
csize                   c_variant_iter_init                 (CVariantIter* iter, CVariant* value);
CVariantIter*           c_variant_iter_copy                 (CVariantIter* iter);
csize                   c_variant_iter_n_children           (CVariantIter* iter);
void                    c_variant_iter_free                 (CVariantIter* iter);
CVariant*               c_variant_iter_next_value           (CVariantIter* iter);
bool                    c_variant_iter_next                 (CVariantIter* iter, const cchar* formatStr, ...);
bool                    c_variant_iter_loop                 (CVariantIter* iter, const cchar* formatStr, ...);


typedef struct _CVariantBuilder CVariantBuilder;
struct _CVariantBuilder {
    /*< private >*/
    union {
        struct {
            csize partial_magic;
            const CVariantType *type;
            cuintptr y[14];
        } s;
        cuintptr x[16];
    } u;
};

typedef enum
{
    C_VARIANT_PARSE_ERROR_FAILED,
    C_VARIANT_PARSE_ERROR_BASIC_TYPE_EXPECTED,
    C_VARIANT_PARSE_ERROR_CANNOT_INFER_TYPE,
    C_VARIANT_PARSE_ERROR_DEFINITE_TYPE_EXPECTED,
    C_VARIANT_PARSE_ERROR_INPUT_NOT_AT_END,
    C_VARIANT_PARSE_ERROR_INVALID_CHARACTER,
    C_VARIANT_PARSE_ERROR_INVALID_FORMAT_STRING,
    C_VARIANT_PARSE_ERROR_INVALID_OBJECT_PATH,
    C_VARIANT_PARSE_ERROR_INVALID_SIGNATURE,
    C_VARIANT_PARSE_ERROR_INVALID_TYPE_STRING,
    C_VARIANT_PARSE_ERROR_NO_COMMON_TYPE,
    C_VARIANT_PARSE_ERROR_NUMBER_OUT_OF_RANGE,
    C_VARIANT_PARSE_ERROR_NUMBER_TOO_BIG,
    C_VARIANT_PARSE_ERROR_TYPE_ERROR,
    C_VARIANT_PARSE_ERROR_UNEXPECTED_TOKEN,
    C_VARIANT_PARSE_ERROR_UNKNOWN_KEYWORD,
    C_VARIANT_PARSE_ERROR_UNTERMINATED_STRING_CONSTANT,
    C_VARIANT_PARSE_ERROR_VALUE_EXPECTED,
    C_VARIANT_PARSE_ERROR_RECURSION
} CVariantParseError;
#define C_VARIANT_PARSE_ERROR (c_variant_parse_error_quark ())

CQuark                          c_variant_parser_get_error_quark    (void);
CQuark                          c_variant_parse_error_quark         (void);

#define C_VARIANT_BUILDER_INIT(variant_type) { { { 2942751021u, variant_type, { 0, } } } }
#define C_VARIANT_BUILDER_INIT_UNSET() { 0, }


CVariantBuilder*        c_variant_builder_new               (const CVariantType* type);
void                    c_variant_builder_unref             (CVariantBuilder* builder);
CVariantBuilder*        c_variant_builder_ref               (CVariantBuilder* builder);
void                    c_variant_builder_init              (CVariantBuilder* builder, const CVariantType* type);
void                    c_variant_builder_init_static       (CVariantBuilder* builder, const CVariantType* type);
CVariant*               c_variant_builder_end               (CVariantBuilder* builder);
void                    c_variant_builder_clear             (CVariantBuilder* builder);
void                    c_variant_builder_open              (CVariantBuilder* builder, const CVariantType* type);
void                    c_variant_builder_close             (CVariantBuilder* builder);
void                    c_variant_builder_add_value         (CVariantBuilder* builder, CVariant* value);
void                    c_variant_builder_add               (CVariantBuilder* builder, const cchar* formatStr, ...);
void                    c_variant_builder_add_parsed        (CVariantBuilder* builder, const cchar* format, ...);
CVariant*               c_variant_new                       (const cchar* formatStr, ...);
void                    c_variant_get                       (CVariant* value, const cchar* formatStr, ...);
CVariant*               c_variant_new_va                    (const cchar* formatStr, const cchar** endPtr, va_list* app);
void                    c_variant_get_va                    (CVariant* value, const cchar* formatStr, const cchar** endPtr, va_list* app);
bool                    c_variant_check_format_string       (CVariant* value, const cchar* formatStr, bool copyOnly);
CVariant*               c_variant_parse                     (const CVariantType* type, const cchar* text, const cchar* limit, const cchar** endPtr, CError** error);
CVariant*               c_variant_new_parsed                (const cchar* format, ...);
CVariant*               c_variant_new_parsed_va             (const cchar* format, va_list* app);
cchar*                  c_variant_parse_error_print_context (CError* error, const cchar* sourceStr);
cint                    c_variant_compare                   (const void* one, const void* two);

typedef struct _CVariantDict CVariantDict;
struct _CVariantDict {
    /*< private >*/
    union {
        struct {
            CVariant *asv;
            csize partial_magic;
            cuintptr y[14];
        } s;
        cuintptr x[16];
    } u;
};

#define C_VARIANT_DICT_INIT(asv) { { { asv, 3488698669u, { 0, } } } }

CVariantDict*           c_variant_dict_new                  (CVariant* fromAsv);
void                    c_variant_dict_init                 (CVariantDict* dict, CVariant* fromAsv);
bool                    c_variant_dict_lookup               (CVariantDict* dict, const cchar* key, const cchar* formatStr, ...);
CVariant*               c_variant_dict_lookup_value         (CVariantDict* dict, const cchar* key, const CVariantType* expectedType);
bool                    c_variant_dict_contains             (CVariantDict* dict, const cchar* key);
void                    c_variant_dict_insert               (CVariantDict* dict, const cchar* key, const cchar* formatStr, ...);
void                    c_variant_dict_insert_value         (CVariantDict* dict, const cchar* key, CVariant* value);
bool                    c_variant_dict_remove               (CVariantDict* dict, const cchar* key);
void                    c_variant_dict_clear                (CVariantDict* dict);
CVariant*               c_variant_dict_end                  (CVariantDict* dict);
CVariantDict*           c_variant_dict_ref                  (CVariantDict* dict);
void                    c_variant_dict_unref                (CVariantDict* dict);

// Core
CVariantTypeInfo*       c_variant_get_type_info             (CVariant* value);
bool                    c_variant_is_trusted                (CVariant* value);
csize                   c_variant_get_depth                 (CVariant* value);
CVariant*               c_variant_maybe_get_child_value     (CVariant *value, csize index_);
CVariant*               c_variant_new_take_bytes            (const CVariantType* type, CBytes* bytes, bool trusted);
CVariant*               c_variant_new_preallocated_trusted  (const CVariantType* type, const void* data, csize size);
CVariant*               c_variant_new_from_children         (const CVariantType* type, CVariant** children, csize n_children, bool trusted);

// serialiser
bool                    c_variant_serialised_check          (CVariantSerialised serialised);
csize                   c_variant_serialised_n_children     (CVariantSerialised serialised);
CVariantSerialised      c_variant_serialised_get_child      (CVariantSerialised serialised, csize index_);
void                    c_variant_serialiser_serialise      (CVariantSerialised serialised, CVariantSerialisedFiller gvs_filler, const void** children, csize n_children);
csize                   c_variant_serialiser_needed_size    (CVariantTypeInfo* type_info, CVariantSerialisedFiller gvs_filler, const void** children, csize n_children);
void                    c_variant_serialised_byteswap       (CVariantSerialised serialised);
bool                    c_variant_serialised_is_normal      (CVariantSerialised serialised);
bool                    c_variant_serialiser_is_string      (const void* data, csize size);
bool                    c_variant_serialiser_is_object_path (const void* data, csize size);
bool                    c_variant_serialiser_is_signature   (const void* data, csize size);

bool                    c_variant_format_string_scan        (const cchar* str, const cchar* limit, const cchar** endPtr);
CVariantType*           c_variant_format_string_scan_type   (const cchar* str, const cchar* limit, const cchar** endPtr);

C_END_EXTERN_C

#endif // clibrary_VARIANT_H
