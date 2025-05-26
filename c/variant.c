//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the “Software”), to deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright
//  notice and this permission notice shall be included in all copies or substantial portions of the Software. THE
//  SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
//  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
//  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-4-23.
//

#include "variant.h"

#include "array.h"
#include "atomic.h"
#include "bit-lock.h"
#include "bytes.h"
#include "cstring.h"
#include "hash-table.h"
#include "log.h"
#include "str.h"
#include "thread.h"
#include "unicode.h"

#define STATE_LOCKED 1
#define STATE_SERIALISED 2
#define STATE_TRUSTED 4
#define STATE_FLOATING 8

#define TYPE_CHECK(value, TYPE, val)                                                                                   \
    if C_UNLIKELY (!c_variant_is_of_type(value, TYPE)) {                                                               \
        c_warn_if_fail("c_variant_is_of_type (" #value ", " #TYPE ")");                                                \
        return val;                                                                                                    \
    }

#define NUMERIC_TYPE(TYPE, type, ctype)                                                                                \
    CVariant *c_variant_new_##type(ctype value)                                                                        \
    {                                                                                                                  \
        return c_variant_new_from_trusted(C_VARIANT_TYPE_##TYPE, &value, sizeof value);                                \
    }                                                                                                                  \
    ctype c_variant_get_##type(CVariant *value)                                                                        \
    {                                                                                                                  \
        const ctype *data;                                                                                             \
        TYPE_CHECK(value, C_VARIANT_TYPE_##TYPE, 0);                                                                   \
        data = c_variant_get_data(value);                                                                              \
        return data != NULL ? *data : 0;                                                                               \
    }

#define DISPATCH_FIXED(type_info, before, after)                                                                       \
    {                                                                                                                  \
        csize fixed_size;                                                                                              \
        c_variant_type_info_query_element(type_info, NULL, &fixed_size);                                               \
        if (fixed_size) {                                                                                              \
            before##fixed_sized##after                                                                                 \
        }                                                                                                              \
        else {                                                                                                         \
            before##variable_sized##after                                                                              \
        }                                                                                                              \
    }

#define DISPATCH_CASES(type_info, before, after)                                                                       \
    switch (c_variant_type_info_get_type_char(type_info)) {                                                            \
    case C_VARIANT_TYPE_INFO_CHAR_MAYBE:                                                                               \
        DISPATCH_FIXED(type_info, before, _maybe##after)                                                               \
                                                                                                                       \
    case C_VARIANT_TYPE_INFO_CHAR_ARRAY:                                                                               \
        DISPATCH_FIXED(type_info, before, _array##after)                                                               \
                                                                                                                       \
    case C_VARIANT_TYPE_INFO_CHAR_DICT_ENTRY:                                                                          \
    case C_VARIANT_TYPE_INFO_CHAR_TUPLE: {                                                                             \
        before##tuple##after                                                                                           \
    }                                                                                                                  \
                                                                                                                       \
    case C_VARIANT_TYPE_INFO_CHAR_VARIANT: {                                                                           \
        before##variant##after                                                                                         \
    }                                                                                                                  \
    }

#define CVSD(d) ((struct stack_dict *)(d))
#define CVHD(d) ((struct heap_dict *)(d))
#define CVSD_MAGIC ((csize)2579507750u)
#define CVSD_MAGIC_PARTIAL ((csize)3488698669u)
#define CVHD_MAGIC ((csize)2450270775u)
#define is_valid_dict(d) (CVSD(d)->magic == CVSD_MAGIC)
#define is_valid_heap_dict(d) (CVHD(d)->magic == CVHD_MAGIC)

#define return_if_invalid_dict(d)                                                                                      \
    C_STMT_START                                                                                                       \
    {                                                                                                                  \
        bool valid_dict C_UNUSED = ensure_valid_dict(d);                                                               \
        c_return_if_fail(valid_dict);                                                                                  \
    }                                                                                                                  \
    C_STMT_END

#define return_val_if_invalid_dict(d, val)                                                                             \
    C_STMT_START                                                                                                       \
    {                                                                                                                  \
        bool valid_dict C_UNUSED = ensure_valid_dict(d);                                                               \
        c_return_val_if_fail(valid_dict, val);                                                                         \
    }                                                                                                                  \
    C_STMT_END

#define CVSB(b) ((struct stack_builder *)(b))
#define CVHB(b) ((struct heap_builder *)(b))
#define CVSB_MAGIC ((csize)1033660112u)
#define CVSB_MAGIC_PARTIAL ((csize)2942751021u)
#define CVHB_MAGIC ((csize)3087242682u)
#define is_valid_builder(b) (CVSB(b)->magic == CVSB_MAGIC)
#define is_valid_heap_builder(b) (CVHB(b)->magic == CVHB_MAGIC)

#define return_if_invalid_builder(b)                                                                                   \
    C_STMT_START                                                                                                       \
    {                                                                                                                  \
        bool valid_builder C_UNUSED = ensure_valid_builder(b);                                                         \
        c_return_if_fail(valid_builder);                                                                               \
    }                                                                                                                  \
    C_STMT_END

#define return_val_if_invalid_builder(b, val)                                                                          \
    C_STMT_START                                                                                                       \
    {                                                                                                                  \
        bool valid_builder C_UNUSED = ensure_valid_builder(b);                                                         \
        c_return_val_if_fail(valid_builder, val);                                                                      \
    }                                                                                                                  \
    C_STMT_END

#define CVSI(i) ((struct stack_iter *)(i))
#define CVHI(i) ((struct heap_iter *)(i))
#define CVSI_MAGIC ((csize)3579507750u)
#define CVHI_MAGIC ((csize)1450270775u)
#define is_valid_iter(i) (i != NULL && CVSI(i)->magic == CVSI_MAGIC)
#define is_valid_heap_iter(i) (is_valid_iter(i) && CVHI(i)->magic == CVHI_MAGIC)

typedef struct _AST AST;

struct _CVariant
{
    CVariantTypeInfo *type_info;
    csize size;

    union
    {
        struct
        {
            CBytes *bytes;
            const void *data;
            csize ordered_offsets_up_to;
            csize checked_offsets_up_to;
        } serialised;

        struct
        {
            CVariant **children;
            csize n_children;
        } tree;
    } contents;

    cint state;
    catomicrefcount ref_count;
    csize depth;

#if GLIB_SIZEOF_VOID_P == 4
    /* Keep suffix aligned to 8 bytes */
    guint _padding;
#endif
    cuint8 suffix[];
};
C_STATIC_ASSERT(C_STRUCT_OFFSET(CVariant, suffix) % 8 == 0);

struct stack_builder
{
    CVariantBuilder *parent;
    CVariantType *type;
    const CVariantType *expected_type;
    const CVariantType *prev_item_type;
    csize min_items;
    csize max_items;
    CVariant **children;
    csize allocated_children;
    csize offset;
    cuint uniform_item_types : 1;
    cuint trusted : 1;
    cuint type_owned : 1;
    csize magic;
};
C_STATIC_ASSERT(sizeof(struct stack_builder) <= sizeof(CVariantBuilder));

struct heap_builder
{
    CVariantBuilder builder;
    csize magic;

    cint ref_count;
};

C_STATIC_ASSERT(sizeof(CVariantBuilder) == sizeof(cuintptr[16]));

struct stack_iter
{
    CVariant *value;
    cssize n, i;

    const cchar *loop_format;

    csize padding[3];
    csize magic;
};

C_STATIC_ASSERT(sizeof(struct stack_iter) <= sizeof(CVariantIter));

struct heap_iter
{
    struct stack_iter iter;

    CVariant *value_ref;
    csize magic;
};

C_STATIC_ASSERT(sizeof(struct heap_iter) <= sizeof(CVariantIter));

struct _CVariantTypeInfo
{
    csize fixed_size;
    cuchar alignment;
    cuchar container_class;
};

typedef struct
{
    CVariantTypeInfo info;

    cchar *type_string;
    catomicrefcount ref_count;
} ContainerInfo;

typedef struct
{
    ContainerInfo container;

    CVariantTypeInfo *element;
} ArrayInfo;

typedef struct
{
    ContainerInfo container;

    CVariantMemberInfo *members;
    csize n_members;
} TupleInfo;

struct Offsets
{
    csize data_size;

    cuchar *array;
    csize length;
    cuint offset_size;

    bool is_normal;
};

typedef struct
{
    cint start, end;
} SourceRef;

// dict
struct stack_dict
{
    CHashTable *values;
    csize magic;
};
C_STATIC_ASSERT(sizeof(struct stack_dict) <= sizeof(CVariantDict));

struct heap_dict
{
    struct stack_dict dict;
    cint ref_count;
    csize magic;
};
typedef struct
{
    const cchar *start;
    const cchar *stream;
    const cchar *end;
    const cchar *this;
} TokenStream;

typedef struct
{
    cchar *(*get_pattern)(AST *ast, CError **error);
    CVariant *(*get_value)(AST *ast, const CVariantType *type, CError **error);
    CVariant *(*get_base_value)(AST *ast, const CVariantType *type, CError **error);
    void (*free)(AST *ast);
} ASTClass;

struct _AST
{
    const ASTClass *class;
    SourceRef source_ref;
};

typedef struct
{
    AST ast;
    AST **children;
    cint n_children;
} Array;

typedef struct
{
    AST ast;
    AST **children;
    cint n_children;
} Tuple;

typedef struct
{
    AST ast;
    AST *child;
} Maybe;

typedef struct
{
    AST ast;
    AST *value;
} Variant;

typedef struct
{
    AST ast;
    AST **keys;
    AST **values;
    cint n_children;
} Dictionary;

typedef struct
{
    AST ast;
    cchar *string;
} String;

typedef struct
{
    AST ast;
    cchar *string;
} ByteString;

typedef struct
{
    AST ast;
    cchar *token;
} Number;

typedef struct
{
    AST ast;
    bool value;
} Boolean;

typedef struct
{
    AST ast;
    CVariant *value;
} Positional;

typedef struct
{
    AST ast;
    CVariantType *type;
    AST *child;
} TypeDecl;

/* == array == */
#define CV_ARRAY_INFO_CLASS 'a'
static void array_info_free(CVariantTypeInfo *info);
static ArrayInfo *CV_ARRAY_INFO(CVariantTypeInfo *info);
static ContainerInfo *array_info_new(const CVariantType *type);

/* == tuple == */
#define CV_TUPLE_INFO_CLASS 'r'
static void tuple_set_base_info(TupleInfo *info);
static void tuple_generate_table(TupleInfo *info);
static void tuple_info_free(CVariantTypeInfo *info);
static csize tuple_align(csize offset, cuint alignment);
static TupleInfo *CV_TUPLE_INFO(CVariantTypeInfo *info);
static ContainerInfo *tuple_info_new(const CVariantType *type);
static bool tuple_get_item(TupleInfo *info, CVariantMemberInfo *item, csize *d, csize *e);
static void tuple_table_append(CVariantMemberInfo **items, csize i, csize a, csize b, csize c);
static void tuple_allocate_members(const CVariantType *type, CVariantMemberInfo **members, csize *n_members);


C_DEFINE_QUARK(c - variant - parse - error - quark, c_variant_parse_error)
CQuark g_variant_parser_get_error_quark(void) { return c_variant_parse_error_quark(); }

static void ast_free(AST *ast);
static void maybe_free(AST *ast);
static void tuple_free(AST *ast);
static void number_free(AST *ast);
static void string_free(AST *ast);
static void variant_free(AST *ast);
static void boolean_free(AST *ast);
static AST *boolean_new(bool value);
static void typedecl_free(AST *ast);
static void bytestring_free(AST *ast);
static void dictionary_free(AST *ast);
static void positional_free(AST *ast);
static bool ensure_valid_dict(CVariantDict *dict);
static void token_stream_next(TokenStream *stream);
static cchar *token_stream_get(TokenStream *stream);
static bool token_stream_prepare(TokenStream *stream);
static void ast_array_free(AST **array, cint n_items);
static CVariant *ast_resolve(AST *ast, CError **error);
static cchar *ast_get_pattern(AST *ast, CError **error);
static void pattern_copy(cchar **out, const cchar **in);
static bool token_stream_is_keyword(TokenStream *stream);
static bool token_stream_is_numeric(TokenStream *stream);
static cchar *maybe_get_pattern(AST *ast, CError **error);
static void add_last_line(CString *err, const cchar *str);
static cchar *tuple_get_pattern(AST *ast, CError **error);
static bool ensure_valid_builder(CVariantBuilder *builder);
static cchar *number_get_pattern(AST *ast, CError **error);
static cchar *string_get_pattern(AST *ast, CError **error);
static cchar *variant_get_pattern(AST *ast, CError **error);
static cchar *boolean_get_pattern(AST *ast, CError **error);
static cchar *typedecl_get_pattern(AST *ast, CError **error);
static bool c_variant_format_string_is_nnp(const cchar *str);
static bool c_variant_format_string_is_leaf(const cchar *str);
static cchar *bytestring_get_pattern(AST *ast, CError **error);
static cchar *dictionary_get_pattern(AST *ast, CError **error);
static cchar *positional_get_pattern(AST *ast, CError **error);
static CVariantType *c_variant_make_maybe_type(CVariant *element);
static CVariantType *c_variant_make_array_type(CVariant *element);
static void c_variant_valist_skip(const cchar **str, va_list *app);
static void c_variant_valist_free_nnp(const cchar *str, void *ptr);
static void ast_array_append(AST ***array, cint *n_items, AST *ast);
static CVariant *c_variant_deep_copy(CVariant *value, bool byteSwap);
static bool token_stream_peek(TokenStream *stream, cchar first_char);
static void token_stream_end_ref(TokenStream *stream, SourceRef *ref);
static cchar *pattern_coalesce(const cchar *left, const cchar *right);
static void c_variant_builder_make_room(struct stack_builder *builder);
static CVariant *c_variant_valist_new(const cchar **str, va_list *app);
static CVariant *c_variant_valist_new_nnp(const cchar **str, void *ptr);
static void c_variant_valist_skip_leaf(const cchar **str, va_list *app);
static void token_stream_start_ref(TokenStream *stream, SourceRef *ref);
static void token_stream_assert(TokenStream *stream, const cchar *token);
static bool token_stream_consume(TokenStream *stream, const cchar *token);
static void *c_variant_valist_get_nnp(const cchar **str, CVariant *value);
static bool parse_num(const cchar *num, const cchar *limit, cuint *result);
static CVariant *c_variant_valist_new_leaf(const cchar **str, va_list *app);
static AST *string_parse(TokenStream *stream, va_list *app, CError **error);
static AST *number_parse(TokenStream *stream, va_list *app, CError **error);
static bool token_stream_peek_string(TokenStream *stream, const cchar *token);
static cchar *ast_array_get_pattern(AST **array, cint n_items, CError **error);
static AST *bytestring_parse(TokenStream *stream, va_list *app, CError **error);
static AST *positional_parse(TokenStream *stream, va_list *app, CError **error);
static CVariant *maybe_wrapper(AST *ast, const CVariantType *type, CError **error);
static CVariant *ast_get_value(AST *ast, const CVariantType *type, CError **error);
static CVariant *ast_type_error(AST *ast, const CVariantType *type, CError **error);
static CVariant *maybe_get_value(AST *ast, const CVariantType *type, CError **error);
static CVariant *tuple_get_value(AST *ast, const CVariantType *type, CError **error);
static CVariant *number_overflow(AST *ast, const CVariantType *type, CError **error);
static CVariant *string_get_value(AST *ast, const CVariantType *type, CError **error);
static AST *parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error);
static CVariant *number_get_value(AST *ast, const CVariantType *type, CError **error);
static CVariant *variant_get_value(AST *ast, const CVariantType *type, CError **error);
static CVariant *boolean_get_value(AST *ast, const CVariantType *type, CError **error);
static CVariant *typedecl_get_value(AST *ast, const CVariantType *type, CError **error);
static bool token_stream_peek2(TokenStream *stream, cchar first_char, cchar second_char);
static CVariant *dictionary_get_value(AST *ast, const CVariantType *type, CError **error);
static CVariant *bytestring_get_value(AST *ast, const CVariantType *type, CError **error);
static CVariant *positional_get_value(AST *ast, const CVariantType *type, CError **error);
static bool valid_format_string(const cchar *format_string, bool single, CVariant *value);
static cchar c_variant_scan_convenience(const cchar **str, bool *constant, cuint *arrays);
static AST *array_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error);
static AST *tuple_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error);
static AST *maybe_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error);
static AST *variant_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error);
static void c_variant_valist_get(const cchar **str, CVariant *value, bool free, va_list *app);
static AST *typedecl_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error);
static AST *dictionary_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error);
static void c_variant_valist_get_leaf(const cchar **str, CVariant *value, bool free, va_list *app);
static void _c_variant_builder_init(CVariantBuilder *builder, const CVariantType *type, bool type_owned);
static void ast_set_error(AST *ast, CError **error, AST *other_ast, cint code, const cchar *format, ...);
static bool token_stream_require(TokenStream *stream, const cchar *token, const cchar *purpose, CError **error);
static void parser_set_error(CError **error, SourceRef *location, SourceRef *other, cint code, const cchar *format,
                             ...);
static void token_stream_set_error(TokenStream *stream, CError **error, bool this_token, cint code, const cchar *format,
                                   ...);
static void parser_set_error_va(CError **error, SourceRef *location, SourceRef *other, cint code, const cchar *format,
                                va_list ap);
static bool unicode_unescape(const cchar *src, cint *src_ofs, cchar *dest, cint *dest_ofs, csize length, SourceRef *ref,
                             CError **error);
static void add_lines_from_range(CString *err, const cchar *str, const cchar *start1, const cchar *end1,
                                 const cchar *start2, const cchar *end2);


/* == new/ref/unref == */
static CRecMutex c_variant_type_info_lock;
static CHashTable *c_variant_type_info_table;
static CPtrArray *c_variant_type_info_gc;

#define CC_THRESHOLD 32
static void cc_while_locked(void);

static CVariant *c_variant_new_from_trusted(const CVariantType *type, const void *data, csize size);

static CVariantType *c_variant_make_dict_entry_type(CVariant *key, CVariant *val);
static CVariantType *c_variant_make_tuple_type(CVariant *const *children, csize n_children);

static cuint _c_variant_type_hash(void const *type);
static bool c_variant_type_check(const CVariantType *type);
static bool _c_variant_type_equal(const CVariantType *type1, const CVariantType *type2);
static void c_variant_type_info_check(const CVariantTypeInfo *info, char container_class);
static CVariantType *c_variant_type_new_tuple_slow(const CVariantType *const *items, cint length);
static bool variant_type_string_scan_internal(const cchar *string, const cchar *limit, const cchar **endPtr,
                                              csize *depth, csize depth_limit);

static cuint cvs_get_offset_size(csize size);
static bool cvs_tuple_is_normal(CVariantSerialised value);
static csize cvs_tuple_n_children(CVariantSerialised value);
static csize cvs_calculate_total_size(csize body_size, csize offsets);
static bool cvs_fixed_sized_array_is_normal(CVariantSerialised value);
static bool cvs_fixed_sized_maybe_is_normal(CVariantSerialised value);
static csize cvs_fixed_sized_maybe_n_children(CVariantSerialised value);
static csize cvs_fixed_sized_array_n_children(CVariantSerialised value);
static csize cvs_offsets_get_offset_n(struct Offsets *offsets, csize n);
static bool cvs_variable_sized_array_is_normal(CVariantSerialised value);
static bool cvs_variable_sized_maybe_is_normal(CVariantSerialised value);
static csize cvs_variable_sized_array_n_children(CVariantSerialised value);
static csize cvs_variable_sized_maybe_n_children(CVariantSerialised value);
static CVariantSerialised cvs_tuple_get_child(CVariantSerialised value, csize index_);
static struct Offsets cvs_variable_sized_array_get_frame_offsets(CVariantSerialised value);
static CVariantSerialised cvs_fixed_sized_maybe_get_child(CVariantSerialised value, csize index_);
static CVariantSerialised cvs_fixed_sized_array_get_child(CVariantSerialised value, csize index_);
static CVariantSerialised cvs_variable_sized_array_get_child(CVariantSerialised value, csize index_);
static CVariantSerialised cvs_variable_sized_maybe_get_child(CVariantSerialised value, csize index_);
static void cvs_tuple_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler, const void **children,
                                csize n_children);
static csize cvs_tuple_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                   const void **children, csize n_children);
static void cvs_tuple_get_member_bounds(CVariantSerialised value, csize index_, csize offset_size,
                                        csize *out_member_start, csize *out_member_end);
static void cvs_variable_sized_array_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler,
                                               const void **children, csize n_children);
static csize cvs_variable_sized_array_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                                  const void **children, csize n_children);
static void cvs_fixed_sized_array_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler,
                                            const void **children, csize n_children);
static void cvs_fixed_sized_maybe_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler,
                                            const void **children, csize n_children);
static void cvs_variable_sized_maybe_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler,
                                               const void **children, csize n_children);
static csize cvs_fixed_sized_array_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                               const void **children, csize n_children);
static csize cvs_fixed_sized_maybe_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                               const void **children, csize n_children);
static csize cvs_variable_sized_maybe_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                                  const void **children, csize n_children);

static void c_variant_lock(CVariant *value);
static void c_variant_unlock(CVariant *value);
static void c_variant_ensure_size(CVariant *value);
static void c_variant_release_children(CVariant *value);
static void c_variant_ensure_serialised(CVariant *value);
static void c_variant_fill_gvs(CVariantSerialised *, void *);
static void c_variant_serialise(CVariant *value, void *data);
static CVariant *c_variant_alloc(const CVariantType *type, bool serialised, bool trusted, csize suffix_size);


static const char c_variant_type_info_basic_chars[24][2] = {"b", " ", "d", " ", " ", "g", "h", "i", " ", " ", " ", " ",
                                                            "n", "o", " ", "q", " ", "s", "t", "u", "v", " ", "x", "y"};

static const CVariantTypeInfo c_variant_type_info_basic_table[24] = {
#define fixed_aligned(x) x, x - 1, 0
#define not_a_type 0, 0, 0
#define unaligned 0, 0, 0
#define aligned(x) 0, x - 1, 0
    /* 'b' */ {fixed_aligned(1)}, /* boolean */
    /* 'c' */ {not_a_type},
    /* 'd' */ {fixed_aligned(8)}, /* double */
    /* 'e' */ {not_a_type},
    /* 'f' */ {not_a_type},
    /* 'g' */ {unaligned}, /* signature string */
    /* 'h' */ {fixed_aligned(4)}, /* file handle (int32) */
    /* 'i' */ {fixed_aligned(4)}, /* int32 */
    /* 'j' */ {not_a_type},
    /* 'k' */ {not_a_type},
    /* 'l' */ {not_a_type},
    /* 'm' */ {not_a_type},
    /* 'n' */ {fixed_aligned(2)}, /* int16 */
    /* 'o' */ {unaligned}, /* object path string */
    /* 'p' */ {not_a_type},
    /* 'q' */ {fixed_aligned(2)}, /* uint16 */
    /* 'r' */ {not_a_type},
    /* 's' */ {unaligned}, /* string */
    /* 't' */ {fixed_aligned(8)}, /* uint64 */
    /* 'u' */ {fixed_aligned(4)}, /* uint32 */
    /* 'v' */ {aligned(8)}, /* variant */
    /* 'w' */ {not_a_type},
    /* 'x' */ {fixed_aligned(8)}, /* int64 */
    /* 'y' */ {fixed_aligned(1)}, /* byte */
#undef fixed_aligned
#undef not_a_type
#undef unaligned
#undef aligned
};

inline static CVariantSerialised c_variant_to_serialised(CVariant *value)
{
    c_assert(value->state & STATE_SERIALISED);
    {
        CVariantSerialised serialised = {
            value->type_info,
            (void *)value->contents.serialised.data,
            value->size,
            value->depth,
            value->contents.serialised.ordered_offsets_up_to,
            value->contents.serialised.checked_offsets_up_to,
        };
        return serialised;
    }
}

static inline csize cvs_read_unaligned_le(cuchar *bytes, cuint size)
{
    union
    {
        cuchar bytes[CLIB_SIZEOF_SIZE_T];
        csize integer;
    } tmpvalue;

    tmpvalue.integer = 0;
    if (bytes != NULL) {
        memcpy(&tmpvalue.bytes, bytes, size);
    }

    return C_SIZE_FROM_LE(tmpvalue.integer);
}

static inline void cvs_write_unaligned_le(cuchar *bytes, csize value, cuint size)
{
    union
    {
        cuchar bytes[CLIB_SIZEOF_SIZE_T];
        csize integer;
    } tmpvalue;

    tmpvalue.integer = C_SIZE_TO_LE(value);
    memcpy(bytes, &tmpvalue.bytes, size);
}


static inline csize cvs_variant_n_children(CVariantSerialised value) { return 1; }

static inline CVariantSerialised cvs_variant_get_child(CVariantSerialised value, csize index_)
{
    CVariantSerialised child = {
        0,
    };

    if (value.size) {
        /* find '\0' character */
        for (child.size = value.size - 1; child.size; child.size--)
            if (value.data[child.size] == '\0')
                break;

        /* ensure we didn't just hit the start of the string */
        if (value.data[child.size] == '\0') {
            const cchar *type_string = (cchar *)&value.data[child.size + 1];
            const cchar *limit = (cchar *)&value.data[value.size];
            const cchar *end;

            if (c_variant_type_string_scan(type_string, limit, &end) && end == limit) {
                const CVariantType *type = (CVariantType *)type_string;
                if (c_variant_type_is_definite(type)) {
                    csize fixed_size;
                    csize child_type_depth;
                    child.type_info = c_variant_type_info_get(type);
                    child.depth = value.depth + 1;
                    if (child.size != 0) {
                        child.data = value.data;
                    }
                    c_variant_type_info_query(child.type_info, NULL, &fixed_size);
                    child_type_depth = c_variant_type_info_query_depth(child.type_info);

                    if ((!fixed_size || fixed_size == child.size) &&
                        value.depth < C_VARIANT_MAX_RECURSION_DEPTH - child_type_depth) {
                        return child;
                    }
                    c_variant_type_info_unref(child.type_info);
                }
            }
        }
    }

    child.type_info = c_variant_type_info_get(C_VARIANT_TYPE_UNIT);
    child.data = NULL;
    child.size = 1;
    child.depth = value.depth + 1;

    return child;
}

static inline csize cvs_variant_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                            const void **children, csize n_children)
{
    CVariantSerialised child = {
        0,
    };
    const cchar *type_string;

    gvs_filler(&child, (void *)children[0]);
    type_string = c_variant_type_info_get_type_string(child.type_info);

    return child.size + 1 + strlen(type_string);
}

static inline void cvs_variant_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler,
                                         const void **children, csize n_children)
{
    CVariantSerialised child = {
        0,
    };
    const cchar *type_string;

    child.data = value.data;

    gvs_filler(&child, (void *)children[0]);
    type_string = c_variant_type_info_get_type_string(child.type_info);
    value.data[child.size] = '\0';
    memcpy(value.data + child.size + 1, type_string, strlen(type_string));
}

static inline bool cvs_variant_is_normal(CVariantSerialised value)
{
    CVariantSerialised child;
    bool normal;
    csize child_type_depth;

    child = cvs_variant_get_child(value, 0);
    child_type_depth = c_variant_type_info_query_depth(child.type_info);

    normal = (value.depth < C_VARIANT_MAX_RECURSION_DEPTH - child_type_depth) &&
        (child.data != NULL || child.size == 0) && c_variant_serialised_is_normal(child);

    c_variant_type_info_unref(child.type_info);

    return normal;
}



bool c_variant_type_string_is_valid(const cchar *typeStr)
{
    const cchar *endPtr;

    c_return_val_if_fail(typeStr != NULL, false);

    if (!c_variant_type_string_scan(typeStr, NULL, &endPtr)) {
        return false;
    }

    return *endPtr == '\0';
}

bool c_variant_type_string_scan(const cchar *str, const cchar *limit, const cchar **endPtr)
{
    return variant_type_string_scan_internal(str, limit, endPtr, NULL, C_VARIANT_MAX_RECURSION_DEPTH);
}

void c_variant_type_free(CVariantType *type)
{
    c_return_if_fail(type == NULL || c_variant_type_check(type));

    c_free(type);
}

CVariantType *c_variant_type_copy(const CVariantType *type)
{
    csize length = 0;
    cchar *new = NULL;

    c_return_val_if_fail(c_variant_type_check(type), NULL);

    length = c_variant_type_get_string_length(type);
    new = c_malloc0(length + 1);

    memcpy(new, type, length);
    new[length] = '\0';

    return (CVariantType *)new;
}

CVariantType *c_variant_type_new(const cchar *typeStr)
{
    c_return_val_if_fail(typeStr != NULL, NULL);

    return c_variant_type_copy(C_VARIANT_TYPE(typeStr));
}

csize c_variant_type_get_string_length(const CVariantType *type)
{
    const cchar *type_string = (const cchar *)type;
    cint brackets = 0;
    csize index = 0;

    c_return_val_if_fail(c_variant_type_check(type), 0);

    do {
        while (type_string[index] == 'a' || type_string[index] == 'm') {
            index++;
        }
        if (type_string[index] == '(' || type_string[index] == '{') {
            brackets++;
        }
        else if (type_string[index] == ')' || type_string[index] == '}') {
            brackets--;
        }
        index++;
    }
    while (brackets);

    return index;
}

const cchar *c_variant_type_peek_string(const CVariantType *type)
{
    c_return_val_if_fail(c_variant_type_check(type), NULL);

    return (const cchar *)type;
}

cchar *c_variant_type_dup_string(const CVariantType *type)
{
    c_return_val_if_fail(c_variant_type_check(type), NULL);

    return c_strndup(c_variant_type_peek_string(type), c_variant_type_get_string_length(type));
}

bool c_variant_type_is_definite(const CVariantType *type)
{
    const cchar *type_string;
    csize type_length;
    csize i;

    c_return_val_if_fail(c_variant_type_check(type), false);

    type_length = c_variant_type_get_string_length(type);
    type_string = c_variant_type_peek_string(type);

    for (i = 0; i < type_length; i++) {
        if (type_string[i] == '*' || type_string[i] == '?' || type_string[i] == 'r') {
            return false;
        }
    }

    return true;
}

bool c_variant_type_is_container(const CVariantType *type)
{
    cchar first_char;

    c_return_val_if_fail(c_variant_type_check(type), false);

    first_char = c_variant_type_peek_string(type)[0];
    switch (first_char) {
    case 'a':
    case 'm':
    case 'r':
    case '(':
    case '{':
    case 'v':
        return true;
    default:
        return false;
    }
}

bool c_variant_type_is_basic(const CVariantType *type)
{
    cchar first_char;

    c_return_val_if_fail(c_variant_type_check(type), false);

    first_char = c_variant_type_peek_string(type)[0];
    switch (first_char) {
    case 'b':
    case 'y':
    case 'n':
    case 'q':
    case 'i':
    case 'h':
    case 'u':
    case 't':
    case 'x':
    case 'd':
    case 's':
    case 'o':
    case 'g':
    case '?':
        return true;
    default:
        return false;
    }
}

bool c_variant_type_is_maybe(const CVariantType *type)
{
    c_return_val_if_fail(c_variant_type_check(type), false);

    return c_variant_type_peek_string(type)[0] == 'm';
}

bool c_variant_type_is_array(const CVariantType *type)
{
    c_return_val_if_fail(c_variant_type_check(type), false);

    return c_variant_type_peek_string(type)[0] == 'a';
}

bool c_variant_type_is_tuple(const CVariantType *type)
{
    c_return_val_if_fail(c_variant_type_check(type), false);

    const cchar typeChar = c_variant_type_peek_string(type)[0];

    return typeChar == 'r' || typeChar == '(';
}

bool c_variant_type_is_dict_entry(const CVariantType *type)
{
    c_return_val_if_fail(c_variant_type_check(type), false);

    return c_variant_type_peek_string(type)[0] == '{';
}

bool c_variant_type_is_variant(const CVariantType *type)
{
    c_return_val_if_fail(c_variant_type_check(type), false);

    return c_variant_type_peek_string(type)[0] == 'v';
}

cuint c_variant_type_hash(void const *type)
{
    c_return_val_if_fail(c_variant_type_check(type), 0);

    return _c_variant_type_hash(type);
}

bool c_variant_type_equal(void const *type1, void const *type2)
{
    c_return_val_if_fail(c_variant_type_check(type1), false);
    c_return_val_if_fail(c_variant_type_check(type2), false);

    return _c_variant_type_equal(type1, type2);
}

bool c_variant_type_is_subtype_of(const CVariantType *type, const CVariantType *supertype)
{
    const cchar *supertype_string;
    const cchar *supertype_end;
    const cchar *type_string;

    c_return_val_if_fail(c_variant_type_check(type), false);
    c_return_val_if_fail(c_variant_type_check(supertype), false);

    supertype_string = c_variant_type_peek_string(supertype);
    type_string = c_variant_type_peek_string(type);

    /* fast path for the basic determinate types */
    if (type_string[0] == supertype_string[0]) {
        switch (type_string[0]) {
        case 'b':
        case 'y':
        case 'n':
        case 'q':
        case 'i':
        case 'h':
        case 'u':
        case 't':
        case 'x':
        case 's':
        case 'o':
        case 'g':
        case 'd':
            return true;
        default:
            break;
        }
    }

    supertype_end = supertype_string + c_variant_type_get_string_length(supertype);

    while (supertype_string < supertype_end) {
        char supertype_char = *supertype_string++;
        if (supertype_char == *type_string) {
            type_string++;
        }
        else if (*type_string == ')') {
            return false;
        }
        else {
            const CVariantType *target_type = (CVariantType *)type_string;
            switch (supertype_char) {
            case 'r':
                if (!c_variant_type_is_tuple(target_type)) {
                    return false;
                }
                break;
            case '*':
                break;
            case '?':
                if (!c_variant_type_is_basic(target_type)) {
                    return false;
                }
                break;
            default:
                return false;
            }
            type_string += c_variant_type_get_string_length(target_type);
        }
    }

    return true;
}

const CVariantType *c_variant_type_element(const CVariantType *type)
{
    const cchar *type_string;

    c_return_val_if_fail(c_variant_type_check(type), NULL);

    type_string = c_variant_type_peek_string(type);

    c_assert(type_string[0] == 'a' || type_string[0] == 'm');

    return (const CVariantType *)&type_string[1];
}

const CVariantType *c_variant_type_first(const CVariantType *type)
{
    const cchar *type_string;

    c_return_val_if_fail(c_variant_type_check(type), NULL);

    type_string = c_variant_type_peek_string(type);
    c_assert(type_string[0] == '(' || type_string[0] == '{');

    if (type_string[1] == ')') {
        return NULL;
    }

    return (const CVariantType *)&type_string[1];
}

const CVariantType *c_variant_type_next(const CVariantType *type)
{
    const cchar *type_string;

    c_return_val_if_fail(c_variant_type_check(type), NULL);

    type_string = c_variant_type_peek_string(type);
    type_string += c_variant_type_get_string_length(type);

    if (*type_string == ')' || *type_string == '}') {
        return NULL;
    }

    return (const CVariantType *)type_string;
}

csize c_variant_type_n_items(const CVariantType *type)
{
    csize count = 0;

    c_return_val_if_fail(c_variant_type_check(type), 0);

    for (type = c_variant_type_first(type); type; type = c_variant_type_next(type)) {
        count++;
    }

    return count;
}

const CVariantType *c_variant_type_key(const CVariantType *type)
{
    const cchar *type_string;

    c_return_val_if_fail(c_variant_type_check(type), NULL);

    type_string = c_variant_type_peek_string(type);
    c_assert(type_string[0] == '{');

    return (const CVariantType *)&type_string[1];
}

const CVariantType *c_variant_type_value(const CVariantType *type)
{
#ifndef G_DISABLE_ASSERT
    const cchar *type_string;
#endif

    c_return_val_if_fail(c_variant_type_check(type), NULL);

#ifndef G_DISABLE_ASSERT
    type_string = c_variant_type_peek_string(type);
    c_assert(type_string[0] == '{');
#endif

    return c_variant_type_next(c_variant_type_key(type));
}

CVariantType *c_variant_type_new_array(const CVariantType *element)
{
    csize size;
    cchar *new;

    c_return_val_if_fail(c_variant_type_check(element), NULL);

    size = c_variant_type_get_string_length(element);
    new = c_malloc0(size + 1);

    new[0] = 'a';
    memcpy(new + 1, element, size);

    return (CVariantType *)new;
}

CVariantType *c_variant_type_new_maybe(const CVariantType *element)
{
    csize size;
    cchar *new;

    c_return_val_if_fail(c_variant_type_check(element), NULL);

    size = c_variant_type_get_string_length(element);
    new = c_malloc0(size + 1);

    new[0] = 'm';
    memcpy(new + 1, element, size);

    return (CVariantType *)new;
}

CVariantType *c_variant_type_new_tuple(const CVariantType *const *items, cint length)
{
    cchar buffer[1024];
    csize i;
    csize offset;
    csize length_unsigned;

    c_return_val_if_fail(length == 0 || items != NULL, NULL);

    if (length < 0) {
        for (length_unsigned = 0; items[length_unsigned] != NULL; length_unsigned++)
            ;
    }
    else {
        length_unsigned = (csize)length;
    }

    offset = 0;
    buffer[offset++] = '(';

    for (i = 0; i < length_unsigned; i++) {
        const CVariantType *type;
        csize size;
        c_return_val_if_fail(c_variant_type_check(items[i]), NULL);

        type = items[i];
        size = c_variant_type_get_string_length(type);

        if (offset + size >= sizeof buffer) {
            return c_variant_type_new_tuple_slow(items, length_unsigned);
        }
        memcpy(&buffer[offset], type, size);
        offset += size;
    }

    c_assert(offset < sizeof buffer);
    buffer[offset++] = ')';

    return (CVariantType *)c_strndup(buffer, offset);
}

CVariantType *c_variant_type_new_dict_entry(const CVariantType *key, const CVariantType *value)
{
    csize keysize, valsize;
    cchar *new;

    c_return_val_if_fail(c_variant_type_check(key), NULL);
    c_return_val_if_fail(c_variant_type_check(value), NULL);

    keysize = c_variant_type_get_string_length(key);
    valsize = c_variant_type_get_string_length(value);

    new = c_malloc0(1 + keysize + valsize + 1);

    new[0] = '{';
    memcpy(new + 1, key, keysize);
    memcpy(new + 1 + keysize, value, valsize);
    new[1 + keysize + valsize] = '}';

    return (CVariantType *)new;
}

const CVariantType *c_variant_type_checked_(const cchar *typeStr)
{
    c_return_val_if_fail(c_variant_type_string_is_valid(typeStr), NULL);
    return (const CVariantType *)typeStr;
}

csize c_variant_type_string_get_depth_(const cchar *typeStr)
{
    const cchar *endptr;
    csize depth = 0;

    c_return_val_if_fail(typeStr != NULL, 0);

    if (!variant_type_string_scan_internal(typeStr, NULL, &endptr, &depth, C_VARIANT_MAX_RECURSION_DEPTH) ||
        *endptr != '\0') {
        return 0;
    }

    return depth;
}

const cchar *c_variant_type_info_get_type_string(CVariantTypeInfo *info)
{
    c_variant_type_info_check(info, 0);

    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *)info;

        /* containers have their type string stored inside them */
        return container->type_string;
    }
    else {
        cint index;

        /* look up the type string in the base type array.  the call to
         * g_variant_type_info_check() above already ensured validity.
         */
        index = info - c_variant_type_info_basic_table;

        return c_variant_type_info_basic_chars[index];
    }
}

void c_variant_type_info_query(CVariantTypeInfo *info, cuint *alignment, csize *size)
{
    if (alignment) {
        *alignment = info->alignment;
    }

    if (size) {
        *size = info->fixed_size;
    }
}

csize c_variant_type_info_query_depth(CVariantTypeInfo *info)
{
    c_variant_type_info_check(info, 0);

    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *)info;
        return c_variant_type_string_get_depth_(container->type_string);
    }

    return 1;
}

CVariantTypeInfo *c_variant_type_info_element(CVariantTypeInfo *info) { return CV_ARRAY_INFO(info)->element; }

void c_variant_type_info_query_element(CVariantTypeInfo *info, cuint *alignment, csize *size)
{
    c_variant_type_info_query(CV_ARRAY_INFO(info)->element, alignment, size);
}

csize c_variant_type_info_n_members(CVariantTypeInfo *info) { return CV_TUPLE_INFO(info)->n_members; }

const CVariantMemberInfo *c_variant_type_info_member_info(CVariantTypeInfo *info, csize index)
{
    TupleInfo *tuple_info = CV_TUPLE_INFO(info);

    if (index < tuple_info->n_members) {
        return &tuple_info->members[index];
    }

    return NULL;
}

CVariantTypeInfo *c_variant_type_info_get(const CVariantType *type)
{
    const cchar *type_string = c_variant_type_peek_string(type);
    const char type_char = type_string[0];

    if (type_char == C_VARIANT_TYPE_INFO_CHAR_MAYBE || type_char == C_VARIANT_TYPE_INFO_CHAR_ARRAY ||
        type_char == C_VARIANT_TYPE_INFO_CHAR_TUPLE || type_char == C_VARIANT_TYPE_INFO_CHAR_DICT_ENTRY) {
        CVariantTypeInfo *info;
        c_rec_mutex_lock(&c_variant_type_info_lock);
        if (c_variant_type_info_table == NULL) {
            c_variant_type_info_table =
                c_hash_table_new((CHashFunc)_c_variant_type_hash, (CEqualFunc)_c_variant_type_equal);
            c_ignore_leak(c_variant_type_info_table);
        }
        info = c_hash_table_lookup(c_variant_type_info_table, type_string);

        if (info == NULL) {
            ContainerInfo *container;
            if (type_char == C_VARIANT_TYPE_INFO_CHAR_MAYBE || type_char == C_VARIANT_TYPE_INFO_CHAR_ARRAY) {
                container = array_info_new(type);
            }
            else {
                container = tuple_info_new(type);
            }

            info = (CVariantTypeInfo *)container;
            container->type_string = c_variant_type_dup_string(type);
            c_atomic_ref_count_init(&container->ref_count);
            c_hash_table_replace(c_variant_type_info_table, container->type_string, info);
        }
        else {
            c_variant_type_info_ref(info);
        }
        c_rec_mutex_unlock(&c_variant_type_info_lock);
        c_variant_type_info_check(info, 0);
        return info;
    }
    else {
        const CVariantTypeInfo *info;
        int index;

        index = type_char - 'b';
        c_assert(C_N_ELEMENTS(c_variant_type_info_basic_table) == 24);
        c_assert_cmpint(0, <=, index);
        c_assert_cmpint(index, <, 24);

        info = c_variant_type_info_basic_table + index;
        c_variant_type_info_check(info, 0);

        return (CVariantTypeInfo *)info;
    }
}

CVariantTypeInfo *c_variant_type_info_ref(CVariantTypeInfo *info)
{
    c_variant_type_info_check(info, 0);

    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *)info;
        c_atomic_ref_count_inc(&container->ref_count);
    }

    return info;
}

void c_variant_type_info_unref(CVariantTypeInfo *info)
{
    c_variant_type_info_check(info, 0);
    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *)info;
        c_rec_mutex_lock(&c_variant_type_info_lock);
        if (c_atomic_ref_count_dec(&container->ref_count)) {
            if (c_variant_type_info_gc == NULL) {
                c_variant_type_info_gc = c_ptr_array_new();
                c_ignore_leak(c_variant_type_info_gc);
            }

            c_atomic_ref_count_init(&container->ref_count);
            c_ptr_array_add(c_variant_type_info_gc, info);

            if (c_variant_type_info_gc->len > CC_THRESHOLD) {
                cc_while_locked();
            }
        }
        c_rec_mutex_unlock(&c_variant_type_info_lock);
    }
}

void c_variant_type_info_assert_no_infos(void)
{
    C_UNUSED bool empty;

    c_rec_mutex_lock(&c_variant_type_info_lock);
    if (c_variant_type_info_table != NULL) {
        cc_while_locked();
    }
    empty = (c_variant_type_info_table == NULL || c_hash_table_size(c_variant_type_info_table) == 0);
    c_rec_mutex_unlock(&c_variant_type_info_lock);

    c_assert(empty);
}

CVariantTypeInfo *c_variant_get_type_info(CVariant *value) { return value->type_info; }

bool c_variant_is_trusted(CVariant *value) { return (value->state & STATE_TRUSTED) != 0; }

csize c_variant_get_depth(CVariant *value) { return value->depth; }

CVariant *c_variant_maybe_get_child_value(CVariant *value, csize index_)
{
    c_return_val_if_fail(value->depth < C_MAX_SIZE, NULL);

    if (~c_atomic_int_get(&value->state) & STATE_SERIALISED) {
        c_return_val_if_fail(index_ < c_variant_n_children(value), NULL);
        c_variant_lock(value);

        if (~value->state & STATE_SERIALISED) {
            CVariant *child;
            child = c_variant_ref(value->contents.tree.children[index_]);
            c_variant_unlock(value);

            return child;
        }
        c_variant_unlock(value);
    }

    {
        CVariantSerialised serialised = c_variant_to_serialised(value);
        CVariantSerialised s_child;

        s_child = c_variant_serialised_get_child(serialised, index_);

        if (!(value->state & STATE_TRUSTED) && s_child.data == NULL) {
            c_variant_type_info_unref(s_child.type_info);
            return NULL;
        }

        c_variant_type_info_unref(s_child.type_info);

        return c_variant_get_child_value(value, index_);
    }
}


NUMERIC_TYPE(BYTE, byte, cuint8)
NUMERIC_TYPE(INT16, int16, cint16)
NUMERIC_TYPE(UINT16, uint16, cuint16)
NUMERIC_TYPE(INT32, int32, cint32)
NUMERIC_TYPE(UINT32, uint32, cuint32)
NUMERIC_TYPE(INT64, int64, cint64)
NUMERIC_TYPE(UINT64, uint64, cuint64)
NUMERIC_TYPE(HANDLE, handle, cint32)
NUMERIC_TYPE(DOUBLE, double, cdouble)


void c_variant_unref(CVariant *value)
{
    c_return_if_fail(value != NULL);

    if (c_atomic_ref_count_dec(&value->ref_count)) {
        if C_UNLIKELY (value->state & STATE_LOCKED) {
            C_LOG_CRIT("attempting to free a locked GVariant instance. This should never happen.");
        }

        value->state |= STATE_LOCKED;
        c_variant_type_info_unref(value->type_info);

        if (value->state & STATE_SERIALISED) {
            c_bytes_unref(value->contents.serialised.bytes);
        }
        else {
            c_variant_release_children(value);
        }

        memset(value, 0, sizeof(CVariant));
        c_free(value);
    }
}

CVariant *c_variant_ref(CVariant *value)
{
    c_return_val_if_fail(value != NULL, NULL);

    c_atomic_ref_count_inc(&value->ref_count);

    return value;
}

CVariant *c_variant_ref_sink(CVariant *value)
{
    int old_state;

    c_return_val_if_fail(value != NULL, NULL);
    c_return_val_if_fail(!c_atomic_ref_count_compare(&value->ref_count, 0), NULL);

    old_state = value->state;

    while (old_state & STATE_FLOATING) {
        int new_state = old_state & ~STATE_FLOATING;
        if (c_atomic_int_compare_and_exchange_full(&value->state, old_state, new_state, &old_state)) {
            return value;
        }
    }

    c_atomic_ref_count_inc(&value->ref_count);

    return value;
}

bool c_variant_is_floating(CVariant *value)
{
    c_return_val_if_fail(value != NULL, false);

    return (value->state & STATE_FLOATING) != 0;
}

CVariant *c_variant_take_ref(CVariant *value)
{
    c_return_val_if_fail(value != NULL, NULL);
    c_return_val_if_fail(!c_atomic_ref_count_compare(&value->ref_count, 0), NULL);

    c_atomic_int_and(&value->state, ~STATE_FLOATING);

    return value;
}

const CVariantType *c_variant_get_type(CVariant *value)
{
    CVariantTypeInfo *type_info;

    c_return_val_if_fail(value != NULL, NULL);

    type_info = c_variant_get_type_info(value);

    return (CVariantType *)c_variant_type_info_get_type_string(type_info);
}

const cchar *c_variant_get_type_string(CVariant *value)
{
    CVariantTypeInfo *type_info;

    c_return_val_if_fail(value != NULL, NULL);

    type_info = c_variant_get_type_info(value);

    return c_variant_type_info_get_type_string(type_info);
}

bool c_variant_is_of_type(CVariant *value, const CVariantType *type)
{
    return c_variant_type_is_subtype_of(c_variant_get_type(value), type);
}

bool c_variant_is_container(CVariant *value) { return c_variant_type_is_container(c_variant_get_type(value)); }

CVariantClass c_variant_classify(CVariant *value)
{
    c_return_val_if_fail(value != NULL, 0);

    return *c_variant_get_type_string(value);
}

CVariant *c_variant_new_boolean(bool value)
{
    cuchar v = value;

    return c_variant_new_from_trusted(C_VARIANT_TYPE_BOOLEAN, &v, 1);
}

CVariant *c_variant_new_string(const cchar *str)
{
    const char *endptr = NULL;

    c_return_val_if_fail(str != NULL, NULL);

    if C_LIKELY (c_utf8_validate(str, -1, &endptr)) {
        return c_variant_new_from_trusted(C_VARIANT_TYPE_STRING, str, endptr - str + 1);
    }

    C_LOG_CRIT("c_variant_new_string(): requires valid UTF-8");

    return NULL;
}

CVariant *c_variant_new_take_string(cchar *str)
{
    const char *end = NULL;

    c_return_val_if_fail(str != NULL, NULL);

    if C_LIKELY (c_utf8_validate(str, -1, &end)) {
        CBytes *bytes = c_bytes_new_take(str, end - str + 1);
        return c_variant_new_take_bytes(C_VARIANT_TYPE_STRING, c_steal_pointer(&bytes), true);
    }

    C_LOG_CRIT("c_variant_new_take_string(): requires valid UTF-8");

    return NULL;
}

CVariant *c_variant_new_printf(const cchar *format_string, ...)
{
    CVariant *value;
    CBytes *bytes;
    cchar *string;
    va_list ap;

    c_return_val_if_fail(format_string != NULL, NULL);

    va_start(ap, format_string);
    string = c_strdup_vprintf(format_string, ap);
    va_end(ap);

    bytes = c_bytes_new_take(string, strlen(string) + 1);
    value = c_variant_new_take_bytes(C_VARIANT_TYPE_STRING, c_steal_pointer(&bytes), true);

    return value;
}

CVariant *c_variant_new_object_path(const cchar *object_path)
{
    c_return_val_if_fail(c_variant_is_object_path(object_path), NULL);

    return c_variant_new_from_trusted(C_VARIANT_TYPE_OBJECT_PATH, object_path, strlen(object_path) + 1);
}

bool c_variant_is_object_path(const cchar *str)
{
    c_return_val_if_fail(str != NULL, false);

    return c_variant_serialiser_is_object_path(str, strlen(str) + 1);
}

CVariant *c_variant_new_signature(const cchar *signature)
{
    c_return_val_if_fail(c_variant_is_signature(signature), NULL);

    return c_variant_new_from_trusted(C_VARIANT_TYPE_SIGNATURE, signature, strlen(signature) + 1);
}

bool c_variant_is_signature(const cchar *str)
{
    c_return_val_if_fail(str != NULL, false);

    return c_variant_serialiser_is_signature(str, strlen(str) + 1);
}

CVariant *c_variant_new_variant(CVariant *value)
{
    c_return_val_if_fail(value != NULL, NULL);

    c_variant_ref_sink(value);

    return c_variant_new_from_children(C_VARIANT_TYPE_VARIANT, c_memdup(&value, sizeof value), 1,
                                       c_variant_is_trusted(value));
}

CVariant *c_variant_new_strv(const cchar *const *strv, cssize len)
{
    CVariant **strings;
    csize i, length_unsigned;

    c_return_val_if_fail(len == 0 || strv != NULL, NULL);

    if (len < 0)
        len = c_strv_length((cchar **)strv);
    length_unsigned = len;

    strings = c_malloc0(sizeof(CVariant *) * length_unsigned);
    for (i = 0; i < length_unsigned; i++) {
        strings[i] = c_variant_ref_sink(c_variant_new_string(strv[i]));
    }

    return c_variant_new_from_children(C_VARIANT_TYPE_STRING_ARRAY, strings, length_unsigned, true);
}

CVariant *c_variant_new_objv(const cchar *const *strv, cssize len)
{
    CVariant **strings;
    csize i, length_unsigned;

    c_return_val_if_fail(len == 0 || strv != NULL, NULL);

    if (len < 0)
        len = c_strv_length((cchar **)strv);
    length_unsigned = len;

    strings = c_malloc0(sizeof(CVariant *) * length_unsigned);
    for (i = 0; i < length_unsigned; i++) {
        strings[i] = c_variant_ref_sink(c_variant_new_object_path(strv[i]));
    }

    return c_variant_new_from_children(C_VARIANT_TYPE_OBJECT_PATH_ARRAY, strings, length_unsigned, true);
}

CVariant *c_variant_new_bytestring(const cchar *str)
{
    c_return_val_if_fail(str != NULL, NULL);

    return c_variant_new_from_trusted(C_VARIANT_TYPE_BYTESTRING, str, strlen(str) + 1);
}

CVariant *c_variant_new_bytestring_array(const cchar *const *strv, cssize len)
{
    CVariant **strings;
    csize i, length_unsigned;

    c_return_val_if_fail(len == 0 || strv != NULL, NULL);

    if (len < 0)
        len = c_strv_length((cchar **)strv);
    length_unsigned = len;

    strings = c_malloc0(sizeof(CVariant *) * length_unsigned);
    for (i = 0; i < length_unsigned; i++)
        strings[i] = c_variant_ref_sink(c_variant_new_bytestring(strv[i]));

    return c_variant_new_from_children(C_VARIANT_TYPE_BYTESTRING_ARRAY, strings, length_unsigned, true);
}

CVariant *c_variant_new_fixed_array(const CVariantType *element_type, const void *elements, csize n_elements,
                                    csize element_size)
{
    CVariantType *array_type;
    csize array_element_size;
    CVariantTypeInfo *array_info;
    CVariant *value;
    void *data;

    c_return_val_if_fail(c_variant_type_is_definite(element_type), NULL);
    c_return_val_if_fail(element_size > 0, NULL);

    array_type = c_variant_type_new_array(element_type);
    array_info = c_variant_type_info_get(array_type);
    c_variant_type_info_query_element(array_info, NULL, &array_element_size);
    if C_UNLIKELY (array_element_size != element_size) {
        if (array_element_size)
            C_LOG_CRIT("g_variant_new_fixed_array: array size %lld"
                       " does not match given element_size %lld.",
                       array_element_size, element_size);
        else
            C_LOG_CRIT("g_variant_get_fixed_array: array does not have fixed size.");
        return NULL;
    }

    if (elements) {
        culong size = sizeof(void *) * n_elements * element_size;
        data = c_malloc0(size);
        memcpy(data, elements, size);
    }
    else {
        data = NULL;
    }
    value = c_variant_new_from_data(array_type, data, n_elements * element_size, false, c_free0, data);

    c_variant_type_free(array_type);
    c_variant_type_info_unref(array_info);

    return value;
}

bool c_variant_get_boolean(CVariant *value)
{
    const cuchar *data;

    TYPE_CHECK(value, C_VARIANT_TYPE_BOOLEAN, false);

    data = c_variant_get_data(value);

    return data != NULL ? *data != 0 : false;
}

CVariant *c_variant_get_variant(CVariant *value)
{
    TYPE_CHECK(value, C_VARIANT_TYPE_VARIANT, NULL);

    return c_variant_get_child_value(value, 0);
}

const cchar *c_variant_get_string(CVariant *value, csize *length)
{
    const void *data;
    csize size;

    c_return_val_if_fail(value != NULL, NULL);
    c_return_val_if_fail(c_variant_is_of_type(value, C_VARIANT_TYPE_STRING) ||
                             c_variant_is_of_type(value, C_VARIANT_TYPE_OBJECT_PATH) ||
                             c_variant_is_of_type(value, C_VARIANT_TYPE_SIGNATURE),
                         NULL);

    data = c_variant_get_data(value);
    size = c_variant_get_size(value);

    if (!c_variant_is_trusted(value)) {
        switch (c_variant_classify(value)) {
        case C_VARIANT_CLASS_STRING:
            if (c_variant_serialiser_is_string(data, size)) {
                break;
            }
            data = "";
            size = 1;
            break;
        case C_VARIANT_CLASS_OBJECT_PATH:
            if (c_variant_serialiser_is_object_path(data, size)) {
                break;
            }

            data = "/";
            size = 2;
            break;

        case C_VARIANT_CLASS_SIGNATURE:
            if (c_variant_serialiser_is_signature(data, size)) {
                break;
            }
            data = "";
            size = 1;
            break;

        default:
            c_assert_not_reached();
        }
    }

    if (length)
        *length = size - 1;

    return data;
}

cchar *c_variant_dup_string(CVariant *value, csize *length) { return c_strdup(c_variant_get_string(value, length)); }

const cchar **c_variant_get_strv(CVariant *value, csize *length)
{
    const cchar **strv;
    csize n;
    csize i;

    TYPE_CHECK(value, C_VARIANT_TYPE_STRING_ARRAY, NULL);

    c_variant_get_data(value);
    n = c_variant_n_children(value);
    strv = c_malloc0(sizeof(const cchar *) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *str = c_variant_get_child_value(value, i);
        strv[i] = c_variant_get_string(str, NULL);
        c_variant_unref(str);
    }
    strv[i] = NULL;

    if (length) {
        *length = n;
    }

    return strv;
}

cchar **c_variant_dup_strv(CVariant *value, csize *length)
{
    cchar **strv;
    csize n;
    csize i;

    TYPE_CHECK(value, C_VARIANT_TYPE_STRING_ARRAY, NULL);

    n = c_variant_n_children(value);
    strv = c_malloc0(sizeof(cchar *) * (n + 1));
    for (i = 0; i < n; i++) {
        CVariant *str = c_variant_get_child_value(value, i);
        strv[i] = c_variant_dup_string(str, NULL);
        c_variant_unref(str);
    }
    strv[i] = NULL;

    if (length)
        *length = n;

    return strv;
}

const cchar **c_variant_get_objv(CVariant *value, csize *length)
{
    const cchar **strv;
    csize n;
    csize i;

    TYPE_CHECK(value, C_VARIANT_TYPE_OBJECT_PATH_ARRAY, NULL);

    c_variant_get_data(value);
    n = c_variant_n_children(value);
    strv = c_malloc0(sizeof(const cchar *) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *str = c_variant_get_child_value(value, i);
        strv[i] = c_variant_get_string(str, NULL);
        c_variant_unref(str);
    }
    strv[i] = NULL;

    if (length)
        *length = n;

    return strv;
}

cchar **c_variant_dup_objv(CVariant *value, csize *length)
{
    cchar **strv;
    csize n;
    csize i;

    TYPE_CHECK(value, C_VARIANT_TYPE_OBJECT_PATH_ARRAY, NULL);

    n = c_variant_n_children(value);
    strv = c_malloc0(sizeof(cchar *) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *str = c_variant_get_child_value(value, i);
        strv[i] = c_variant_dup_string(str, NULL);
        c_variant_unref(str);
    }
    strv[i] = NULL;

    if (length)
        *length = n;

    return strv;
}

const cchar *c_variant_get_bytestring(CVariant *value)
{
    const cchar *str;
    csize size;

    TYPE_CHECK(value, C_VARIANT_TYPE_BYTESTRING, NULL);

    /* Won't be NULL since this is an array type */
    str = c_variant_get_data(value);
    size = c_variant_get_size(value);

    if (size && str[size - 1] == '\0') {
        return str;
    }

    return "";
}

cchar *c_variant_dup_bytestring(CVariant *value, csize *length)
{
    const cchar *original = c_variant_get_bytestring(value);
    csize size;

    /* don't crash in case get_bytestring() had an assert failure */
    if (original == NULL)
        return NULL;

    size = strlen(original);

    if (length)
        *length = size;

    return c_memdup(original, size + 1);
}

const cchar **c_variant_get_bytestring_array(CVariant *value, csize *length)
{
    const cchar **strv;
    csize n;
    csize i;

    TYPE_CHECK(value, C_VARIANT_TYPE_BYTESTRING_ARRAY, NULL);

    c_variant_get_data(value);
    n = c_variant_n_children(value);
    strv = c_malloc0(sizeof(const cchar *) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *string;
        string = c_variant_get_child_value(value, i);
        strv[i] = c_variant_get_bytestring(string);
        c_variant_unref(string);
    }
    strv[i] = NULL;

    if (length)
        *length = n;

    return strv;
}

cchar **c_variant_dup_bytestring_array(CVariant *value, csize *length)
{
    cchar **strv;
    csize n;
    csize i;

    TYPE_CHECK(value, C_VARIANT_TYPE_BYTESTRING_ARRAY, NULL);

    c_variant_get_data(value);
    n = c_variant_n_children(value);
    strv = c_malloc0(sizeof(cchar *) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *string;
        string = c_variant_get_child_value(value, i);
        strv[i] = c_variant_dup_bytestring(string, NULL);
        c_variant_unref(string);
    }
    strv[i] = NULL;

    if (length)
        *length = n;

    return strv;
}

CVariant *c_variant_new_maybe(const CVariantType *child_type, CVariant *child)
{
    CVariantType *maybe_type;
    CVariant *value;

    c_return_val_if_fail(child_type == NULL || c_variant_type_is_definite(child_type), 0);
    c_return_val_if_fail(child_type != NULL || child != NULL, NULL);
    c_return_val_if_fail(child_type == NULL || child == NULL || c_variant_is_of_type(child, child_type), NULL);

    if (child_type == NULL) {
        child_type = c_variant_get_type(child);
    }

    maybe_type = c_variant_type_new_maybe(child_type);

    if (child != NULL) {
        CVariant **children;
        bool trusted;

        children = c_malloc0(sizeof(CVariant *));
        children[0] = c_variant_ref_sink(child);
        trusted = c_variant_is_trusted(children[0]);
        value = c_variant_new_from_children(maybe_type, children, 1, trusted);
    }
    else {
        value = c_variant_new_from_children(maybe_type, NULL, 0, true);
    }

    c_variant_type_free(maybe_type);

    return value;
}

CVariant *c_variant_new_array(const CVariantType *child_type, CVariant *const *children, csize n_children)
{
    CVariantType *array_type;
    CVariant **my_children;
    bool trusted;
    CVariant *value;
    csize i;

    c_return_val_if_fail(n_children > 0 || child_type != NULL, NULL);
    c_return_val_if_fail(n_children == 0 || children != NULL, NULL);
    c_return_val_if_fail(child_type == NULL || c_variant_type_is_definite(child_type), NULL);

    my_children = c_malloc0(sizeof(CVariant *) * n_children);
    trusted = true;

    if (child_type == NULL) {
        child_type = c_variant_get_type(children[0]);
    }
    array_type = c_variant_type_new_array(child_type);

    for (i = 0; i < n_children; i++) {
        bool is_of_child_type = c_variant_is_of_type(children[i], child_type);
        if C_UNLIKELY (!is_of_child_type) {
            while (i != 0) {
                c_variant_unref(my_children[--i]);
            }
            c_free(my_children);
            c_return_val_if_fail(is_of_child_type, NULL);
        }
        my_children[i] = c_variant_ref_sink(children[i]);
        trusted &= c_variant_is_trusted(children[i]);
    }

    value = c_variant_new_from_children(array_type, my_children, n_children, trusted);
    c_variant_type_free(array_type);

    return value;
}

CVariant *c_variant_new_tuple(CVariant *const *children, csize n_children)
{
    CVariantType *tuple_type;
    CVariant **my_children;
    bool trusted;
    CVariant *value;
    csize i;

    c_return_val_if_fail(n_children == 0 || children != NULL, NULL);

    my_children = c_malloc0(sizeof(CVariant *) * n_children);
    trusted = true;

    for (i = 0; i < n_children; i++) {
        my_children[i] = c_variant_ref_sink(children[i]);
        trusted &= c_variant_is_trusted(children[i]);
    }

    tuple_type = c_variant_make_tuple_type(children, n_children);
    value = c_variant_new_from_children(tuple_type, my_children, n_children, trusted);
    c_variant_type_free(tuple_type);

    return value;
}

CVariant *c_variant_new_dict_entry(CVariant *key, CVariant *value)
{
    CVariantType *dict_type;
    CVariant **children;
    bool trusted;

    c_return_val_if_fail(key != NULL && value != NULL, NULL);
    c_return_val_if_fail(!c_variant_is_container(key), NULL);

    children = c_malloc0(sizeof(CVariant *) * 2);
    children[0] = c_variant_ref_sink(key);
    children[1] = c_variant_ref_sink(value);
    trusted = c_variant_is_trusted(key) && c_variant_is_trusted(value);

    dict_type = c_variant_make_dict_entry_type(key, value);
    value = c_variant_new_from_children(dict_type, children, 2, trusted);
    c_variant_type_free(dict_type);

    return value;
}

CVariant *c_variant_get_maybe(CVariant *value)
{
    TYPE_CHECK(value, C_VARIANT_TYPE_MAYBE, NULL);

    if (c_variant_n_children(value)) {
        return c_variant_get_child_value(value, 0);
    }

    return NULL;
}

csize c_variant_n_children(CVariant *value)
{
    csize n_children;

    c_variant_lock(value);

    if (value->state & STATE_SERIALISED)
        n_children = c_variant_serialised_n_children(c_variant_to_serialised(value));
    else
        n_children = value->contents.tree.n_children;

    c_variant_unlock(value);

    return n_children;
}

void c_variant_get_child(CVariant *value, csize idx, const cchar *formatStr, ...)
{
    CVariant *child;
    va_list ap;

    /* if any direct-pointer-access formats are in use, flatten first */
    if (strchr(formatStr, '&'))
        c_variant_get_data(value);

    child = c_variant_get_child_value(value, idx);
    c_return_if_fail(valid_format_string(formatStr, true, child));

    va_start(ap, formatStr);
    c_variant_get_va(child, formatStr, NULL, &ap);
    va_end(ap);

    c_variant_unref(child);
}

CVariant *c_variant_get_child_value(CVariant *value, csize index_)
{
    c_return_val_if_fail(value->depth < C_MAX_SIZE, NULL);

    if (~c_atomic_int_get(&value->state) & STATE_SERIALISED) {
        c_return_val_if_fail(index_ < c_variant_n_children(value), NULL);
        c_variant_lock(value);
        if (~value->state & STATE_SERIALISED) {
            CVariant *child;
            child = c_variant_ref(value->contents.tree.children[index_]);
            c_variant_unlock(value);
            return child;
        }
        c_variant_unlock(value);
    }

    {
        CVariantSerialised serialised = c_variant_to_serialised(value);
        CVariantSerialised s_child;
        CVariant *child;

        s_child = c_variant_serialised_get_child(serialised, index_);

        value->contents.serialised.ordered_offsets_up_to =
            C_MAX(value->contents.serialised.ordered_offsets_up_to, serialised.ordered_offsets_up_to);
        value->contents.serialised.checked_offsets_up_to =
            C_MAX(value->contents.serialised.checked_offsets_up_to, serialised.checked_offsets_up_to);

        if (!(value->state & STATE_TRUSTED) &&
            c_variant_type_info_query_depth(s_child.type_info) >= C_VARIANT_MAX_RECURSION_DEPTH - value->depth) {
            c_assert(c_variant_is_of_type(value, C_VARIANT_TYPE_VARIANT));
            c_variant_type_info_unref(s_child.type_info);
            return c_variant_new_tuple(NULL, 0);
        }

        /* create a new serialized instance out of it */
        child = c_malloc0(sizeof(CVariant));
        child->type_info = s_child.type_info;
        child->state = (value->state & STATE_TRUSTED) | STATE_SERIALISED;
        child->size = s_child.size;
        c_atomic_ref_count_init(&child->ref_count);
        child->depth = value->depth + 1;
        child->contents.serialised.bytes = c_bytes_ref(value->contents.serialised.bytes);
        child->contents.serialised.data = s_child.data;
        child->contents.serialised.ordered_offsets_up_to =
            (value->state & STATE_TRUSTED) ? C_MAX_SIZE : s_child.ordered_offsets_up_to;
        child->contents.serialised.checked_offsets_up_to =
            (value->state & STATE_TRUSTED) ? C_MAX_SIZE : s_child.checked_offsets_up_to;

        return child;
    }
}

bool c_variant_lookup(CVariant *dictionary, const cchar *key, const cchar *format_string, ...)
{
    CVariantType *type;
    CVariant *value;

    /* flatten */
    c_variant_get_data(dictionary);

    type = c_variant_format_string_scan_type(format_string, NULL, NULL);
    value = c_variant_lookup_value(dictionary, key, type);
    c_variant_type_free(type);

    if (value) {
        va_list ap;

        va_start(ap, format_string);
        c_variant_get_va(value, format_string, NULL, &ap);
        c_variant_unref(value);
        va_end(ap);

        return true;
    }

    return false;
}

CVariant *c_variant_lookup_value(CVariant *dictionary, const cchar *key, const CVariantType *expected_type)
{
    CVariantIter iter;
    CVariant *entry;
    CVariant *value;

    c_return_val_if_fail(c_variant_is_of_type(dictionary, C_VARIANT_TYPE("a{s*}")) ||
                             c_variant_is_of_type(dictionary, C_VARIANT_TYPE("a{o*}")),
                         NULL);
    c_variant_iter_init(&iter, dictionary);

    while ((entry = c_variant_iter_next_value(&iter))) {
        CVariant *entry_key;
        bool matches;

        entry_key = c_variant_get_child_value(entry, 0);
        matches = strcmp(c_variant_get_string(entry_key, NULL), key) == 0;
        c_variant_unref(entry_key);

        if (matches) {
            break;
        }
        c_variant_unref(entry);
    }

    if (entry == NULL) {
        return NULL;
    }

    value = c_variant_get_child_value(entry, 1);
    c_variant_unref(entry);

    if (c_variant_is_of_type(value, C_VARIANT_TYPE_VARIANT)) {
        CVariant *tmp;
        tmp = c_variant_get_variant(value);
        c_variant_unref(value);
        if (expected_type && !c_variant_is_of_type(tmp, expected_type)) {
            c_variant_unref(tmp);
            tmp = NULL;
        }
        value = tmp;
    }

    c_return_val_if_fail(expected_type == NULL || value == NULL || c_variant_is_of_type(value, expected_type), NULL);

    return value;
}

const void *c_variant_get_fixed_array(CVariant *value, csize *n_elements, csize element_size)
{
    CVariantTypeInfo *array_info;
    csize array_element_size;
    const void *data;
    csize size;

    TYPE_CHECK(value, C_VARIANT_TYPE_ARRAY, NULL);

    c_return_val_if_fail(n_elements != NULL, NULL);
    c_return_val_if_fail(element_size > 0, NULL);

    array_info = c_variant_get_type_info(value);
    c_variant_type_info_query_element(array_info, NULL, &array_element_size);

    c_return_val_if_fail(array_element_size, NULL);

    if C_UNLIKELY (array_element_size != element_size) {
        if (array_element_size)
            C_LOG_CRIT("g_variant_get_fixed_array: assertion "
                       "'g_variant_array_has_fixed_size (value, element_size)' "
                       "failed: array size %lld does not match "
                       "given element_size %lld.",
                       array_element_size, element_size);
        else
            C_LOG_CRIT("g_variant_get_fixed_array: assertion "
                       "'g_variant_array_has_fixed_size (value, element_size)' "
                       "failed: array does not have fixed size.");
    }

    data = c_variant_get_data(value);
    size = c_variant_get_size(value);

    if (size % element_size) {
        *n_elements = 0;
    }
    else {
        *n_elements = size / element_size;
    }

    if (*n_elements) {
        return data;
    }

    return NULL;
}

csize c_variant_get_size(CVariant *value)
{
    c_variant_lock(value);
    c_variant_ensure_size(value);
    c_variant_unlock(value);

    return value->size;
}

const void *c_variant_get_data(CVariant *value)
{
    c_variant_lock(value);
    c_variant_ensure_serialised(value);
    c_variant_unlock(value);

    return value->contents.serialised.data;
}

CBytes *c_variant_get_data_as_bytes(CVariant *value)
{
    const cchar *bytes_data;
    const cchar *data;
    csize bytes_size = 0;
    csize size;

    c_variant_lock(value);
    c_variant_ensure_serialised(value);
    c_variant_unlock(value);

    if (value->contents.serialised.bytes != NULL) {
        bytes_data = c_bytes_get_data(value->contents.serialised.bytes, &bytes_size);
    }
    else {
        bytes_data = NULL;
    }

    data = value->contents.serialised.data;
    size = value->size;

    if (data == NULL) {
        c_assert(size == 0);
        data = bytes_data;
    }

    if (bytes_data != NULL && data == bytes_data && size == bytes_size)
        return c_bytes_ref(value->contents.serialised.bytes);
    else if (bytes_data != NULL)
        return c_bytes_new_from_bytes(value->contents.serialised.bytes, data - bytes_data, size);
    else
        return c_bytes_new(value->contents.serialised.data, size);
}

void c_variant_store(CVariant *value, void *data)
{
    c_variant_lock(value);

    if (value->state & STATE_SERIALISED) {
        if (value->contents.serialised.data != NULL)
            memcpy(data, value->contents.serialised.data, value->size);
        else
            memset(data, 0, value->size);
    }
    else {
        c_variant_serialise(value, data);
    }

    c_variant_unlock(value);
}

cchar *c_variant_print(CVariant *value, bool type_annotate)
{
    return c_string_free(c_variant_print_string(value, NULL, type_annotate), false);
}

CString *c_variant_print_string(CVariant *value, CString *string, bool type_annotate)
{
    const cchar *value_type_string = c_variant_get_type_string(value);

    if C_UNLIKELY (string == NULL) {
        string = c_string_new(NULL);
    }

    switch (value_type_string[0]) {
    case C_VARIANT_CLASS_MAYBE:
        if (type_annotate) {
            c_string_append_printf(string, "@%s ", value_type_string);
        }
        if (c_variant_n_children(value)) {
            const CVariantType *base_type;
            cuint i, depth;
            CVariant *element = NULL;
            for (depth = 0, base_type = c_variant_get_type(value); c_variant_type_is_maybe(base_type);
                 depth++, base_type = c_variant_type_element(base_type))
                ;
            element = c_variant_ref(value);
            for (i = 0; i < depth && element != NULL; i++) {
                CVariant *new_element = c_variant_n_children(element) ? c_variant_get_child_value(element, 0) : NULL;
                c_variant_unref(element);
                element = c_steal_pointer(&new_element);
            }
            if (element == NULL) {
                for (; i > 1; i--) {
                    c_string_append(string, "just ");
                }
                c_string_append(string, "nothing");
            }
            else {
                c_variant_print_string(element, string, false);
            }
            c_clear_pointer((void **)&element, (CDestroyNotify)c_variant_unref);
        }
        else {
            c_string_append(string, "nothing");
        }
        break;
    case C_VARIANT_CLASS_ARRAY:
        if (value_type_string[1] == 'y') {
            const cchar *str;
            csize size;
            csize i;

            str = c_variant_get_data(value);
            size = c_variant_get_size(value);

            for (i = 0; i < size; i++) {
                if (str[i] == '\0') {
                    break;
                }
            }
            if (i == size - 1) {
                cchar *escaped = c_strescape(str, NULL);

                if (strchr(str, '\''))
                    c_string_append_printf(string, "b\"%s\"", escaped);
                else
                    c_string_append_printf(string, "b'%s'", escaped);

                c_free(escaped);
                break;
            }
            else {
                /* fall through and handle normally... */
            }
        }
        if (value_type_string[1] == '{') {
            /* dictionary */
            const cchar *comma = "";
            csize n, i;

            if ((n = c_variant_n_children(value)) == 0) {
                if (type_annotate) {
                    c_string_append_printf(string, "@%s ", value_type_string);
                }
                c_string_append(string, "{}");
                break;
            }

            c_string_append_c(string, '{');
            for (i = 0; i < n; i++) {
                CVariant *entry, *key, *val;
                c_string_append(string, comma);
                comma = ", ";
                entry = c_variant_get_child_value(value, i);
                key = c_variant_get_child_value(entry, 0);
                val = c_variant_get_child_value(entry, 1);
                c_variant_unref(entry);

                c_variant_print_string(key, string, type_annotate);
                c_variant_unref(key);
                c_string_append(string, ": ");
                c_variant_print_string(val, string, type_annotate);
                c_variant_unref(val);
                type_annotate = false;
            }
            c_string_append_c(string, '}');
        }
        else {
            /* normal (non-dictionary) array */
            const cchar *comma = "";
            csize n, i;
            if ((n = c_variant_n_children(value)) == 0) {
                if (type_annotate) {
                    c_string_append_printf(string, "@%s ", value_type_string);
                }
                c_string_append(string, "[]");
                break;
            }
            c_string_append_c(string, '[');
            for (i = 0; i < n; i++) {
                CVariant *element;
                c_string_append(string, comma);
                comma = ", ";
                element = c_variant_get_child_value(value, i);
                c_variant_print_string(element, string, type_annotate);
                c_variant_unref(element);
                type_annotate = false;
            }
            c_string_append_c(string, ']');
        }
        break;
    case C_VARIANT_CLASS_TUPLE: {
        csize n, i;
        n = c_variant_n_children(value);
        c_string_append_c(string, '(');
        for (i = 0; i < n; i++) {
            CVariant *element;
            element = c_variant_get_child_value(value, i);
            c_variant_print_string(element, string, type_annotate);
            c_string_append(string, ", ");
            c_variant_unref(element);
        }
        c_string_truncate(string, string->len - (n > 0) - (n > 1));
        c_string_append_c(string, ')');
    } break;
    case C_VARIANT_CLASS_DICT_ENTRY: {
        CVariant *element;
        c_string_append_c(string, '{');
        element = c_variant_get_child_value(value, 0);
        c_variant_print_string(element, string, type_annotate);
        c_variant_unref(element);
        c_string_append(string, ", ");
        element = c_variant_get_child_value(value, 1);
        c_variant_print_string(element, string, type_annotate);
        c_variant_unref(element);
        c_string_append_c(string, '}');
    } break;
    case C_VARIANT_CLASS_VARIANT: {
        CVariant *child = c_variant_get_variant(value);
        c_string_append_c(string, '<');
        c_variant_print_string(child, string, true);
        c_string_append_c(string, '>');
        c_variant_unref(child);
    } break;
    case C_VARIANT_CLASS_BOOLEAN:
        if (c_variant_get_boolean(value))
            c_string_append(string, "true");
        else
            c_string_append(string, "false");
        break;
    case C_VARIANT_CLASS_STRING: {
        const cchar *str = c_variant_get_string(value, NULL);
        cunichar quote = strchr(str, '\'') ? '"' : '\'';
        c_string_append_c(string, quote);
        while (*str) {
            cunichar c = c_utf8_get_char(str);
            if (c == quote || c == '\\') {
                c_string_append_c(string, '\\');
            }
            if (c_unichar_isprint(c)) {
                c_string_append_unichar(string, c);
            }
            else {
                c_string_append_c(string, '\\');
                if (c < 0x10000) {
                    switch (c) {
                    case '\a':
                        c_string_append_c(string, 'a');
                        break;
                    case '\b':
                        c_string_append_c(string, 'b');
                        break;
                    case '\f':
                        c_string_append_c(string, 'f');
                        break;
                    case '\n':
                        c_string_append_c(string, 'n');
                        break;
                    case '\r':
                        c_string_append_c(string, 'r');
                        break;
                    case '\t':
                        c_string_append_c(string, 't');
                        break;
                    case '\v':
                        c_string_append_c(string, 'v');
                        break;
                    default:
                        c_string_append_printf(string, "u%04x", c);
                        break;
                    }
                }
                else {
                    c_string_append_printf(string, "U%08x", c);
                }
            }
            str = c_utf8_next_char(str);
        }
        c_string_append_c(string, quote);
    } break;
    case C_VARIANT_CLASS_BYTE:
        if (type_annotate) {
            c_string_append(string, "byte ");
        }
        c_string_append_printf(string, "0x%02x", c_variant_get_byte(value));
        break;
    case C_VARIANT_CLASS_INT16:
        if (type_annotate) {
            c_string_append(string, "int16 ");
        }
        c_string_append_printf(string, "%d", c_variant_get_int16(value));
        break;
    case C_VARIANT_CLASS_UINT16:
        if (type_annotate) {
            c_string_append(string, "uint16 ");
        }
        c_string_append_printf(string, "%u", c_variant_get_uint16(value));
        break;
    case C_VARIANT_CLASS_INT32:
        c_string_append_printf(string, "%d", c_variant_get_int32(value));
        break;
    case C_VARIANT_CLASS_HANDLE:
        if (type_annotate) {
            c_string_append(string, "handle ");
        }
        c_string_append_printf(string, "%d", c_variant_get_handle(value));
        break;
    case C_VARIANT_CLASS_UINT32:
        if (type_annotate) {
            c_string_append(string, "uint32 ");
        }
        c_string_append_printf(string, "%u", c_variant_get_uint32(value));
        break;

    case C_VARIANT_CLASS_INT64:
        if (type_annotate) {
            c_string_append(string, "int64 ");
        }
        c_string_append_printf(string, "%lld", c_variant_get_int64(value));
        break;
    case C_VARIANT_CLASS_UINT64:
        if (type_annotate) {
            c_string_append(string, "uint64 ");
        }
        c_string_append_printf(string, "%llu", c_variant_get_uint64(value));
        break;
    case C_VARIANT_CLASS_DOUBLE: {
        cchar buffer[100];
        cint i;
        c_ascii_dtostr(buffer, sizeof buffer, c_variant_get_double(value));
        for (i = 0; buffer[i]; i++) {
            if (buffer[i] == '.' || buffer[i] == 'e' || buffer[i] == 'n' || buffer[i] == 'N') {
                break;
            }
        }
        if (buffer[i] == '\0') {
            buffer[i++] = '.';
            buffer[i++] = '0';
            buffer[i++] = '\0';
        }
        c_string_append(string, buffer);
        break;
    }
    case C_VARIANT_CLASS_OBJECT_PATH: {
        if (type_annotate) {
            c_string_append(string, "objectpath ");
        }
        c_string_append_printf(string, "\'%s\'", c_variant_get_string(value, NULL));
        break;
    }
    case C_VARIANT_CLASS_SIGNATURE: {
        if (type_annotate) {
            c_string_append(string, "signature ");
        }
        c_string_append_printf(string, "\'%s\'", c_variant_get_string(value, NULL));
        break;
    }
    default: {
        c_assert_not_reached();
    }
    }

    return string;
}

cuint c_variant_hash(const void *value_)
{
    CVariant *value = (CVariant *)value_;

    switch (c_variant_classify(value)) {
    case C_VARIANT_CLASS_STRING:
    case C_VARIANT_CLASS_OBJECT_PATH:
    case C_VARIANT_CLASS_SIGNATURE:
        return c_str_hash(c_variant_get_string(value, NULL));

    case C_VARIANT_CLASS_BOOLEAN:
        /* this is a very odd thing to hash... */
        return c_variant_get_boolean(value);

    case C_VARIANT_CLASS_BYTE:
        return c_variant_get_byte(value);

    case C_VARIANT_CLASS_INT16:
    case C_VARIANT_CLASS_UINT16: {
        const cuint16 *ptr;

        ptr = c_variant_get_data(value);

        if (ptr)
            return *ptr;
        else
            return 0;
    }

    case C_VARIANT_CLASS_INT32:
    case C_VARIANT_CLASS_UINT32:
    case C_VARIANT_CLASS_HANDLE: {
        const cuint *ptr;

        ptr = c_variant_get_data(value);

        if (ptr)
            return *ptr;
        else
            return 0;
    }

    case C_VARIANT_CLASS_INT64:
    case C_VARIANT_CLASS_UINT64:
    case C_VARIANT_CLASS_DOUBLE:
        /* need a separate case for these guys because otherwise
         * performance could be quite bad on big endian systems
         */
        {
            const cuint *ptr;

            ptr = c_variant_get_data(value);

            if (ptr)
                return ptr[0] + ptr[1];
            else
                return 0;
        }

    default:
        c_return_val_if_fail(!c_variant_is_container(value), 0);
        c_assert_not_reached();
    }
}

bool c_variant_equal(const void *one, const void *two)
{
    bool equal;

    c_return_val_if_fail(one != NULL && two != NULL, false);

    if (c_variant_get_type_info((CVariant *)one) != c_variant_get_type_info((CVariant *)two)) {
        return false;
    }

    if (c_variant_is_trusted((CVariant *)one) && c_variant_is_trusted((CVariant *)two)) {
        const void *data_one;
        const void *data_two;
        csize size_one, size_two;

        size_one = c_variant_get_size((CVariant *)one);
        size_two = c_variant_get_size((CVariant *)two);

        if (size_one != size_two) {
            return false;
        }

        data_one = c_variant_get_data((CVariant *)one);
        data_two = c_variant_get_data((CVariant *)two);

        if (size_one)
            equal = memcmp(data_one, data_two, size_one) == 0;
        else
            equal = true;
    }
    else {
        cchar *strone, *strtwo;
        strone = c_variant_print((CVariant *)one, false);
        strtwo = c_variant_print((CVariant *)two, false);
        equal = strcmp(strone, strtwo) == 0;
        c_free(strone);
        c_free(strtwo);
    }

    return equal;
}

CVariant *c_variant_get_normal_form(CVariant *value)
{
    CVariant *trusted;

    if (c_variant_is_normal_form(value))
        return c_variant_ref(value);

    trusted = c_variant_deep_copy(value, false);
    c_assert(c_variant_is_trusted(trusted));

    return c_variant_ref_sink(trusted);
}

bool c_variant_is_normal_form(CVariant *value)
{
    if (value->state & STATE_TRUSTED) {
        return true;
    }

    c_variant_lock(value);

    if (value->depth >= C_VARIANT_MAX_RECURSION_DEPTH) {
        return false;
    }

    if (value->state & STATE_SERIALISED) {
        if (c_variant_serialised_is_normal(c_variant_to_serialised(value))) {
            value->state |= STATE_TRUSTED;
        }
    }
    else {
        bool normal = true;
        csize i;

        for (i = 0; i < value->contents.tree.n_children; i++) {
            normal &= c_variant_is_normal_form(value->contents.tree.children[i]);
        }

        if (normal) {
            value->state |= STATE_TRUSTED;
        }
    }

    c_variant_unlock(value);

    return (value->state & STATE_TRUSTED) != 0;
}

CVariant *c_variant_byteswap(CVariant *value)
{
    CVariantTypeInfo *type_info;
    cuint alignment;
    CVariant *new;

    type_info = c_variant_get_type_info(value);

    c_variant_type_info_query(type_info, &alignment, NULL);

    if (alignment && c_variant_is_normal_form(value)) {
        /* (potentially) contains multi-byte numeric data, but is also already in
         * normal form so we can use a faster byteswapping codepath on the
         * serialised data */
        CVariantSerialised serialised = {
            0,
        };
        CBytes *bytes;

        serialised.type_info = c_variant_get_type_info(value);
        serialised.size = c_variant_get_size(value);
        serialised.data = c_malloc0(serialised.size);
        serialised.depth = c_variant_get_depth(value);
        serialised.ordered_offsets_up_to = C_MAX_SIZE; /* operating on the normal form */
        serialised.checked_offsets_up_to = C_MAX_SIZE;
        c_variant_store(value, serialised.data);

        c_variant_serialised_byteswap(serialised);

        bytes = c_bytes_new_take(serialised.data, serialised.size);
        new = c_variant_ref_sink(c_variant_new_take_bytes(c_variant_get_type(value), c_steal_pointer(&bytes), true));
    }
    else if (alignment)
        /* (potentially) contains multi-byte numeric data */
        new = c_variant_ref_sink(c_variant_deep_copy(value, true));
    else
        /* contains no multi-byte data */
        new = c_variant_get_normal_form(value);

    c_assert(c_variant_is_trusted(new));

    return c_steal_pointer(&new);
}

CVariant *c_variant_new_from_bytes(const CVariantType *type, CBytes *bytes, bool trusted)
{
    return c_variant_new_take_bytes(type, c_bytes_ref(bytes), trusted);
}

CVariant *c_variant_new_from_data(const CVariantType *type, const void *data, csize size, bool trusted,
                                  CDestroyNotify notify, void *user_data)
{
}

CVariantIter *c_variant_iter_new(CVariant *value)
{
    CVariantIter *iter;

    iter = (CVariantIter *)c_malloc0(sizeof(struct heap_iter));
    CVHI(iter)->value_ref = c_variant_ref(value);
    CVHI(iter)->magic = CVHI_MAGIC;

    c_variant_iter_init(iter, value);

    return iter;
}

csize c_variant_iter_init(CVariantIter *iter, CVariant *value)
{
    CVSI(iter)->magic = CVSI_MAGIC;
    CVSI(iter)->value = value;
    CVSI(iter)->n = c_variant_n_children(value);
    CVSI(iter)->i = -1;
    CVSI(iter)->loop_format = NULL;

    return CVSI(iter)->n;
}

CVariantIter *c_variant_iter_copy(CVariantIter *iter)
{
    CVariantIter *copy;

    c_return_val_if_fail(is_valid_iter(iter), 0);

    copy = c_variant_iter_new(CVSI(iter)->value);
    CVSI(copy)->i = CVSI(iter)->i;

    return copy;
}

csize c_variant_iter_n_children(CVariantIter *iter)
{
    c_return_val_if_fail(is_valid_iter(iter), 0);

    return CVSI(iter)->n;
}

void c_variant_iter_free(CVariantIter *iter)
{
    c_return_if_fail(is_valid_heap_iter(iter));

    c_variant_unref(CVHI(iter)->value_ref);
    CVHI(iter)->magic = 0;

    c_free0(CVHI(iter));
}

CVariant *c_variant_iter_next_value(CVariantIter *iter)
{
    c_return_val_if_fail(is_valid_iter(iter), false);

    if C_UNLIKELY (CVSI(iter)->i >= CVSI(iter)->n) {
        C_LOG_CRIT("c_variant_iter_next_value: must not be called again "
                   "after NULL has already been returned.");
        return NULL;
    }

    CVSI(iter)->i++;

    if (CVSI(iter)->i < CVSI(iter)->n) {
        return c_variant_get_child_value(CVSI(iter)->value, CVSI(iter)->i);
    }

    return NULL;
}

bool c_variant_iter_next(CVariantIter *iter, const cchar *formatStr, ...)
{
    CVariant *value;

    value = c_variant_iter_next_value(iter);

    c_return_val_if_fail(valid_format_string(formatStr, true, value), false);

    if (value != NULL) {
        va_list ap;

        va_start(ap, formatStr);
        c_variant_valist_get(&formatStr, value, false, &ap);
        va_end(ap);

        c_variant_unref(value);
    }

    return value != NULL;
}

bool c_variant_iter_loop(CVariantIter *iter, const cchar *formatStr, ...)
{
    bool first_time = CVSI(iter)->loop_format == NULL;
    CVariant *value;
    va_list ap;

    c_return_val_if_fail(first_time || formatStr == CVSI(iter)->loop_format, false);

    if (first_time) {
        TYPE_CHECK(CVSI(iter)->value, C_VARIANT_TYPE_ARRAY, false);
        CVSI(iter)->loop_format = formatStr;

        if (strchr(formatStr, '&'))
            c_variant_get_data(CVSI(iter)->value);
    }

    value = c_variant_iter_next_value(iter);

    c_return_val_if_fail(!first_time || valid_format_string(formatStr, true, value), false);

    va_start(ap, formatStr);
    c_variant_valist_get(&formatStr, value, !first_time, &ap);
    va_end(ap);

    if (value != NULL)
        c_variant_unref(value);

    return value != NULL;
}

CVariantBuilder *c_variant_builder_new(const CVariantType *type)
{
    CVariantBuilder *builder;

    builder = (CVariantBuilder *)c_malloc0(sizeof(struct heap_builder));
    c_variant_builder_init(builder, type);
    CVHB(builder)->magic = CVHB_MAGIC;
    CVHB(builder)->ref_count = 1;

    return builder;
}

void c_variant_builder_unref(CVariantBuilder *builder)
{
    c_return_if_fail(is_valid_heap_builder(builder));

    if (--CVHB(builder)->ref_count)
        return;

    c_variant_builder_clear(builder);
    CVHB(builder)->magic = 0;

    c_free0(CVHB(builder));
}

CVariantBuilder *c_variant_builder_ref(CVariantBuilder *builder)
{
    c_return_val_if_fail(is_valid_heap_builder(builder), NULL);

    CVHB(builder)->ref_count++;

    return builder;
}

void c_variant_builder_init(CVariantBuilder *builder, const CVariantType *type)
{
    _c_variant_builder_init(builder, c_variant_type_copy(type), true);
}

void c_variant_builder_init_static(CVariantBuilder *builder, const CVariantType *type)
{
    _c_variant_builder_init(builder, type, false);
}

CVariant *c_variant_builder_end(CVariantBuilder *builder)
{
    const CVariantType *type;
    CVariantType *new_type = NULL;
    CVariant *value;
    CVariant **children;

    return_val_if_invalid_builder(builder, NULL);
    c_return_val_if_fail(CVSB(builder)->offset >= CVSB(builder)->min_items, NULL);
    c_return_val_if_fail(!CVSB(builder)->uniform_item_types || CVSB(builder)->prev_item_type != NULL ||
                             c_variant_type_is_definite(CVSB(builder)->type),
                         NULL);

    if (c_variant_type_is_definite(CVSB(builder)->type))
        type = CVSB(builder)->type;

    else if (c_variant_type_is_maybe(CVSB(builder)->type))
        type = new_type = c_variant_make_maybe_type(CVSB(builder)->children[0]);

    else if (c_variant_type_is_array(CVSB(builder)->type))
        type = new_type = c_variant_make_array_type(CVSB(builder)->children[0]);

    else if (c_variant_type_is_tuple(CVSB(builder)->type))
        type = new_type = c_variant_make_tuple_type(CVSB(builder)->children, CVSB(builder)->offset);
    else if (c_variant_type_is_dict_entry(CVSB(builder)->type))
        type = new_type = c_variant_make_dict_entry_type(CVSB(builder)->children[0], CVSB(builder)->children[1]);
    else
        c_assert_not_reached();

    children = CVSB(builder)->children;

    /* shrink allocation to release extra space to allocator */
    if C_UNLIKELY (CVSB(builder)->offset < CVSB(builder)->allocated_children)
        children = c_realloc(children, CVSB(builder)->offset * sizeof(CVariant *));

    value = c_variant_new_from_children(type, children, CVSB(builder)->offset, CVSB(builder)->trusted);
    CVSB(builder)->children = NULL;
    CVSB(builder)->offset = 0;

    c_variant_builder_clear(builder);

    if (new_type != NULL)
        c_variant_type_free(new_type);

    return value;
}

void c_variant_builder_clear(CVariantBuilder *builder)
{
    csize i;
    if (CVSB(builder)->magic == 0)
        /* all-zeros or partial case */
        return;

    return_if_invalid_builder(builder);

    if (CVSB(builder)->type_owned)
        c_variant_type_free(CVSB(builder)->type);

    for (i = 0; i < CVSB(builder)->offset; i++)
        c_variant_unref(CVSB(builder)->children[i]);

    c_free(CVSB(builder)->children);

    if (CVSB(builder)->parent) {
        c_variant_builder_clear(CVSB(builder)->parent);
        c_free(CVSB(builder)->parent);
    }

    memset(builder, 0, sizeof(CVariantBuilder));
}

void c_variant_builder_open(CVariantBuilder *builder, const CVariantType *type)
{
    CVariantBuilder *parent;

    return_if_invalid_builder(builder);
    c_return_if_fail(CVSB(builder)->offset < CVSB(builder)->max_items);
    c_return_if_fail(!CVSB(builder)->expected_type || c_variant_type_is_subtype_of(type, CVSB(builder)->expected_type));
    c_return_if_fail(!CVSB(builder)->prev_item_type ||
                     c_variant_type_is_subtype_of(CVSB(builder)->prev_item_type, type));

    parent = c_memdup(builder, sizeof(CVariantBuilder));
    c_variant_builder_init(builder, type);
    CVSB(builder)->parent = parent;

    /* push the prev_item_type down into the subcontainer */
    if (CVSB(parent)->prev_item_type) {
        if (!CVSB(builder)->uniform_item_types) {
            CVSB(builder)->prev_item_type = c_variant_type_first(CVSB(parent)->prev_item_type);
        }
        else if (!c_variant_type_is_variant(CVSB(builder)->type)) {
            CVSB(builder)->prev_item_type = c_variant_type_element(CVSB(parent)->prev_item_type);
        }
    }
}

void c_variant_builder_close(CVariantBuilder *builder)
{
    CVariantBuilder *parent;

    return_if_invalid_builder(builder);
    c_return_if_fail(CVSB(builder)->parent != NULL);

    parent = CVSB(builder)->parent;
    CVSB(builder)->parent = NULL;

    c_variant_builder_add_value(parent, c_variant_builder_end(builder));
    *builder = *parent;

    c_free(parent);
}

void c_variant_builder_add_value(CVariantBuilder *builder, CVariant *value)
{
    return_if_invalid_builder(builder);
    c_return_if_fail(CVSB(builder)->offset < CVSB(builder)->max_items);
    c_return_if_fail(!CVSB(builder)->expected_type || c_variant_is_of_type(value, CVSB(builder)->expected_type));
    c_return_if_fail(!CVSB(builder)->prev_item_type || c_variant_is_of_type(value, CVSB(builder)->prev_item_type));

    CVSB(builder)->trusted &= c_variant_is_trusted(value);

    if (!CVSB(builder)->uniform_item_types) {
        /* advance our expected type pointers */
        if (CVSB(builder)->expected_type)
            CVSB(builder)->expected_type = c_variant_type_next(CVSB(builder)->expected_type);

        if (CVSB(builder)->prev_item_type)
            CVSB(builder)->prev_item_type = c_variant_type_next(CVSB(builder)->prev_item_type);
    }
    else
        CVSB(builder)->prev_item_type = c_variant_get_type(value);

    c_variant_builder_make_room(CVSB(builder));

    CVSB(builder)->children[CVSB(builder)->offset++] = c_variant_ref_sink(value);
}

void c_variant_builder_add(CVariantBuilder *builder, const cchar *formatStr, ...)
{
    CVariant *variant;
    va_list ap;

    va_start(ap, formatStr);
    variant = c_variant_new_va(formatStr, NULL, &ap);
    va_end(ap);

    c_variant_builder_add_value(builder, variant);
}

void c_variant_builder_add_parsed(CVariantBuilder *builder, const cchar *format, ...)
{
    va_list ap;

    va_start(ap, format);
    c_variant_builder_add_value(builder, c_variant_new_parsed_va(format, &ap));
    va_end(ap);
}

CVariant *c_variant_new(const cchar *formatStr, ...)
{
    CVariant *value;
    va_list ap;

    c_return_val_if_fail(valid_format_string(formatStr, true, NULL) && formatStr[0] != '?' && formatStr[0] != '@' &&
                             formatStr[0] != '*' && formatStr[0] != 'r',
                         NULL);

    va_start(ap, formatStr);
    value = c_variant_new_va(formatStr, NULL, &ap);
    va_end(ap);

    return value;
}

void c_variant_get(CVariant *value, const cchar *formatStr, ...)
{
    va_list ap;

    c_return_if_fail(value != NULL);
    c_return_if_fail(valid_format_string(formatStr, true, value));

    /* if any direct-pointer-access formats are in use, flatten first */
    if (strchr(formatStr, '&')) {
        c_variant_get_data(value);
    }

    va_start(ap, formatStr);
    c_variant_get_va(value, formatStr, NULL, &ap);
    va_end(ap);
}

CVariant *c_variant_new_va(const cchar *formatStr, const cchar **endPtr, va_list *app)
{
    CVariant *value;

    c_return_val_if_fail(valid_format_string(formatStr, !endPtr, NULL), NULL);
    c_return_val_if_fail(app != NULL, NULL);

    value = c_variant_valist_new(&formatStr, app);
    if (endPtr != NULL) {
        *endPtr = formatStr;
    }

    return value;
}

void c_variant_get_va(CVariant *value, const cchar *formatStr, const cchar **endPtr, va_list *app)
{
    c_return_if_fail(valid_format_string(formatStr, !endPtr, value));
    c_return_if_fail(value != NULL);
    c_return_if_fail(app != NULL);

    /* if any direct-pointer-access formats are in use, flatten first */
    if (strchr(formatStr, '&')) {
        c_variant_get_data(value);
    }

    c_variant_valist_get(&formatStr, value, false, app);

    if (endPtr != NULL) {
        *endPtr = formatStr;
    }
}

bool c_variant_check_format_string(CVariant *value, const cchar *formatStr, bool copyOnly)
{
    const cchar *original_format = formatStr;
    const cchar *type_string;

    type_string = c_variant_get_type_string(value);
    while (*type_string || *formatStr) {
        cchar format = *formatStr++;
        switch (format) {
        case '&':
            if C_UNLIKELY (copyOnly) {
                C_LOG_CRIT("g_variant_check_format_string() is being called by a function with a GVariant varargs "
                           "interface to validate the passed format string for type safety.  The passed format "
                           "(%s) contains a '&' character which would result in a pointer being returned to the "
                           "data inside of a GVariant instance that may no longer exist by the time the function "
                           "returns.  Modify your code to use a format string without '&'.",
                           original_format);
                return false;
            }
            C_FALLTHROUGH;
        case '^':
        case '@':
            /* ignore these 2 (or 3) */
            continue;
        case '?': {
            /* attempt to consume one of 'bynqiuxthdsog' */
            char s = *type_string++;
            if (s == '\0' || strchr("bynqiuxthdsog", s) == NULL) {
                return false;
            }
            continue;
        }
        case 'r': {
            /* ensure it's a tuple */
            if (*type_string != '(') {
                return false;
            }
            C_FALLTHROUGH;
        }
        case '*': {
            /* consume a full type string for the '*' or 'r' */
            if (!c_variant_type_string_scan(type_string, NULL, &type_string)) {
                return false;
            }
            continue;
        }
        default: {
            /* attempt to consume exactly one character equal to the format */
            if (format != *type_string++) {
                return false;
            }
        }
        }
    }

    return true;
}

CVariant *c_variant_parse(const CVariantType *type, const cchar *text, const cchar *limit, const cchar **endPtr,
                          CError **error)
{
    TokenStream stream = {
        0,
    };
    CVariant *result = NULL;
    AST *ast;

    c_return_val_if_fail(text != NULL, NULL);
    c_return_val_if_fail(text == limit || text != NULL, NULL);

    stream.start = text;
    stream.stream = text;
    stream.end = limit;

    if ((ast = parse(&stream, C_VARIANT_MAX_RECURSION_DEPTH, NULL, error))) {
        if (type == NULL) {
            result = ast_resolve(ast, error);
        }
        else {
            result = ast_get_value(ast, type, error);
        }

        if (result != NULL) {
            c_variant_ref_sink(result);
            if (endPtr == NULL) {
                while (stream.stream != limit && c_ascii_isspace(*stream.stream)) {
                    stream.stream++;
                }
                if (stream.stream != limit && *stream.stream != '\0') {
                    SourceRef ref = {stream.stream - text, stream.stream - text};
                    parser_set_error(error, &ref, NULL, C_VARIANT_PARSE_ERROR_INPUT_NOT_AT_END,
                                     "expected end of input");
                    c_variant_unref(result);
                    result = NULL;
                }
            }
            else {
                *endPtr = stream.stream;
            }
        }

        ast_free(ast);
    }

    return result;
}

CVariant *c_variant_new_parsed(const cchar *format, ...)
{
    CVariant *result;
    va_list ap;

    va_start(ap, format);
    result = c_variant_new_parsed_va(format, &ap);
    va_end(ap);

    return result;
}

CVariant *c_variant_new_parsed_va(const cchar *format, va_list *app)
{
    TokenStream stream = {
        0,
    };
    CVariant *result = NULL;
    CError *error = NULL;
    AST *ast;

    c_return_val_if_fail(format != NULL, NULL);
    c_return_val_if_fail(app != NULL, NULL);

    stream.start = format;
    stream.stream = format;
    stream.end = NULL;

    if ((ast = parse(&stream, C_VARIANT_MAX_RECURSION_DEPTH, app, &error))) {
        result = ast_resolve(ast, &error);
        ast_free(ast);
    }

    if (error != NULL) {
        C_LOG_ERROR("g_variant_new_parsed: %s", error->message);
    }

    if (*stream.stream) {
        C_LOG_ERROR("g_variant_new_parsed: trailing text after value");
    }

    c_clear_error(&error);

    return result;
}

cchar *c_variant_parse_error_print_context(CError *error, const cchar *sourceStr)
{
    const cchar *colon, *dash, *comma;
    bool success = false;
    CString *err;

    c_return_val_if_fail(error->domain == C_VARIANT_PARSE_ERROR, false);

    /**
     * We can only have a limited number of possible types of ranges
     * emitted from the parser:
     *
     *  - a:          -- usually errors from the tokeniser (eof, invalid char, etc.)
     *  - a-b:        -- usually errors from handling one single token
     *  - a-b,c-d:    -- errors involving two tokens (ie: type inferencing)
     *
     * We never see, for example "a,c".
     */

    colon = strchr(error->message, ':');
    dash = strchr(error->message, '-');
    comma = strchr(error->message, ',');

    if (!colon) {
        return NULL;
    }

    err = c_string_new(colon + 1);
    c_string_append(err, ":\n");

    if (dash == NULL || colon < dash) {
        cuint point;
        if (!parse_num(error->message, colon, &point)) {
            goto out;
        }
        if (point >= strlen(sourceStr)) {
            add_last_line(err, sourceStr);
        }
        else {
            add_lines_from_range(err, sourceStr, sourceStr + point, sourceStr + point + 1, NULL, NULL);
        }
    }
    else {
        if (comma && comma < colon) {
            cuint start1, end1, start2, end2;
            const cchar *dash2;

            dash2 = strchr(comma, '-');

            if (!parse_num(error->message, dash, &start1) || !parse_num(dash + 1, comma, &end1) ||
                !parse_num(comma + 1, dash2, &start2) || !parse_num(dash2 + 1, colon, &end2)) {
                goto out;
            }

            add_lines_from_range(err, sourceStr, sourceStr + start1, sourceStr + end1, sourceStr + start2,
                                 sourceStr + end2);
        }
        else {
            cuint start, end;
            /* One range */
            if (!parse_num(error->message, dash, &start) || !parse_num(dash + 1, colon, &end)) {
                goto out;
            }
            add_lines_from_range(err, sourceStr, sourceStr + start, sourceStr + end, NULL, NULL);
        }
    }

    success = true;

out:
    return c_string_free(err, !success);
}

cint c_variant_compare(const void *one, const void *two)
{
    CVariant *a = (CVariant *)one;
    CVariant *b = (CVariant *)two;

    c_return_val_if_fail(c_variant_classify(a) == c_variant_classify(b), 0);

    switch (c_variant_classify(a)) {
    case C_VARIANT_CLASS_BOOLEAN:
        return c_variant_get_boolean(a) - c_variant_get_boolean(b);
    case C_VARIANT_CLASS_BYTE:
        return ((cint)c_variant_get_byte(a)) - ((cint)c_variant_get_byte(b));
    case C_VARIANT_CLASS_INT16:
        return ((cint)c_variant_get_int16(a)) - ((cint)c_variant_get_int16(b));
    case C_VARIANT_CLASS_UINT16:
        return ((cint)c_variant_get_uint16(a)) - ((cint)c_variant_get_uint16(b));
    case C_VARIANT_CLASS_INT32: {
        cint32 a_val = c_variant_get_int32(a);
        cint32 b_val = c_variant_get_int32(b);
        return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
    }
    case C_VARIANT_CLASS_UINT32: {
        cuint32 a_val = c_variant_get_uint32(a);
        cuint32 b_val = c_variant_get_uint32(b);
        return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
    }
    case C_VARIANT_CLASS_INT64: {
        cint64 a_val = c_variant_get_int64(a);
        cint64 b_val = c_variant_get_int64(b);
        return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
    }
    case C_VARIANT_CLASS_UINT64: {
        cuint64 a_val = c_variant_get_uint64(a);
        cuint64 b_val = c_variant_get_uint64(b);
        return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
    }
    case C_VARIANT_CLASS_DOUBLE: {
        cdouble a_val = c_variant_get_double(a);
        cdouble b_val = c_variant_get_double(b);
        return (a_val == b_val) ? 0 : (a_val > b_val) ? 1 : -1;
    }
    case C_VARIANT_CLASS_STRING:
    case C_VARIANT_CLASS_OBJECT_PATH:
    case C_VARIANT_CLASS_SIGNATURE:
        return strcmp(c_variant_get_string(a, NULL), c_variant_get_string(b, NULL));
    default: {
        c_return_val_if_fail(!c_variant_is_container(a), 0);
        c_assert_not_reached();
    }
    }
}

CVariantDict *c_variant_dict_new(CVariant *fromAsv)
{
    CVariantDict *dict;
    C_STATIC_ASSERT(sizeof(CVariantDict) >= sizeof(struct heap_dict));

    dict = c_malloc0(sizeof(CVariantDict));
    c_variant_dict_init(dict, fromAsv);
    CVHD(dict)->magic = CVHD_MAGIC;
    CVHD(dict)->ref_count = 1;

    return dict;
}

void c_variant_dict_init(CVariantDict *dict, CVariant *fromAsv)
{
    CVariantIter iter;
    cchar *key;
    CVariant *value;

    CVSD(dict)->values = c_hash_table_new_full(c_str_hash, c_str_equal, c_free0, (CDestroyNotify)c_variant_unref);
    CVSD(dict)->magic = CVSD_MAGIC;

    if (fromAsv) {
        c_variant_iter_init(&iter, fromAsv);
        while (c_variant_iter_next(&iter, "{sv}", &key, &value)) {
            c_hash_table_insert(CVSD(dict)->values, key, value);
        }
    }
}

bool c_variant_dict_lookup(CVariantDict *dict, const cchar *key, const cchar *formatStr, ...)
{
    CVariant *value;
    va_list ap;

    return_val_if_invalid_dict(dict, false);
    c_return_val_if_fail(key != NULL, false);
    c_return_val_if_fail(formatStr != NULL, false);

    value = c_hash_table_lookup(CVSD(dict)->values, key);
    if (value == NULL || !c_variant_check_format_string(value, formatStr, false)) {
        return false;
    }

    va_start(ap, formatStr);
    c_variant_get_va(value, formatStr, NULL, &ap);
    va_end(ap);

    return true;
}

CVariant *c_variant_dict_lookup_value(CVariantDict *dict, const cchar *key, const CVariantType *expectedType)
{
    CVariant *result;

    return_val_if_invalid_dict(dict, NULL);
    c_return_val_if_fail(key != NULL, NULL);

    result = c_hash_table_lookup(CVSD(dict)->values, key);
    if (result && (!expectedType || c_variant_is_of_type(result, expectedType))) {
        return c_variant_ref(result);
    }

    return NULL;
}

bool c_variant_dict_contains(CVariantDict *dict, const cchar *key)
{
    return_val_if_invalid_dict(dict, false);
    c_return_val_if_fail(key != NULL, false);

    return c_hash_table_contains(CVSD(dict)->values, key);
}

void c_variant_dict_insert(CVariantDict *dict, const cchar *key, const cchar *formatStr, ...)
{
    va_list ap;

    return_if_invalid_dict(dict);
    c_return_if_fail(key != NULL);
    c_return_if_fail(formatStr != NULL);

    va_start(ap, formatStr);
    c_variant_dict_insert_value(dict, key, c_variant_new_va(formatStr, NULL, &ap));
    va_end(ap);
}

void c_variant_dict_insert_value(CVariantDict *dict, const cchar *key, CVariant *value)
{
    return_if_invalid_dict(dict);
    c_return_if_fail(key != NULL);
    c_return_if_fail(value != NULL);

    c_hash_table_insert(CVSD(dict)->values, c_strdup(key), c_variant_ref_sink(value));
}

bool c_variant_dict_remove(CVariantDict *dict, const cchar *key)
{
    return_val_if_invalid_dict(dict, false);
    c_return_val_if_fail(key != NULL, false);

    return c_hash_table_remove(CVSD(dict)->values, key);
}

void c_variant_dict_clear(CVariantDict *dict)
{
    if (CVSD(dict)->magic == 0) {
        return;
    }

    return_if_invalid_dict(dict);

    c_hash_table_unref(CVSD(dict)->values);
    CVSD(dict)->values = NULL;
    CVSD(dict)->magic = 0;
}

CVariant *c_variant_dict_end(CVariantDict *dict)
{
    CVariantBuilder builder;
    CHashTableIter iter;
    void *key;
    void *value;

    return_val_if_invalid_dict(dict, NULL);

    c_variant_builder_init_static(&builder, C_VARIANT_TYPE_VARDICT);

    c_hash_table_iter_init(&iter, CVSD(dict)->values);
    while (c_hash_table_iter_next(&iter, &key, &value)) {
        c_variant_builder_add(&builder, "{sv}", (const cchar *)key, (CVariant *)value);
    }
    c_variant_dict_clear(dict);

    return c_variant_builder_end(&builder);
}

CVariantDict *c_variant_dict_ref(CVariantDict *dict)
{
    c_return_val_if_fail(is_valid_heap_dict(dict), NULL);

    CVHD(dict)->ref_count++;

    return dict;
}

void c_variant_dict_unref(CVariantDict *dict)
{
    c_return_if_fail(is_valid_heap_dict(dict));

    if (--CVHD(dict)->ref_count == 0) {
        c_variant_dict_clear(dict);
        c_free0(dict);
    }
}

CVariant *c_variant_new_take_bytes(const CVariantType *type, CBytes *bytes, bool trusted)
{
    CVariant *value;
    cuint alignment;
    csize size;
    CBytes *owned_bytes = NULL;
    CVariantSerialised serialised;

    value = c_variant_alloc(type, true, trusted, 0);

    c_variant_type_info_query(value->type_info, &alignment, &size);

    serialised.type_info = value->type_info;
    serialised.data = (cuchar *)c_bytes_get_data(bytes, &serialised.size);
    serialised.depth = 0;
    serialised.ordered_offsets_up_to = trusted ? C_MAX_SIZE : 0;
    serialised.checked_offsets_up_to = trusted ? C_MAX_SIZE : 0;

    if (!c_variant_serialised_check(serialised)) {
#ifdef HAVE_POSIX_MEMALIGN
        void *aligned_data = NULL;
        csize aligned_size = c_bytes_get_size(bytes);
        if (aligned_size != 0 &&
            posix_memalign(&aligned_data, C_MAX(sizeof(void *), alignment + 1), aligned_size) != 0) {
            C_LOG_ERROR("posix_memalign failed");
        }

        if (aligned_size != 0) {
            memcpy(aligned_data, c_bytes_get_data(bytes, NULL), aligned_size);
        }

        owned_bytes = bytes;
        bytes = c_bytes_new_with_free_func(aligned_data, aligned_size, free, aligned_data);
        aligned_data = NULL;
#else
        owned_bytes = bytes;
        bytes = c_bytes_new(c_bytes_get_data(bytes, NULL), c_bytes_get_size(bytes));
#endif
    }

    value->contents.serialised.bytes = bytes;

    if (size && c_bytes_get_size(bytes) != size) {
        value->contents.serialised.data = NULL;
        value->size = size;
    }
    else {
        value->contents.serialised.data = c_bytes_get_data(bytes, &value->size);
    }

    value->contents.serialised.ordered_offsets_up_to = trusted ? C_MAX_SIZE : 0;
    value->contents.serialised.checked_offsets_up_to = trusted ? C_MAX_SIZE : 0;

    c_clear_pointer((void **)&owned_bytes, (CDestroyNotify)c_bytes_unref);

    return value;
}

CVariant *c_variant_new_preallocated_trusted(const CVariantType *type, const void *data, csize size)
{
    CVariant *value;
    csize expected_size;
    cuint alignment;

    value = c_variant_alloc(type, true, true, size);

    c_variant_type_info_query(value->type_info, &alignment, &expected_size);

    c_assert(expected_size == 0 || size == expected_size);

    value->contents.serialised.ordered_offsets_up_to = C_MAX_SIZE;
    value->contents.serialised.checked_offsets_up_to = C_MAX_SIZE;
    value->contents.serialised.bytes = NULL;
    value->contents.serialised.data = value->suffix;
    value->size = size;

    memcpy(value->suffix, data, size);

    return value;
}

CVariant *c_variant_new_from_children(const CVariantType *type, CVariant **children, csize n_children, bool trusted)
{
    CVariant *value;

    value = c_variant_alloc(type, false, trusted, 0);
    value->contents.tree.children = children;
    value->contents.tree.n_children = n_children;

    return value;
}

bool c_variant_serialised_check(CVariantSerialised serialised)
{
    csize fixed_size;
    cuint alignment;

    if (serialised.type_info == NULL) {
        return false;
    }
    c_variant_type_info_query(serialised.type_info, &alignment, &fixed_size);

    if (fixed_size != 0 && serialised.size != fixed_size) {
        return false;
    }
    else if (fixed_size == 0 && !(serialised.size == 0 || serialised.data != NULL)) {
        return false;
    }

    if (serialised.ordered_offsets_up_to > serialised.checked_offsets_up_to) {
        return false;
    }

    alignment &= sizeof(struct {
                     char a;
                     union
                     {
                         cuint64 x;
                         void *y;
                         cdouble z;
                     } b;
                 }) -
        9;

    return (serialised.size <= alignment || (alignment & (csize)serialised.data) == 0);
}

csize c_variant_serialised_n_children(CVariantSerialised serialised)
{
    c_assert(c_variant_serialised_check(serialised));

    DISPATCH_CASES(serialised.type_info, return cvs_ /**/, /**/ _n_children(serialised);)
    c_assert_not_reached();
}

CVariantSerialised c_variant_serialised_get_child(CVariantSerialised serialised, csize index_)
{
    CVariantSerialised child;

    c_assert(c_variant_serialised_check(serialised));

    if C_LIKELY (index_ < c_variant_serialised_n_children(serialised)) {
        DISPATCH_CASES(serialised.type_info, child = cvs_ /**/, /**/ _get_child(serialised, index_);
                       c_assert(child.size || child.data == NULL); c_assert(c_variant_serialised_check(child));
                       return child;)
        c_assert_not_reached();
    }

    C_LOG_CRIT("Attempt to access item %lu"
               " in a container with only %llu items",
               index_, c_variant_serialised_n_children(serialised));
}

void c_variant_serialiser_serialise(CVariantSerialised serialised, CVariantSerialisedFiller gvs_filler,
                                    const void **children, csize n_children)
{
    c_assert(c_variant_serialised_check(serialised));

    DISPATCH_CASES(serialised.type_info, cvs_ /**/, /**/ _serialise(serialised, gvs_filler, children, n_children);
                   return;)
    c_assert_not_reached();
}

csize c_variant_serialiser_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                       const void **children, csize n_children)
{
    DISPATCH_CASES(type_info, return cvs_ /**/, /**/ _needed_size(type_info, gvs_filler, children, n_children);)
    c_assert_not_reached();
}

void c_variant_serialised_byteswap(CVariantSerialised serialised)
{
    csize fixed_size;
    cuint alignment;

    c_assert(c_variant_serialised_check(serialised));

    if (!serialised.data)
        return;

    c_variant_type_info_query(serialised.type_info, &alignment, &fixed_size);
    if (!alignment)
        return;

    if (alignment + 1 == fixed_size) {
        switch (fixed_size) {
        case 2: {
            cuint16 *ptr = (cuint16 *)serialised.data;
            c_assert_cmpint(serialised.size, ==, 2);
            *ptr = C_UINT16_SWAP_LE_BE(*ptr);
            return;
        }
        case 4: {
            cuint32 *ptr = (cuint32 *)serialised.data;
            c_assert_cmpint(serialised.size, ==, 4);
            *ptr = C_UINT32_SWAP_LE_BE(*ptr);
            return;
        }
        case 8: {
            cuint64 *ptr = (cuint64 *)serialised.data;
            c_assert_cmpint(serialised.size, ==, 8);
            *ptr = C_UINT64_SWAP_LE_BE(*ptr);
            return;
        }
        default:
            c_assert_not_reached();
        }
    }
    else {
        csize children, i;
        children = c_variant_serialised_n_children(serialised);
        for (i = 0; i < children; i++) {
            CVariantSerialised child;
            child = c_variant_serialised_get_child(serialised, i);
            c_variant_serialised_byteswap(child);
            c_variant_type_info_unref(child.type_info);
        }
    }
}

bool c_variant_serialised_is_normal(CVariantSerialised serialised)
{
    if (serialised.depth >= C_VARIANT_MAX_RECURSION_DEPTH) {
        return false;
    }

    DISPATCH_CASES(serialised.type_info, return cvs_ /**/, /**/ _is_normal(serialised);)

    if (serialised.data == NULL) {
        return false;
    }

    /* some hard-coded terminal cases */
    switch (c_variant_type_info_get_type_char(serialised.type_info)) {
    case 'b': /* boolean */
        return serialised.data[0] < 2;
    case 's': /* string */
        return c_variant_serialiser_is_string(serialised.data, serialised.size);
    case 'o':
        return c_variant_serialiser_is_object_path(serialised.data, serialised.size);
    case 'g':
        return c_variant_serialiser_is_signature(serialised.data, serialised.size);
    default:
        /**
         * all of the other types are fixed-sized numerical types for
         * which all possible values are valid (including various NaN
         * representations for floating point values).
         */
        return true;
    }
}

bool c_variant_serialiser_is_string(const void *data, csize size)
{
    const cchar *expected_end;
    const cchar *end;

    /* Strings must end with a nul terminator. */
    if (size == 0) {
        return false;
    }

    expected_end = ((cchar *)data) + size - 1;

    if (*expected_end != '\0') {
        return false;
    }

    c_utf8_validate_len(data, size, &end);

    return end == expected_end;
}

bool c_variant_serialiser_is_object_path(const void *data, csize size)
{
    const cchar *str = data;
    csize i;

    if (!c_variant_serialiser_is_string(data, size)) {
        return false;
    }

    /* The path must begin with an ASCII '/' (integer 47) character */
    if (str[0] != '/') {
        return false;
    }

    for (i = 1; str[i]; i++) {
        /**
         * Each element must only contain the ASCII characters
         * "[A-Z][a-z][0-9]_"
         */
        if (c_ascii_isalnum(str[i]) || str[i] == '_')
            ;
        /* must consist of elements separated by slash characters. */
        else if (str[i] == '/') {
            /* No element may be the empty string. */
            /* Multiple '/' characters cannot occur in sequence. */
            if (str[i - 1] == '/') {
                return false;
            }
        }
        else {
            return false;
        }
    }
    /**
     * A trailing '/' character is not allowed unless the path is the
     * root path (a single '/' character).
     */
    if (i > 1 && str[i - 1] == '/') {
        return false;
    }

    return true;
}

bool c_variant_serialiser_is_signature(const void *data, csize size)
{
    const cchar *str = data;
    csize first_invalid;

    if (!c_variant_serialiser_is_string(data, size)) {
        return false;
    }

    /* make sure no non-definite characters appear */
    first_invalid = strspn(str, "ybnqiuxthdvasog(){}");
    if (str[first_invalid]) {
        return false;
    }

    /* make sure each type string is well-formed */
    while (*str) {
        if (!c_variant_type_string_scan(str, NULL, &str)) {
            return false;
        }
    }

    return true;
}

bool c_variant_format_string_scan(const cchar *str, const cchar *limit, const cchar **endPtr)
{
#define next_char() (str == limit ? '\0' : *(str++))
#define peek_char() (str == limit ? '\0' : *str)
    char c;

    switch (next_char()) {
    case 'b':
    case 'y':
    case 'n':
    case 'q':
    case 'i':
    case 'u':
    case 'x':
    case 't':
    case 'h':
    case 'd':
    case 's':
    case 'o':
    case 'g':
    case 'v':
    case '*':
    case '?':
    case 'r':
        break;
    case 'm':
        return c_variant_format_string_scan(str, limit, endPtr);
    case 'a':
    case '@':
        return c_variant_type_string_scan(str, limit, endPtr);
    case '(':
        while (peek_char() != ')') {
            if (!c_variant_format_string_scan(str, limit, &str)) {
                return false;
            }
        }
        next_char(); /* consume ')' */
        break;
    case '{':
        c = next_char();
        if (c == '&') {
            c = next_char();
            if (c != 's' && c != 'o' && c != 'g') {
                return false;
            }
        }
        else {
            if (c == '@') {
                c = next_char();
            }
            /* ISO/IEC 9899:1999 (C99) §7.21.5.2:
             * The terminating null character is considered to be
             * part of the string.
             */
            if (c != '\0' && strchr("bynqiuxthdsog?", c) == NULL) {
                return false;
            }
        }
        if (!c_variant_format_string_scan(str, limit, &str)) {
            return false;
        }
        if (next_char() != '}') {
            return false;
        }
        break;
    case '^':
        if ((c = next_char()) == 'a') {
            if ((c = next_char()) == '&') {
                if ((c = next_char()) == 'a') {
                    if ((c = next_char()) == 'y') {
                        break; /* '^a&ay' */
                    }
                }
                else if (c == 's' || c == 'o') {
                    break; /* '^a&s', '^a&o' */
                }
            }
            else if (c == 'a') {
                if ((c = next_char()) == 'y') {
                    break; /* '^aay' */
                }
            }
            else if (c == 's' || c == 'o') {
                break; /* '^as', '^ao' */
            }
            else if (c == 'y') {
                break; /* '^ay' */
            }
        }
        else if (c == '&') {
            if ((c = next_char()) == 'a') {
                if ((c = next_char()) == 'y') {
                    break; /* '^&ay' */
                }
            }
        }
        return false;
    case '&':
        c = next_char();
        if (c != 's' && c != 'o' && c != 'g') {
            return false;
        }
        break;
    default:
        return false;
    }

    if (endPtr != NULL) {
        *endPtr = str;
    }

#undef next_char
#undef peek_char

    return true;
}

CVariantType *c_variant_format_string_scan_type(const cchar *str, const cchar *limit, const cchar **endPtr)
{
    const cchar *my_end;
    csize i;
    cchar *new;

    if (endPtr == NULL) {
        endPtr = &my_end;
    }

    if (!c_variant_format_string_scan(str, limit, endPtr)) {
        return NULL;
    }

    new = c_malloc0(*endPtr - str + 1);
    i = 0;
    while (str != *endPtr) {
        if (*str != '@' && *str != '&' && *str != '^') {
            new[i++] = *str;
        }
        str++;
    }
    new[i++] = '\0';

    c_assert(c_variant_type_string_is_valid(new));

    return (CVariantType *)new;
}

static bool variant_type_string_scan_internal(const cchar *string, const cchar *limit, const cchar **endptr,
                                              csize *depth, csize depth_limit)
{
    csize max_depth = 0, child_depth;

    c_return_val_if_fail(string != NULL, false);

    if (string == limit || *string == '\0') {
        return false;
    }

    switch (*string++) {
    case '(': {
        while (string == limit || *string != ')') {
            if (depth_limit == 0 ||
                !variant_type_string_scan_internal(string, limit, &string, &child_depth, depth_limit - 1)) {
                return false;
            }
            max_depth = C_MAX(max_depth, child_depth + 1);
        }
        string++;
        break;
    }
    case '{': {
        if (depth_limit == 0 || string == limit || *string == '\0' || /* { */
            !strchr("bynqihuxtdsog?", *string++) || /* key */
            !variant_type_string_scan_internal(string, limit, &string, &child_depth, depth_limit - 1) || /* value */
            string == limit || *string++ != '}') /* } */
            return false;
        max_depth = C_MAX(max_depth, child_depth + 1);
        break;
    }
    case 'm':
    case 'a': {
        if (depth_limit == 0 ||
            !variant_type_string_scan_internal(string, limit, &string, &child_depth, depth_limit - 1)) {
            return false;
        }
        max_depth = C_MAX(max_depth, child_depth + 1);
        break;
    }
    case 'b':
    case 'y':
    case 'n':
    case 'q':
    case 'i':
    case 'u':
    case 'x':
    case 't':
    case 'd':
    case 's':
    case 'o':
    case 'g':
    case 'v':
    case 'r':
    case '*':
    case '?':
    case 'h': {
        max_depth = C_MAX(max_depth, 1);
        break;
    }
    default:
        return false;
    }

    if (endptr != NULL) {
        *endptr = string;
    }
    if (depth != NULL) {
        *depth = max_depth;
    }

    return true;
}

static bool c_variant_type_check(const CVariantType *type)
{
    if (type == NULL) {
        return false;
    }

#if 0
    return c_variant_type_string_scan ((const cchar *) type, NULL, NULL);
#else
    return true;
#endif
}

static cuint _c_variant_type_hash(void const *type)
{
    const cchar *type_string = type;
    cuint value = 0;
    csize index = 0;
    int brackets = 0;

    do {
        value = (value << 5) - value + type_string[index];
        while (type_string[index] == 'a' || type_string[index] == 'm') {
            index++;
            value = (value << 5) - value + type_string[index];
        }

        if (type_string[index] == '(' || type_string[index] == '{') {
            brackets++;
        }
        else if (type_string[index] == ')' || type_string[index] == '}') {
            brackets--;
        }
        index++;
    }
    while (brackets);

    return value;
}

static bool _c_variant_type_equal(const CVariantType *type1, const CVariantType *type2)
{
    const char *str1 = (const char *)type1;
    const char *str2 = (const char *)type2;
    csize index = 0;
    int brackets = 0;

    if (str1 == str2) {
        return true;
    }

    do {
        if (str1[index] != str2[index]) {
            return false;
        }

        while (str1[index] == 'a' || str1[index] == 'm') {
            index++;
            if (str1[index] != str2[index]) {
                return false;
            }
        }

        if (str1[index] == '(' || str1[index] == '{') {
            brackets++;
        }
        else if (str1[index] == ')' || str1[index] == '}') {
            brackets--;
        }
        index++;
    }
    while (brackets);

    return true;
}

static CVariantType *c_variant_type_new_tuple_slow(const CVariantType *const *items, cint length)
{
    CString *string;
    cint i;

    string = c_string_new("(");
    for (i = 0; i < length; i++) {
        const CVariantType *type;
        csize size;
        c_return_val_if_fail(c_variant_type_check(items[i]), NULL);
        type = items[i];
        size = c_variant_type_get_string_length(type);
        c_string_append_len(string, (const cchar *)type, size);
    }
    c_string_append_c(string, ')');

    return (CVariantType *)c_string_free(string, false);
}

static void c_variant_type_info_check(const CVariantTypeInfo *info, char container_class)
{
    c_assert(!container_class || info->container_class == container_class);

    /* alignment can only be one of these */
    c_assert(info->alignment == 0 || info->alignment == 1 || info->alignment == 3 || info->alignment == 7);

    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *)info;

        /* extra checks for containers */
        c_assert(!c_atomic_ref_count_compare(&container->ref_count, 0));
        c_assert(container->type_string != NULL);
    }
    else {
        cint index;

        /* if not a container, then ensure that it is a valid member of
         * the basic types table
         */
        index = info - c_variant_type_info_basic_table;

        c_assert(C_N_ELEMENTS(c_variant_type_info_basic_table) == 24);
        c_assert(C_N_ELEMENTS(c_variant_type_info_basic_chars) == 24);
        c_assert(0 <= index && index < 24);
        c_assert(c_variant_type_info_basic_chars[index][0] != ' ');
    }
}

/* == array == */
static ArrayInfo *CV_ARRAY_INFO(CVariantTypeInfo *info)
{
    c_variant_type_info_check(info, CV_ARRAY_INFO_CLASS);

    return (ArrayInfo *)info;
}

static void array_info_free(CVariantTypeInfo *info)
{
    ArrayInfo *array_info;

    c_assert(info->container_class == CV_ARRAY_INFO_CLASS);
    array_info = (ArrayInfo *)info;

    c_variant_type_info_unref(array_info->element);
    c_free0(array_info);
}

static ContainerInfo *array_info_new(const CVariantType *type)
{
    ArrayInfo *info;

    info = c_malloc0(sizeof(ArrayInfo));
    info->container.info.container_class = CV_ARRAY_INFO_CLASS;

    info->element = c_variant_type_info_get(c_variant_type_element(type));
    info->container.info.alignment = info->element->alignment;
    info->container.info.fixed_size = 0;

    return (ContainerInfo *)info;
}

/* == tuple == */
static TupleInfo *CV_TUPLE_INFO(CVariantTypeInfo *info)
{
    c_variant_type_info_check(info, CV_TUPLE_INFO_CLASS);

    return (TupleInfo *)info;
}

static void tuple_info_free(CVariantTypeInfo *info)
{
    TupleInfo *tuple_info;
    csize i;

    c_assert(info->container_class == CV_TUPLE_INFO_CLASS);
    tuple_info = (TupleInfo *)info;

    for (i = 0; i < tuple_info->n_members; i++) {
        c_variant_type_info_unref(tuple_info->members[i].type_info);
    }

    c_free0(tuple_info->members);
    c_free0(tuple_info);
}

static void tuple_allocate_members(const CVariantType *type, CVariantMemberInfo **members, csize *n_members)
{
    const CVariantType *item_type;
    csize i = 0;

    *n_members = c_variant_type_n_items(type);
    *members = c_malloc0(sizeof(CVariantMemberInfo) * *n_members);

    item_type = c_variant_type_first(type);
    while (item_type) {
        CVariantMemberInfo *member = &(*members)[i++];
        member->type_info = c_variant_type_info_get(item_type);
        item_type = c_variant_type_next(item_type);

        if (member->type_info->fixed_size) {
            member->ending_type = C_VARIANT_MEMBER_ENDING_FIXED;
        }
        else if (item_type == NULL) {
            member->ending_type = C_VARIANT_MEMBER_ENDING_LAST;
        }
        else {
            member->ending_type = C_VARIANT_MEMBER_ENDING_OFFSET;
        }
    }

    c_assert(i == *n_members);
}

static bool tuple_get_item(TupleInfo *info, CVariantMemberInfo *item, csize *d, csize *e)
{
    if (&info->members[info->n_members] == item) {
        return false;
    }

    *d = item->type_info->alignment;
    *e = item->type_info->fixed_size;

    return true;
}

static void tuple_table_append(CVariantMemberInfo **items, csize i, csize a, csize b, csize c)
{
    CVariantMemberInfo *item = (*items)++;

    a += ~b & c; /* take the "aligned" part of 'c' and add to 'a' */
    c &= b; /* chop 'c' to contain only the unaligned part */


    item->i = i;
    item->a = a + b;
    item->b = ~b;
    item->c = c;
}

static csize tuple_align(csize offset, cuint alignment) { return offset + ((-offset) & alignment); }

static void tuple_generate_table(TupleInfo *info)
{
    CVariantMemberInfo *items = info->members;
    csize i = -1, a = 0, b = 0, c = 0, d, e;

    while (tuple_get_item(info, items, &d, &e)) {
        if (d <= b) {
            c = tuple_align(c, d); /* rule 1 */
        }
        else {
            a += tuple_align(c, b), b = d, c = 0; /* rule 2 */
        }

        tuple_table_append(&items, i, a, b, c);

        if (e == 0) {
            i++, a = b = c = 0;
        }
        else {
            c += e; /* rule 3 */
        }
    }
}

static void tuple_set_base_info(TupleInfo *info)
{
    CVariantTypeInfo *base = &info->container.info;

    if (info->n_members > 0) {
        CVariantMemberInfo *m;
        base->alignment = 0;
        for (m = info->members; m < &info->members[info->n_members]; m++) {
            base->alignment |= m->type_info->alignment;
        }

        m--; /* take 'm' back to the last item */
        if (m->i == (csize)-1 && m->type_info->fixed_size) {
            base->fixed_size = tuple_align(((m->a & m->b) | m->c) + m->type_info->fixed_size, base->alignment);
        }
        else {
            base->fixed_size = 0;
        }
    }
    else {
        base->alignment = 0;
        base->fixed_size = 1;
    }
}

static ContainerInfo *tuple_info_new(const CVariantType *type)
{
    TupleInfo *info;

    info = c_malloc0(sizeof(TupleInfo));
    info->container.info.container_class = CV_TUPLE_INFO_CLASS;

    tuple_allocate_members(type, &info->members, &info->n_members);
    tuple_generate_table(info);
    tuple_set_base_info(info);

    return (ContainerInfo *)info;
}

/* == new/ref/unref == */
static void cc_while_locked(void)
{
    while (c_variant_type_info_gc->len > 0) {
        CVariantTypeInfo *info = c_ptr_array_steal_index_fast(c_variant_type_info_gc, 0);
        ContainerInfo *container = (ContainerInfo *)info;
        if (c_atomic_ref_count_dec(&container->ref_count)) {
            c_hash_table_remove(c_variant_type_info_table, container->type_string);
            c_free(container->type_string);

            if (info->container_class == CV_ARRAY_INFO_CLASS)
                array_info_free(info);
            else if (info->container_class == CV_TUPLE_INFO_CLASS)
                tuple_info_free(info);
            else
                c_assert_not_reached();
        }
    }
}

static CVariant *c_variant_new_from_trusted(const CVariantType *type, const void *data, csize size)
{
    if (size <= C_VARIANT_MAX_PREALLOCATED) {
        return c_variant_new_preallocated_trusted(type, data, size);
    }
    else {
        return c_variant_new_take_bytes(type, c_bytes_new(data, size), true);
    }
}

static CVariantType *c_variant_make_tuple_type(CVariant *const *children, csize n_children)
{
    const CVariantType **types;
    CVariantType *type;
    csize i;

    types = c_malloc0(sizeof(const CVariantType *) * n_children);

    for (i = 0; i < n_children; i++) {
        types[i] = c_variant_get_type(children[i]);
    }

    type = c_variant_type_new_tuple(types, n_children);
    c_free(types);

    return type;
}

static CVariantType *c_variant_make_dict_entry_type(CVariant *key, CVariant *val)
{
    return c_variant_type_new_dict_entry(c_variant_get_type(key), c_variant_get_type(val));
}

static void c_variant_lock(CVariant *value) { c_bit_lock(&value->state, 0); }

static void c_variant_unlock(CVariant *value) { c_bit_unlock(&value->state, 0); }

static void c_variant_release_children(CVariant *value)
{
    csize i;

    c_assert(value->state & STATE_LOCKED);
    c_assert(~value->state & STATE_SERIALISED);

    for (i = 0; i < value->contents.tree.n_children; i++)
        c_variant_unref(value->contents.tree.children[i]);

    c_free(value->contents.tree.children);
}

static void c_variant_ensure_size(CVariant *value)
{
    c_assert(value->state & STATE_LOCKED);

    if (value->size == (csize)-1) {
        void **children;
        csize n_children;

        children = (void **)value->contents.tree.children;
        n_children = value->contents.tree.n_children;
        value->size =
            c_variant_serialiser_needed_size(value->type_info, c_variant_fill_gvs, (const void **)children, n_children);
    }
}

static void c_variant_serialise(CVariant *value, void *data)
{
    CVariantSerialised serialised = {
        0,
    };
    void **children;
    csize n_children;

    c_assert(~value->state & STATE_SERIALISED);
    c_assert(value->state & STATE_LOCKED);

    serialised.type_info = value->type_info;
    serialised.size = value->size;
    serialised.data = data;
    serialised.depth = value->depth;
    serialised.ordered_offsets_up_to = 0;
    serialised.checked_offsets_up_to = 0;

    children = (void **)value->contents.tree.children;
    n_children = value->contents.tree.n_children;

    c_variant_serialiser_serialise(serialised, c_variant_fill_gvs, (const void **)children, n_children);
}

static void c_variant_fill_gvs(CVariantSerialised *serialised, void *data)
{
    CVariant *value = data;

    c_variant_lock(value);
    c_variant_ensure_size(value);
    c_variant_unlock(value);

    if (serialised->type_info == NULL) {
        serialised->type_info = value->type_info;
    }
    c_assert(serialised->type_info == value->type_info);

    if (serialised->size == 0) {
        serialised->size = value->size;
    }
    c_assert(serialised->size == value->size);
    serialised->depth = value->depth;

    if (value->state & STATE_SERIALISED) {
        serialised->ordered_offsets_up_to = value->contents.serialised.ordered_offsets_up_to;
        serialised->checked_offsets_up_to = value->contents.serialised.checked_offsets_up_to;
    }
    else {
        serialised->ordered_offsets_up_to = 0;
        serialised->checked_offsets_up_to = 0;
    }

    if (serialised->data) {
        c_variant_store(value, serialised->data);
    }
}

static void c_variant_ensure_serialised(CVariant *value)
{
    c_assert(value->state & STATE_LOCKED);

    if (~value->state & STATE_SERIALISED) {
        CBytes *bytes;
        void *data;

        c_variant_ensure_size(value);
        data = c_malloc0(value->size);
        c_variant_serialise(value, data);

        c_variant_release_children(value);

        bytes = c_bytes_new_take(data, value->size);
        value->contents.serialised.data = c_bytes_get_data(bytes, NULL);
        value->contents.serialised.bytes = bytes;
        value->contents.serialised.ordered_offsets_up_to = C_MAX_SIZE;
        value->contents.serialised.checked_offsets_up_to = C_MAX_SIZE;
        value->state |= STATE_SERIALISED;
    }
}

static CVariant *c_variant_alloc(const CVariantType *type, bool serialised, bool trusted, csize suffix_size)
{
    C_UNUSED bool size_check;
    CVariant *value;
    csize size;

    size_check = c_size_checked_add(&size, sizeof *value, suffix_size);
    c_assert(size_check);

    value = c_malloc0(size);
    value->type_info = c_variant_type_info_get(type);
    value->state = (serialised ? STATE_SERIALISED : 0) | (trusted ? STATE_TRUSTED : 0) | STATE_FLOATING;
    value->size = (cssize)-1;
    c_atomic_ref_count_init(&value->ref_count);
    value->depth = 0;

    return value;
}

static csize cvs_fixed_sized_maybe_n_children(CVariantSerialised value)
{
    csize element_fixed_size;

    c_variant_type_info_query_element(value.type_info, NULL, &element_fixed_size);

    return (element_fixed_size == value.size) ? 1 : 0;
}

static CVariantSerialised cvs_fixed_sized_maybe_get_child(CVariantSerialised value, csize index_)
{
    value.type_info = c_variant_type_info_element(value.type_info);
    c_variant_type_info_ref(value.type_info);
    value.depth++;
    value.ordered_offsets_up_to = 0;
    value.checked_offsets_up_to = 0;

    (void)index_;

    return value;
}

static csize cvs_fixed_sized_maybe_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                               const void **children, csize n_children)
{
    if (n_children) {
        csize element_fixed_size;
        c_variant_type_info_query_element(type_info, NULL, &element_fixed_size);
        return element_fixed_size;
    }

    return 0;
}

static void cvs_fixed_sized_maybe_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler,
                                            const void **children, csize n_children)
{
    if (n_children) {
        CVariantSerialised child = {NULL, value.data, value.size, value.depth + 1, 0, 0};
        gvs_filler(&child, (void *)children[0]);
    }
}

static bool cvs_fixed_sized_maybe_is_normal(CVariantSerialised value)
{
    if (value.size > 0) {
        csize element_fixed_size;
        c_variant_type_info_query_element(value.type_info, NULL, &element_fixed_size);

        if (value.size != element_fixed_size) {
            return false;
        }

        value.type_info = c_variant_type_info_element(value.type_info);
        value.depth++;
        value.ordered_offsets_up_to = 0;
        value.checked_offsets_up_to = 0;

        return c_variant_serialised_is_normal(value);
    }

    return true;
}

static csize cvs_variable_sized_maybe_n_children(CVariantSerialised value) { return (value.size > 0) ? 1 : 0; }

static CVariantSerialised cvs_variable_sized_maybe_get_child(CVariantSerialised value, csize index_)
{
    value.type_info = c_variant_type_info_element(value.type_info);
    c_variant_type_info_ref(value.type_info);
    value.size--;

    /* if it's zero-sized then it may as well be NULL */
    if (value.size == 0) {
        value.data = NULL;
    }

    value.depth++;
    value.ordered_offsets_up_to = 0;
    value.checked_offsets_up_to = 0;

    return value;
}

static csize cvs_variable_sized_maybe_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                                  const void **children, csize n_children)
{
    if (n_children) {
        CVariantSerialised child = {
            0,
        };
        gvs_filler(&child, (void *)children[0]);

        return child.size + 1;
    }

    return 0;
}

static void cvs_variable_sized_maybe_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler,
                                               const void **children, csize n_children)
{
    if (n_children) {
        CVariantSerialised child = {NULL, value.data, value.size - 1, value.depth + 1, 0, 0};
        gvs_filler(&child, (void *)children[0]);
        value.data[child.size] = '\0';
    }
}

static bool cvs_variable_sized_maybe_is_normal(CVariantSerialised value)
{
    if (value.size == 0) {
        return true;
    }

    if (value.data[value.size - 1] != '\0') {
        return false;
    }

    value.type_info = c_variant_type_info_element(value.type_info);
    value.size--;
    value.depth++;
    value.ordered_offsets_up_to = 0;
    value.checked_offsets_up_to = 0;

    return c_variant_serialised_is_normal(value);
}

static csize cvs_fixed_sized_array_n_children(CVariantSerialised value)
{
    csize element_fixed_size;

    c_variant_type_info_query_element(value.type_info, NULL, &element_fixed_size);

    if (value.size % element_fixed_size == 0) {
        return value.size / element_fixed_size;
    }

    return 0;
}

static CVariantSerialised cvs_fixed_sized_array_get_child(CVariantSerialised value, csize index_)
{
    CVariantSerialised child = {
        0,
    };

    child.type_info = c_variant_type_info_element(value.type_info);
    c_variant_type_info_query(child.type_info, NULL, &child.size);
    child.data = value.data + (child.size * index_);
    c_variant_type_info_ref(child.type_info);
    child.depth = value.depth + 1;

    return child;
}

static csize cvs_fixed_sized_array_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                               const void **children, csize n_children)
{
    csize element_fixed_size;

    c_variant_type_info_query_element(type_info, NULL, &element_fixed_size);

    return element_fixed_size * n_children;
}

static void cvs_fixed_sized_array_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler,
                                            const void **children, csize n_children)
{
    CVariantSerialised child = {
        0,
    };
    csize i;

    child.type_info = c_variant_type_info_element(value.type_info);
    c_variant_type_info_query(child.type_info, NULL, &child.size);
    child.data = value.data;
    child.depth = value.depth + 1;

    for (i = 0; i < n_children; i++) {
        gvs_filler(&child, (void *)children[i]);
        child.data += child.size;
    }
}

static bool cvs_fixed_sized_array_is_normal(CVariantSerialised value)
{
    CVariantSerialised child = {
        0,
    };

    child.type_info = c_variant_type_info_element(value.type_info);
    c_variant_type_info_query(child.type_info, NULL, &child.size);
    child.depth = value.depth + 1;

    if (value.size % child.size != 0) {
        return false;
    }

    for (child.data = value.data; child.data < value.data + value.size; child.data += child.size) {
        if (!c_variant_serialised_is_normal(child)) {
            return false;
        }
    }

    return true;
}

static cuint cvs_get_offset_size(csize size)
{
    if (size > C_MAX_UINT32) {
        return 8;
    }
    else if (size > C_MAX_UINT16) {
        return 4;
    }
    else if (size > C_MAX_UINT8) {
        return 2;
    }
    else if (size > 0) {
        return 1;
    }

    return 0;
}

static csize cvs_calculate_total_size(csize body_size, csize offsets)
{
    if (body_size + 1 * offsets <= C_MAX_UINT8) {
        return body_size + 1 * offsets;
    }

    if (body_size + 2 * offsets <= C_MAX_UINT16) {
        return body_size + 2 * offsets;
    }

    if (body_size + 4 * offsets <= C_MAX_UINT32) {
        return body_size + 4 * offsets;
    }

    return body_size + 8 * offsets;
}

static csize cvs_offsets_get_offset_n(struct Offsets *offsets, csize n)
{
    return cvs_read_unaligned_le(offsets->array + (offsets->offset_size * n), offsets->offset_size);
}

static struct Offsets cvs_variable_sized_array_get_frame_offsets(CVariantSerialised value)
{
    struct Offsets out = {
        0,
    };
    csize offsets_array_size;
    csize last_end;

    if (value.size == 0) {
        out.is_normal = true;
        return out;
    }

    out.offset_size = cvs_get_offset_size(value.size);
    last_end = cvs_read_unaligned_le(value.data + value.size - out.offset_size, out.offset_size);

    if (last_end > value.size) {
        return out; /* offsets not normal */
    }

    offsets_array_size = value.size - last_end;

    if (offsets_array_size % out.offset_size) {
        return out; /* offsets not normal */
    }

    out.data_size = last_end;
    out.array = value.data + last_end;
    out.length = offsets_array_size / out.offset_size;

    if (out.length > 0 && cvs_calculate_total_size(last_end, out.length) != value.size) {
        return out; /* offset size not minimal */
    }

    out.is_normal = true;

    return out;
}

static csize cvs_variable_sized_array_n_children(CVariantSerialised value)
{
    return cvs_variable_sized_array_get_frame_offsets(value).length;
}


#define DEFINE_FIND_UNORDERED(type, le_to_native)                                                                      \
    static csize find_unordered_##type(const cuint8 *data, csize start, csize len)                                     \
    {                                                                                                                  \
        csize off;                                                                                                     \
        type current_le, previous_le, current, previous;                                                               \
                                                                                                                       \
        memcpy(&previous_le, data + start * sizeof(current), sizeof(current));                                         \
        previous = le_to_native(previous_le);                                                                          \
        for (off = (start + 1) * sizeof(current); off < len * sizeof(current); off += sizeof(current)) {               \
            memcpy(&current_le, data + off, sizeof(current));                                                          \
            current = le_to_native(current_le);                                                                        \
            if (current < previous) {                                                                                  \
                break;                                                                                                 \
            }                                                                                                          \
            previous = current;                                                                                        \
        }                                                                                                              \
        return off / sizeof(current) - 1;                                                                              \
    }

#define NO_CONVERSION(x) (x)
DEFINE_FIND_UNORDERED(cuint8, NO_CONVERSION);
DEFINE_FIND_UNORDERED(cuint16, C_UINT16_FROM_LE);
DEFINE_FIND_UNORDERED(cuint32, C_UINT32_FROM_LE);
DEFINE_FIND_UNORDERED(cuint64, C_UINT64_FROM_LE);

static CVariantSerialised cvs_variable_sized_array_get_child(CVariantSerialised value, csize index_)
{
    CVariantSerialised child = {
        0,
    };

    struct Offsets offsets = cvs_variable_sized_array_get_frame_offsets(value);

    csize start;
    csize end;

    child.type_info = c_variant_type_info_element(value.type_info);
    c_variant_type_info_ref(child.type_info);
    child.depth = value.depth + 1;

    if (offsets.array != NULL && index_ > value.checked_offsets_up_to &&
        value.ordered_offsets_up_to == value.checked_offsets_up_to) {
        switch (offsets.offset_size) {
        case 1: {
            value.ordered_offsets_up_to = find_unordered_cuint8(offsets.array, value.checked_offsets_up_to, index_ + 1);
            break;
        }
        case 2: {
            value.ordered_offsets_up_to =
                find_unordered_cuint16(offsets.array, value.checked_offsets_up_to, index_ + 1);
            break;
        }
        case 4: {
            value.ordered_offsets_up_to =
                find_unordered_cuint32(offsets.array, value.checked_offsets_up_to, index_ + 1);
            break;
        }
        case 8: {
            value.ordered_offsets_up_to =
                find_unordered_cuint64(offsets.array, value.checked_offsets_up_to, index_ + 1);
            break;
        }
        default: {
            c_assert_not_reached();
        }
        }
        value.checked_offsets_up_to = index_;
    }

    if (index_ > value.ordered_offsets_up_to) {
        return child;
    }

    if (index_ > 0) {
        cuint alignment;
        start = cvs_offsets_get_offset_n(&offsets, index_ - 1);
        c_variant_type_info_query(child.type_info, &alignment, NULL);
        start += (-start) & alignment;
    }
    else {
        start = 0;
    }

    end = cvs_offsets_get_offset_n(&offsets, index_);

    if (start < end && end <= value.size && end <= offsets.data_size) {
        child.data = value.data + start;
        child.size = end - start;
    }

    return child;
}

static csize cvs_variable_sized_array_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                                  const void **children, csize n_children)
{
    cuint alignment;
    csize offset;
    csize i;

    c_variant_type_info_query(type_info, &alignment, NULL);
    offset = 0;

    for (i = 0; i < n_children; i++) {
        CVariantSerialised child = {
            0,
        };
        offset += (-offset) & alignment;
        gvs_filler(&child, (void *)children[i]);
        offset += child.size;
    }

    return cvs_calculate_total_size(offset, n_children);
}

static void cvs_variable_sized_array_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler,
                                               const void **children, csize n_children)
{
    cuchar *offset_ptr;
    csize offset_size;
    cuint alignment;
    csize offset;
    csize i;

    c_variant_type_info_query(value.type_info, &alignment, NULL);
    offset_size = cvs_get_offset_size(value.size);
    offset = 0;

    offset_ptr = value.data + value.size - offset_size * n_children;

    for (i = 0; i < n_children; i++) {
        CVariantSerialised child = {
            0,
        };
        while (offset & alignment) {
            value.data[offset++] = '\0';
        }

        child.data = value.data + offset;
        gvs_filler(&child, (void *)children[i]);
        offset += child.size;

        cvs_write_unaligned_le(offset_ptr, offset, offset_size);
        offset_ptr += offset_size;
    }
}

static bool cvs_variable_sized_array_is_normal(CVariantSerialised value)
{
    CVariantSerialised child = {
        0,
    };
    cuint alignment;
    csize offset;
    csize i;

    struct Offsets offsets = cvs_variable_sized_array_get_frame_offsets(value);
    if (!offsets.is_normal) {
        return false;
    }

    if (value.size != 0 && offsets.length == 0) {
        return false;
    }

    c_assert(value.size != 0 || offsets.length == 0);

    child.type_info = c_variant_type_info_element(value.type_info);
    c_variant_type_info_query(child.type_info, &alignment, NULL);
    child.depth = value.depth + 1;
    offset = 0;

    for (i = 0; i < offsets.length; i++) {
        csize this_end;
        this_end = cvs_read_unaligned_le(offsets.array + offsets.offset_size * i, offsets.offset_size);

        if (this_end < offset || this_end > offsets.data_size) {
            return false;
        }

        while (offset & alignment) {
            if (!(offset < this_end && value.data[offset] == '\0')) {
                return false;
            }
            offset++;
        }

        child.data = value.data + offset;
        child.size = this_end - offset;

        if (child.size == 0) {
            child.data = NULL;
        }

        if (!c_variant_serialised_is_normal(child)) {
            return false;
        }
        offset = this_end;
    }

    c_assert(offset == offsets.data_size);

    /* All offsets have now been checked. */
    value.ordered_offsets_up_to = C_MAX_SIZE;
    value.checked_offsets_up_to = C_MAX_SIZE;

    return true;
}

static void cvs_tuple_get_member_bounds(CVariantSerialised value, csize index_, csize offset_size,
                                        csize *out_member_start, csize *out_member_end)
{
    const CVariantMemberInfo *member_info;
    csize member_start, member_end;

    member_info = c_variant_type_info_member_info(value.type_info, index_);

    if (member_info->i + 1 && offset_size * (member_info->i + 1) <= value.size) {
        member_start = cvs_read_unaligned_le(value.data + value.size - offset_size * (member_info->i + 1), offset_size);
    }
    else {
        member_start = 0;
    }

    member_start += member_info->a;
    member_start &= member_info->b;
    member_start |= member_info->c;

    if (member_info->ending_type == C_VARIANT_MEMBER_ENDING_LAST && offset_size * (member_info->i + 1) <= value.size) {
        member_end = value.size - offset_size * (member_info->i + 1);
    }
    else if (member_info->ending_type == C_VARIANT_MEMBER_ENDING_FIXED) {
        csize fixed_size;

        c_variant_type_info_query(member_info->type_info, NULL, &fixed_size);
        member_end = member_start + fixed_size;
    }
    else if (member_info->ending_type == C_VARIANT_MEMBER_ENDING_OFFSET &&
             offset_size * (member_info->i + 2) <= value.size) {
        member_end = cvs_read_unaligned_le(value.data + value.size - offset_size * (member_info->i + 2), offset_size);
    }
    else {
        member_end = C_MAX_SIZE;
    }

    if (out_member_start != NULL)
        *out_member_start = member_start;
    if (out_member_end != NULL)
        *out_member_end = member_end;
}

static csize cvs_tuple_n_children(CVariantSerialised value) { return c_variant_type_info_n_members(value.type_info); }

static CVariantSerialised cvs_tuple_get_child(CVariantSerialised value, csize index_)
{
    const CVariantMemberInfo *member_info;
    CVariantSerialised child = {
        0,
    };
    csize offset_size;
    csize start, end, last_end;

    member_info = c_variant_type_info_member_info(value.type_info, index_);
    child.type_info = c_variant_type_info_ref(member_info->type_info);
    child.depth = value.depth + 1;
    offset_size = cvs_get_offset_size(value.size);

    if (member_info->ending_type == C_VARIANT_MEMBER_ENDING_FIXED)
        c_variant_type_info_query(child.type_info, NULL, &child.size);

    if C_UNLIKELY (value.data == NULL && value.size != 0) {
        c_assert(child.size != 0);
        child.data = NULL;

        return child;
    }

    if (index_ > value.checked_offsets_up_to && value.ordered_offsets_up_to == value.checked_offsets_up_to) {
        csize i, prev_i_end = 0;

        if (value.checked_offsets_up_to > 0)
            cvs_tuple_get_member_bounds(value, value.checked_offsets_up_to - 1, offset_size, NULL, &prev_i_end);

        for (i = value.checked_offsets_up_to; i <= index_; i++) {
            csize i_start, i_end;

            cvs_tuple_get_member_bounds(value, i, offset_size, &i_start, &i_end);

            if (i_start > i_end || i_start < prev_i_end || i_end > value.size)
                break;

            prev_i_end = i_end;
        }

        value.ordered_offsets_up_to = i - 1;
        value.checked_offsets_up_to = index_;
    }

    if (index_ > value.ordered_offsets_up_to) {
        return child;
    }

    if (member_info->ending_type == C_VARIANT_MEMBER_ENDING_OFFSET) {
        if (offset_size * (member_info->i + 2) > value.size)
            return child;
    }
    else {
        if (offset_size * (member_info->i + 1) > value.size)
            return child;
    }

    /* The child should not extend into the offset table. */
    cvs_tuple_get_member_bounds(value, index_, offset_size, &start, &end);
    cvs_tuple_get_member_bounds(value, c_variant_type_info_n_members(value.type_info) - 1, offset_size, NULL,
                                &last_end);

    if (start < end && end <= value.size && end <= last_end) {
        child.data = value.data + start;
        child.size = end - start;
    }

    return child;
}

static csize cvs_tuple_needed_size(CVariantTypeInfo *type_info, CVariantSerialisedFiller gvs_filler,
                                   const void **children, csize n_children)
{
    const CVariantMemberInfo *member_info = NULL;
    csize fixed_size;
    csize offset;
    csize i;

    c_variant_type_info_query(type_info, NULL, &fixed_size);

    if (fixed_size)
        return fixed_size;

    offset = 0;

    c_assert(n_children > 0);

    for (i = 0; i < n_children; i++) {
        cuint alignment;

        member_info = c_variant_type_info_member_info(type_info, i);
        c_variant_type_info_query(member_info->type_info, &alignment, &fixed_size);
        offset += (-offset) & alignment;

        if (fixed_size)
            offset += fixed_size;
        else {
            CVariantSerialised child = {
                0,
            };

            gvs_filler(&child, (void *)children[i]);
            offset += child.size;
        }
    }

    return cvs_calculate_total_size(offset, member_info->i + 1);
}

static void cvs_tuple_serialise(CVariantSerialised value, CVariantSerialisedFiller gvs_filler, const void **children,
                                csize n_children)
{
    csize offset_size;
    csize offset;
    csize i;

    offset_size = cvs_get_offset_size(value.size);
    offset = 0;

    for (i = 0; i < n_children; i++) {
        const CVariantMemberInfo *member_info;
        CVariantSerialised child = {
            0,
        };
        cuint alignment;

        member_info = c_variant_type_info_member_info(value.type_info, i);
        c_variant_type_info_query(member_info->type_info, &alignment, NULL);

        while (offset & alignment)
            value.data[offset++] = '\0';

        child.data = value.data + offset;
        gvs_filler(&child, (void *)children[i]);
        offset += child.size;

        if (member_info->ending_type == C_VARIANT_MEMBER_ENDING_OFFSET) {
            value.size -= offset_size;
            cvs_write_unaligned_le(value.data + value.size, offset, offset_size);
        }
    }

    while (offset < value.size)
        value.data[offset++] = '\0';
}

static bool cvs_tuple_is_normal(CVariantSerialised value)
{
    cuint offset_size;
    csize offset_ptr;
    csize length;
    csize offset;
    csize i;
    csize offset_table_size;

    /* as per the comment in gvs_tuple_get_child() */
    if C_UNLIKELY (value.data == NULL && value.size != 0)
        return false;

    offset_size = cvs_get_offset_size(value.size);
    length = c_variant_type_info_n_members(value.type_info);
    offset_ptr = value.size;
    offset = 0;

    for (i = 0; i < length; i++) {
        const CVariantMemberInfo *member_info;
        CVariantSerialised child = {
            0,
        };
        csize fixed_size;
        cuint alignment;
        csize end;

        member_info = c_variant_type_info_member_info(value.type_info, i);
        child.type_info = member_info->type_info;
        child.depth = value.depth + 1;

        c_variant_type_info_query(child.type_info, &alignment, &fixed_size);

        while (offset & alignment) {
            if (offset > value.size || value.data[offset] != '\0') {
                return false;
            }
            offset++;
        }

        child.data = value.data + offset;

        switch (member_info->ending_type) {
        case C_VARIANT_MEMBER_ENDING_FIXED:
            end = offset + fixed_size;
            break;
        case C_VARIANT_MEMBER_ENDING_LAST:
            end = offset_ptr;
            break;
        case C_VARIANT_MEMBER_ENDING_OFFSET:
            if (offset_ptr < offset_size)
                return false;
            offset_ptr -= offset_size;
            if (offset_ptr < offset)
                return false;
            end = cvs_read_unaligned_le(value.data + offset_ptr, offset_size);
            break;
        default:
            c_assert_not_reached();
        }

        if (end < offset || end > offset_ptr)
            return false;

        child.size = end - offset;

        if (child.size == 0)
            child.data = NULL;

        if (!c_variant_serialised_is_normal(child)) {
            return false;
        }
        offset = end;
    }

    value.ordered_offsets_up_to = C_MAX_SIZE;
    value.checked_offsets_up_to = C_MAX_SIZE;

    {
        csize fixed_size;
        cuint alignment;

        c_variant_type_info_query(value.type_info, &alignment, &fixed_size);
        if (fixed_size) {
            c_assert(fixed_size == value.size);
            c_assert(offset_ptr == value.size);

            if (i == 0) {
                if (value.data[offset++] != '\0')
                    return false;
            }
            else {
                while (offset & alignment)
                    if (value.data[offset++] != '\0')
                        return false;
            }
            c_assert(offset == value.size);
        }
    }

    if (offset_ptr != offset)
        return false;

    offset_table_size = value.size - offset_ptr;
    if (value.size > 0 && cvs_calculate_total_size(offset, offset_table_size / offset_size) != value.size) {
        return false; /* offset size not minimal */
    }

    return true;
}

static bool ensure_valid_dict(CVariantDict *dict)
{
    if (dict == NULL) {
        return false;
    }
    else if (is_valid_dict(dict)) {
        return true;
    }

    if (dict->u.s.partial_magic == CVSD_MAGIC_PARTIAL) {
        static CVariantDict cleared_dict;
        if (memcmp(cleared_dict.u.s.y, dict->u.s.y, sizeof cleared_dict.u.s.y)) {
            return false;
        }
        c_variant_dict_init(dict, dict->u.s.asv);
    }

    return is_valid_dict(dict);
}

static bool parse_num(const cchar *num, const cchar *limit, cuint *result)
{
    cchar *endptr;
    cint64 bignum;

    bignum = c_ascii_strtoll(num, &endptr, 10);
    if (endptr != limit) {
        return false;
    }

    if (bignum < 0 || bignum > C_MAX_INT32) {
        return false;
    }

    *result = (cuint)bignum;

    return true;
}

static void add_last_line(CString *err, const cchar *str)
{
    const cchar *last_nl;
    cchar *chomped;
    cint i;

    /* This is an error at the end of input.  If we have a file
     * with newlines, that's probably the empty string after the
     * last newline, which is not the most useful thing to show.
     *
     * Instead, show the last line of non-whitespace that we have
     * and put the pointer at the end of it.
     */
    chomped = c_strchomp(c_strdup(str));
    last_nl = strrchr(chomped, '\n');
    if (last_nl == NULL) {
        last_nl = chomped;
    }
    else {
        last_nl++;
    }

    /* Print the last line like so:
     *
     *   [1, 2, 3,
     *            ^
     */
    c_string_append(err, "  ");
    if (last_nl[0]) {
        c_string_append(err, last_nl);
    }
    else {
        c_string_append(err, "(empty input)");
    }
    c_string_append(err, "\n  ");
    for (i = 0; last_nl[i]; i++) {
        c_string_append_c(err, ' ');
    }
    c_string_append(err, "^\n");
    c_free(chomped);
}

static void add_lines_from_range(CString *err, const cchar *str, const cchar *start1, const cchar *end1,
                                 const cchar *start2, const cchar *end2)
{
    while (str < end1 || str < end2) {
        const cchar *nl;
        nl = str + strcspn(str, "\n");
        if ((start1 < nl && str < end1) || (start2 < nl && str < end2)) {
            const cchar *s;
            c_string_append(err, "  ");
            c_string_append_len(err, str, nl - str);
            c_string_append(err, "\n  ");

            /* And add underlines... */
            for (s = str; s < nl; s++) {
                if ((start1 <= s && s < end1) || (start2 <= s && s < end2)) {
                    c_string_append_c(err, '^');
                }
                else {
                    c_string_append_c(err, ' ');
                }
            }
            c_string_append_c(err, '\n');
        }

        if (!*nl) {
            break;
        }
        str = nl + 1;
    }
}

static AST *parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error)
{
    SourceRef source_ref;
    AST *result;

    if (max_depth == 0) {
        token_stream_set_error(stream, error, false, C_VARIANT_PARSE_ERROR_RECURSION, "variant nested too deeply");
        return NULL;
    }

    token_stream_prepare(stream);
    token_stream_start_ref(stream, &source_ref);

    if (token_stream_peek(stream, '[')) {
        result = array_parse(stream, max_depth, app, error);
    }
    else if (token_stream_peek(stream, '(')) {
        result = tuple_parse(stream, max_depth, app, error);
    }
    else if (token_stream_peek(stream, '<')) {
        result = variant_parse(stream, max_depth, app, error);
    }
    else if (token_stream_peek(stream, '{')) {
        result = dictionary_parse(stream, max_depth, app, error);
    }
    else if (app && token_stream_peek(stream, '%')) {
        result = positional_parse(stream, app, error);
    }
    else if (token_stream_consume(stream, "true")) {
        result = boolean_new(true);
    }
    else if (token_stream_consume(stream, "false")) {
        result = boolean_new(false);
    }
    else if (token_stream_is_numeric(stream) || token_stream_peek_string(stream, "inf") ||
             token_stream_peek_string(stream, "nan")) {
        result = number_parse(stream, app, error);
    }
    else if (token_stream_peek(stream, 'n') || token_stream_peek(stream, 'j')) {
        result = maybe_parse(stream, max_depth, app, error);
    }
    else if (token_stream_peek(stream, '@') || token_stream_is_keyword(stream)) {
        result = typedecl_parse(stream, max_depth, app, error);
    }
    else if (token_stream_peek(stream, '\'') || token_stream_peek(stream, '"')) {
        result = string_parse(stream, app, error);
    }
    else if (token_stream_peek2(stream, 'b', '\'') || token_stream_peek2(stream, 'b', '"')) {
        result = bytestring_parse(stream, app, error);
    }
    else {
        token_stream_set_error(stream, error, false, C_VARIANT_PARSE_ERROR_VALUE_EXPECTED, "expected value");
        return NULL;
    }

    if (result != NULL) {
        token_stream_end_ref(stream, &source_ref);
        result->source_ref = source_ref;
    }

    return result;
}

static void token_stream_set_error(TokenStream *stream, CError **error, bool this_token, cint code, const cchar *format,
                                   ...)
{
    SourceRef ref;
    va_list ap;

    ref.start = stream->this - stream->start;

    if (this_token)
        ref.end = stream->stream - stream->start;
    else
        ref.end = ref.start;

    va_start(ap, format);
    parser_set_error_va(error, &ref, NULL, code, format, ap);
    va_end(ap);
}

static bool token_stream_prepare(TokenStream *stream)
{
    cint brackets = 0;
    const cchar *end;

    if (stream->this != NULL)
        return true;

    while (stream->stream != stream->end && c_ascii_isspace(*stream->stream))
        stream->stream++;

    if (stream->stream == stream->end || *stream->stream == '\0') {
        stream->this = stream->stream;
        return false;
    }

    switch (stream->stream[0]) {
    case '-':
    case '+':
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        for (end = stream->stream; end != stream->end; end++) {
            if (!c_ascii_isalnum(*end) && *end != '-' && *end != '+' && *end != '.') {
                break;
            }
        }
        break;
    case 'b':
        if (stream->stream + 1 != stream->end && (stream->stream[1] == '\'' || stream->stream[1] == '"')) {
            for (end = stream->stream + 2; end != stream->end; end++) {
                if (*end == stream->stream[1] || *end == '\0' ||
                    (*end == '\\' && (++end == stream->end || *end == '\0'))) {
                    break;
                }
            }
            if (end != stream->end && *end) {
                end++;
            }
            break;
        }
        C_FALLTHROUGH;
    case 'a': /* 'b' */
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
        for (end = stream->stream; end != stream->end; end++)
            if (!c_ascii_isalnum(*end))
                break;
        break;

    case '\'':
    case '"':
        for (end = stream->stream + 1; end != stream->end; end++)
            if (*end == stream->stream[0] || *end == '\0' || (*end == '\\' && (++end == stream->end || *end == '\0')))
                break;
        if (end != stream->end && *end)
            end++;
        break;
    case '@':
    case '%':
        for (end = stream->stream + 1; end != stream->end && *end != '\0' && *end != ',' && *end != ':' &&
             *end != '>' && *end != ']' && !c_ascii_isspace(*end);
             end++)
            if (*end == '(' || *end == '{')
                brackets++;
            else if ((*end == ')' || *end == '}') && !brackets--)
                break;
        break;
    default:
        end = stream->stream + 1;
        break;
    }

    stream->this = stream->stream;
    stream->stream = end;

    /* We must have at least one byte in a token. */
    c_assert(stream->stream - stream->this >= 1);

    return true;
}

static void token_stream_next(TokenStream *stream) { stream->this = NULL; }

static bool token_stream_peek(TokenStream *stream, cchar first_char)
{
    if (!token_stream_prepare(stream))
        return false;

    return stream->stream - stream->this >= 1 && stream->this[0] == first_char;
}

static bool token_stream_peek2(TokenStream *stream, cchar first_char, cchar second_char)
{
    if (!token_stream_prepare(stream))
        return false;

    return stream->stream - stream->this >= 2 && stream->this[0] == first_char && stream->this[1] == second_char;
}

static bool token_stream_is_keyword(TokenStream *stream)
{
    if (!token_stream_prepare(stream))
        return false;

    return stream->stream - stream->this >= 2 && c_ascii_isalpha(stream->this[0]) && c_ascii_isalpha(stream->this[1]);
}

static bool token_stream_is_numeric(TokenStream *stream)
{
    if (!token_stream_prepare(stream)) {
        return false;
    }

    return (stream->stream - stream->this >= 1 &&
            (c_ascii_isdigit(stream->this[0]) || stream->this[0] == '-' || stream->this[0] == '+' ||
             stream->this[0] == '.'));
}

static bool token_stream_peek_string(TokenStream *stream, const cchar *token)
{
    size_t length = strlen(token);

    return token_stream_prepare(stream) && (size_t)(stream->stream - stream->this) == length &&
        memcmp(stream->this, token, length) == 0;
}

static bool token_stream_consume(TokenStream *stream, const cchar *token)
{
    if (!token_stream_peek_string(stream, token)) {
        return false;
    }

    token_stream_next(stream);
    return true;
}

static bool token_stream_require(TokenStream *stream, const cchar *token, const cchar *purpose, CError **error)
{
    if (!token_stream_consume(stream, token)) {
        token_stream_set_error(stream, error, false, C_VARIANT_PARSE_ERROR_UNEXPECTED_TOKEN, "expected '%s'%s", token,
                               purpose);
        return false;
    }

    return true;
}

static void token_stream_assert(TokenStream *stream, const cchar *token)
{
    bool correct_token C_UNUSED /* when compiling with G_DISABLE_ASSERT */;

    correct_token = token_stream_consume(stream, token);
    c_assert(correct_token);
}

static cchar *token_stream_get(TokenStream *stream)
{
    cchar *result;

    if (!token_stream_prepare(stream)) {
        return NULL;
    }

    result = c_strndup(stream->this, stream->stream - stream->this);

    return result;
}

static void token_stream_start_ref(TokenStream *stream, SourceRef *ref)
{
    token_stream_prepare(stream);
    ref->start = stream->this - stream->start;
}

static void token_stream_end_ref(TokenStream *stream, SourceRef *ref) { ref->end = stream->stream - stream->start; }

/* This is guaranteed to write exactly as many bytes to `out` as it consumes
 * from `in`. i.e. The `out` buffer doesn’t need to be any longer than `in`. */
static void pattern_copy(cchar **out, const cchar **in)
{
    cint brackets = 0;

    while (**in == 'a' || **in == 'm' || **in == 'M')
        *(*out)++ = *(*in)++;

    do {
        if (**in == '(' || **in == '{')
            brackets++;

        else if (**in == ')' || **in == '}')
            brackets--;

        *(*out)++ = *(*in)++;
    }
    while (brackets);
}

/* Returns the most general pattern that is subpattern of left and subpattern
 * of right, or NULL if there is no such pattern. */
static cchar *pattern_coalesce(const cchar *left, const cchar *right)
{
    cchar *result;
    cchar *out;
    size_t buflen;
    size_t left_len = strlen(left), right_len = strlen(right);

    /* the length of the output is loosely bound by the sum of the input
     * lengths, not simply the greater of the two lengths.
     *
     *   (*(iii)) + ((iii)*) = ((iii)(iii))
     *
     *      8     +    8     = 12
     *
     * This can be proven by the fact that `out` is never incremented by more
     * bytes than are consumed from `left` or `right` in each iteration.
     */
    c_assert(left_len < C_MAX_SIZE - right_len);
    buflen = left_len + right_len + 1;
    out = result = c_malloc0(buflen);

    while (*left && *right) {
        if (*left == *right) {
            *out++ = *left++;
            right++;
        }

        else {
            const cchar **one = &left, **the_other = &right;

        again:
            if (**one == '*' && **the_other != ')') {
                pattern_copy(&out, the_other);
                (*one)++;
            }

            else if (**one == 'M' && **the_other == 'm') {
                *out++ = *(*the_other)++;
            }

            else if (**one == 'M' && **the_other != 'm' && **the_other != '*') {
                (*one)++;
            }

            else if (**one == 'N' && strchr("ynqiuxthd", **the_other)) {
                *out++ = *(*the_other)++;
                (*one)++;
            }

            else if (**one == 'S' && strchr("sog", **the_other)) {
                *out++ = *(*the_other)++;
                (*one)++;
            }

            else if (one == &left) {
                one = &right, the_other = &left;
                goto again;
            }

            else
                break;
        }
    }

    /* Need at least one byte remaining for trailing nul. */
    c_assert(out < result + buflen);

    if (*left || *right) {
        c_free(result);
        result = NULL;
    }
    else
        *out++ = '\0';

    return result;
}

// array
static void array_free(AST *ast)
{
    Array *array = (Array *)ast;

    ast_array_free(array->children, array->n_children);
    c_free0(array);
}

static CVariant *array_get_value(AST *ast, const CVariantType *type, CError **error)
{
    Array *array = (Array *)ast;
    const CVariantType *childtype;
    CVariantBuilder builder;
    cint i;

    if (!c_variant_type_is_array(type))
        return ast_type_error(ast, type, error);

    c_variant_builder_init_static(&builder, type);
    childtype = c_variant_type_element(type);

    for (i = 0; i < array->n_children; i++) {
        CVariant *child;
        if (!(child = ast_get_value(array->children[i], childtype, error))) {
            c_variant_builder_clear(&builder);
            return NULL;
        }
        c_variant_builder_add_value(&builder, child);
    }

    return c_variant_builder_end(&builder);
}

static cchar *array_get_pattern(AST *ast, CError **error)
{
    Array *array = (Array *)ast;
    cchar *pattern;
    cchar *result;

    if (array->n_children == 0)
        return c_strdup("Ma*");

    pattern = ast_array_get_pattern(array->children, array->n_children, error);

    if (pattern == NULL)
        return NULL;

    result = c_strdup_printf("Ma%s", pattern);
    c_free(pattern);

    return result;
}

static AST *array_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error)
{
    static const ASTClass array_class = {array_get_pattern, maybe_wrapper, array_get_value, array_free};
    bool need_comma = false;
    Array *array;

    array = c_malloc0(sizeof(Array));
    array->ast.class = &array_class;
    array->children = NULL;
    array->n_children = 0;

    token_stream_assert(stream, "[");
    while (!token_stream_consume(stream, "]")) {
        AST *child;
        if (need_comma && !token_stream_require(stream, ",", " or ']' to follow array element", error)) {
            goto error;
        }
        child = parse(stream, max_depth - 1, app, error);
        if (!child) {
            goto error;
        }
        ast_array_append(&array->children, &array->n_children, child);
        need_comma = true;
    }

    return (AST *)array;

error:
    ast_array_free(array->children, array->n_children);
    c_free0(array);

    return NULL;
}

static CVariantType *c_variant_make_array_type(CVariant *element)
{
    return c_variant_type_new_array(c_variant_get_type(element));
}

// AST
static void ast_array_append(AST ***array, cint *n_items, AST *ast)
{
    if ((*n_items & (*n_items - 1)) == 0) {
        *array = c_realloc(*array, sizeof(AST *) * ((*n_items ? 2 * *n_items : 1)));
    }

    (*array)[(*n_items)++] = ast;
}

static void ast_array_free(AST **array, cint n_items)
{
    cint i;

    for (i = 0; i < n_items; i++) {
        ast_free(array[i]);
    }

    c_free(array);
}

static cchar *ast_array_get_pattern(AST **array, cint n_items, CError **error)
{
    cchar *pattern;
    cint i;

    /**
     * Find the pattern which applies to all children in the array, by l-folding a
     * coalesce operation.
     */
    pattern = ast_get_pattern(array[0], error);

    if (pattern == NULL)
        return NULL;

    for (i = 1; i < n_items; i++) {
        cchar *tmp, *merged;
        tmp = ast_get_pattern(array[i], error);
        if (tmp == NULL) {
            c_free(pattern);
            return NULL;
        }

        merged = pattern_coalesce(pattern, tmp);
        c_free(pattern);
        pattern = merged;

        if (merged == NULL) {
            int j = 0;
            while (true) {
                cchar *tmp2;
                cchar *m;
                if (j >= i) {
                    ast_set_error(array[i], error, NULL, C_VARIANT_PARSE_ERROR_NO_COMMON_TYPE,
                                  "unable to find a common type");
                    c_free(tmp);
                    return NULL;
                }
                tmp2 = ast_get_pattern(array[j], NULL);
                c_assert(tmp2 != NULL);
                m = pattern_coalesce(tmp, tmp2);
                c_free(tmp2);
                c_free(m);
                if (m == NULL) {
                    ast_set_error(array[j], error, array[i], C_VARIANT_PARSE_ERROR_NO_COMMON_TYPE,
                                  "unable to find a common type");
                    c_free(tmp);
                    return NULL;
                }
                j++;
            }
        }
        c_free(tmp);
    }

    return pattern;
}

static cchar *ast_get_pattern(AST *ast, CError **error) { return ast->class->get_pattern(ast, error); }

static CVariant *ast_get_value(AST *ast, const CVariantType *type, CError **error)
{
    return ast->class->get_value(ast, type, error);
}

static void ast_free(AST *ast) { ast->class->free(ast); }

static void ast_set_error(AST *ast, CError **error, AST *other_ast, cint code, const cchar *format, ...)
{
    va_list ap;

    va_start(ap, format);
    parser_set_error_va(error, &ast->source_ref, other_ast ? &other_ast->source_ref : NULL, code, format, ap);
    va_end(ap);
}

static CVariant *ast_type_error(AST *ast, const CVariantType *type, CError **error)
{
    cchar *typestr;

    typestr = c_variant_type_dup_string(type);
    ast_set_error(ast, error, NULL, C_VARIANT_PARSE_ERROR_TYPE_ERROR, "can not parse as value of type '%s'", typestr);
    c_free(typestr);

    return NULL;
}

static CVariant *ast_resolve(AST *ast, CError **error)
{
    CVariant *value;
    cchar *pattern;
    cint i, j = 0;

    pattern = ast_get_pattern(ast, error);

    if (pattern == NULL)
        return NULL;

    /* choose reasonable defaults
     *
     *   1) favour non-maybe values where possible
     *   2) default type for strings is 's'
     *   3) default type for integers is 'i'
     */
    for (i = 0; pattern[i]; i++)
        switch (pattern[i]) {
        case '*':
            ast_set_error(ast, error, NULL, C_VARIANT_PARSE_ERROR_CANNOT_INFER_TYPE, "unable to infer type");
            c_free(pattern);
            return NULL;

        case 'M':
            break;

        case 'S':
            pattern[j++] = 's';
            break;

        case 'N':
            pattern[j++] = 'i';
            break;

        default:
            pattern[j++] = pattern[i];
            break;
        }
    pattern[j++] = '\0';

    value = ast_get_value(ast, C_VARIANT_TYPE(pattern), error);
    c_free(pattern);

    return value;
}

static void parser_set_error(CError **error, SourceRef *location, SourceRef *other, cint code, const cchar *format, ...)
{
    va_list ap;

    va_start(ap, format);
    parser_set_error_va(error, location, other, code, format, ap);
    va_end(ap);
}

static void parser_set_error_va(CError **error, SourceRef *location, SourceRef *other, cint code, const cchar *format,
                                va_list ap)
{
    CString *msg = c_string_new(NULL);

    if (location->start == location->end)
        c_string_append_printf(msg, "%d", location->start);
    else
        c_string_append_printf(msg, "%d-%d", location->start, location->end);

    if (other != NULL) {
        c_assert(other->start != other->end);
        c_string_append_printf(msg, ",%d-%d", other->start, other->end);
    }
    c_string_append_c(msg, ':');

    c_string_append_vprintf(msg, format, ap);
    c_set_error_literal(error, C_VARIANT_PARSE_ERROR, code, msg->str);
    c_string_free(msg, true);
}


// maybe
static CVariant *maybe_wrapper(AST *ast, const CVariantType *type, CError **error)
{
    const CVariantType *base_type;
    CVariant *base_value;
    CVariant *value = NULL;
    unsigned int depth;
    bool trusted;
    CVariantTypeInfo *base_type_info = NULL;
    csize base_serialised_fixed_size, base_serialised_size, serialised_size, n_suffix_zeros;
    cuint8 *serialised = NULL;
    CBytes *bytes = NULL;
    csize i;

    for (depth = 0, base_type = type; c_variant_type_is_maybe(base_type);
         depth++, base_type = c_variant_type_element(base_type))
        ;

    base_value = ast->class->get_base_value(ast, base_type, error);

    if (base_value == NULL || depth == 0) {
        return c_steal_pointer(&base_value);
    }

    trusted = c_variant_is_trusted(base_value);

    base_type_info = c_variant_type_info_get(base_type);
    c_variant_type_info_query(base_type_info, NULL, &base_serialised_fixed_size);
    c_variant_type_info_unref(base_type_info);

    base_serialised_size = c_variant_get_size(base_value);
    n_suffix_zeros = (base_serialised_fixed_size > 0) ? depth - 1 : depth;
    c_assert(base_serialised_size <= C_MAX_SIZE - n_suffix_zeros);
    serialised_size = base_serialised_size + n_suffix_zeros;

    c_assert(serialised_size >= base_serialised_size);

    /* Serialise the base value. */
    serialised = c_malloc0(serialised_size);
    c_variant_store(base_value, serialised);

    /* Zero-out the suffix zeros to complete the serialisation of the maybe wrappers. */
    for (i = base_serialised_size; i < serialised_size; i++) {
        serialised[i] = 0;
    }

    bytes = c_bytes_new_take(c_steal_pointer(&serialised), serialised_size);
    value = c_variant_new_from_bytes(type, bytes, trusted);
    c_bytes_unref(bytes);

    c_variant_unref(base_value);

    return c_steal_pointer(&value);
}

static cchar *maybe_get_pattern(AST *ast, CError **error)
{
    Maybe *maybe = (Maybe *)ast;

    if (maybe->child != NULL) {
        cchar *child_pattern;
        cchar *pattern;
        child_pattern = ast_get_pattern(maybe->child, error);
        if (child_pattern == NULL) {
            return NULL;
        }
        pattern = c_strdup_printf("m%s", child_pattern);
        c_free(child_pattern);
        return pattern;
    }

    return c_strdup("m*");
}

static CVariant *maybe_get_value(AST *ast, const CVariantType *type, CError **error)
{
    Maybe *maybe = (Maybe *)ast;
    CVariant *value;

    if (!c_variant_type_is_maybe(type)) {
        return ast_type_error(ast, type, error);
    }

    type = c_variant_type_element(type);

    if (maybe->child) {
        value = ast_get_value(maybe->child, type, error);
        if (value == NULL) {
            return NULL;
        }
    }
    else {
        value = NULL;
    }

    return c_variant_new_maybe(type, value);
}

static void maybe_free(AST *ast)
{
    Maybe *maybe = (Maybe *)ast;

    if (maybe->child != NULL) {
        ast_free(maybe->child);
    }

    c_free0(maybe);
}

static AST *maybe_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error)
{
    static const ASTClass maybe_class = {maybe_get_pattern, maybe_get_value, NULL, maybe_free};

    AST *child = NULL;
    Maybe *maybe;

    if (token_stream_consume(stream, "just")) {
        child = parse(stream, max_depth - 1, app, error);
        if (child == NULL) {
            return NULL;
        }
    }
    else if (!token_stream_consume(stream, "nothing")) {
        token_stream_set_error(stream, error, true, C_VARIANT_PARSE_ERROR_UNKNOWN_KEYWORD, "unknown keyword");
        return NULL;
    }

    maybe = c_malloc0(sizeof(Maybe));
    maybe->ast.class = &maybe_class;
    maybe->child = child;

    return (AST *)maybe;
}

static CVariantType *c_variant_make_maybe_type(CVariant *element)
{
    return c_variant_type_new_maybe(c_variant_get_type(element));
}


// tuple
static cchar *tuple_get_pattern(AST *ast, CError **error)
{
    Tuple *tuple = (Tuple *)ast;
    cchar *result = NULL;
    cchar **parts;
    cint i;

    parts = c_malloc0(sizeof(cchar *) * (tuple->n_children + 4));
    parts[tuple->n_children + 1] = (cchar *)")";
    parts[tuple->n_children + 2] = NULL;
    parts[0] = (cchar *)"M(";

    for (i = 0; i < tuple->n_children; i++) {
        if (!(parts[i + 1] = ast_get_pattern(tuple->children[i], error))) {
            break;
        }
    }

    if (i == tuple->n_children) {
        result = c_strjoinv("", parts);
    }

    /* parts[0] should not be freed */
    while (i) {
        c_free(parts[i--]);
    }
    c_free(parts);

    return result;
}

static CVariant *tuple_get_value(AST *ast, const CVariantType *type, CError **error)
{
    Tuple *tuple = (Tuple *)ast;
    const CVariantType *childtype;
    CVariantBuilder builder;
    cint i;

    if (!c_variant_type_is_tuple(type)) {
        return ast_type_error(ast, type, error);
    }

    c_variant_builder_init_static(&builder, type);
    childtype = c_variant_type_first(type);

    for (i = 0; i < tuple->n_children; i++) {
        CVariant *child;
        if (childtype == NULL) {
            c_variant_builder_clear(&builder);
            return ast_type_error(ast, type, error);
        }
        if (!(child = ast_get_value(tuple->children[i], childtype, error))) {
            c_variant_builder_clear(&builder);
            return false;
        }
        c_variant_builder_add_value(&builder, child);
        childtype = c_variant_type_next(childtype);
    }

    if (childtype != NULL) {
        c_variant_builder_clear(&builder);
        return ast_type_error(ast, type, error);
    }

    return c_variant_builder_end(&builder);
}

static void tuple_free(AST *ast)
{
    Tuple *tuple = (Tuple *)ast;

    ast_array_free(tuple->children, tuple->n_children);
    c_free0(tuple);
}

static AST *tuple_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error)
{
    static const ASTClass tuple_class = {tuple_get_pattern, maybe_wrapper, tuple_get_value, tuple_free};
    bool need_comma = false;
    bool first = true;
    Tuple *tuple;

    tuple = c_malloc0(sizeof(Tuple));
    tuple->ast.class = &tuple_class;
    tuple->children = NULL;
    tuple->n_children = 0;

    token_stream_assert(stream, "(");
    while (!token_stream_consume(stream, ")")) {
        AST *child;
        if (need_comma && !token_stream_require(stream, ",", " or ')' to follow tuple element", error)) {
            goto error;
        }
        child = parse(stream, max_depth - 1, app, error);
        if (!child) {
            goto error;
        }
        ast_array_append(&tuple->children, &tuple->n_children, child);
        if (first) {
            if (!token_stream_require(stream, ",", " after first tuple element", error)) {
                goto error;
            }
            first = false;
        }
        else {
            need_comma = true;
        }
    }

    return (AST *)tuple;

error:
    ast_array_free(tuple->children, tuple->n_children);
    c_free0(tuple);

    return NULL;
}

static cchar *variant_get_pattern(AST *ast, CError **error) { return c_strdup("Mv"); }

static CVariant *variant_get_value(AST *ast, const CVariantType *type, CError **error)
{
    Variant *variant = (Variant *)ast;
    CVariant *child;

    if (!c_variant_type_equal(type, C_VARIANT_TYPE_VARIANT)) {
        return ast_type_error(ast, type, error);
    }

    child = ast_resolve(variant->value, error);

    if (child == NULL) {
        return NULL;
    }

    return c_variant_new_variant(child);
}

static void variant_free(AST *ast)
{
    Variant *variant = (Variant *)ast;

    ast_free(variant->value);
    c_free0(variant);
}

static AST *variant_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error)
{
    static const ASTClass variant_class = {variant_get_pattern, maybe_wrapper, variant_get_value, variant_free};
    Variant *variant;
    AST *value;

    token_stream_assert(stream, "<");
    value = parse(stream, max_depth - 1, app, error);

    if (!value) {
        return NULL;
    }

    if (!token_stream_require(stream, ">", " to follow variant value", error)) {
        ast_free(value);
        return NULL;
    }

    variant = c_malloc0(sizeof(Variant));
    variant->ast.class = &variant_class;
    variant->value = value;

    return (AST *)variant;
}

// dictionary
static cchar *dictionary_get_pattern(AST *ast, CError **error)
{
    Dictionary *dict = (Dictionary *)ast;
    cchar *value_pattern;
    cchar *key_pattern;
    cchar key_char;
    cchar *result;

    if (dict->n_children == 0) {
        return c_strdup("Ma{**}");
    }

    key_pattern = ast_array_get_pattern(dict->keys, abs(dict->n_children), error);

    if (key_pattern == NULL) {
        return NULL;
    }

    /* we can not have maybe keys */
    if (key_pattern[0] == 'M') {
        key_char = key_pattern[1];
    }
    else {
        key_char = key_pattern[0];
    }

    c_free(key_pattern);

    if (!strchr("bynqiuxthdsogNS", key_char)) {
        ast_set_error(ast, error, NULL, C_VARIANT_PARSE_ERROR_BASIC_TYPE_EXPECTED,
                      "dictionary keys must have basic types");
        return NULL;
    }

    value_pattern = ast_get_pattern(dict->values[0], error);

    if (value_pattern == NULL) {
        return NULL;
    }

    result = c_strdup_printf("M%s{%c%s}", dict->n_children > 0 ? "a" : "", key_char, value_pattern);
    c_free(value_pattern);

    return result;
}

static CVariant *dictionary_get_value(AST *ast, const CVariantType *type, CError **error)
{
    Dictionary *dict = (Dictionary *)ast;

    if (dict->n_children == -1) {
        const CVariantType *subtype;
        CVariantBuilder builder;
        CVariant *subvalue;

        if (!c_variant_type_is_dict_entry(type)) {
            return ast_type_error(ast, type, error);
        }

        c_variant_builder_init_static(&builder, type);
        subtype = c_variant_type_key(type);
        if (!(subvalue = ast_get_value(dict->keys[0], subtype, error))) {
            c_variant_builder_clear(&builder);
            return NULL;
        }
        c_variant_builder_add_value(&builder, subvalue);
        subtype = c_variant_type_value(type);
        if (!(subvalue = ast_get_value(dict->values[0], subtype, error))) {
            c_variant_builder_clear(&builder);
            return NULL;
        }
        c_variant_builder_add_value(&builder, subvalue);
        return c_variant_builder_end(&builder);
    }
    else {
        const CVariantType *entry, *key, *val;
        CVariantBuilder builder;
        cint i;
        if (!c_variant_type_is_subtype_of(type, C_VARIANT_TYPE_DICTIONARY)) {
            return ast_type_error(ast, type, error);
        }
        entry = c_variant_type_element(type);
        key = c_variant_type_key(entry);
        val = c_variant_type_value(entry);
        c_variant_builder_init_static(&builder, type);
        for (i = 0; i < dict->n_children; i++) {
            CVariant *subvalue;
            c_variant_builder_open(&builder, entry);
            if (!(subvalue = ast_get_value(dict->keys[i], key, error))) {
                c_variant_builder_clear(&builder);
                return NULL;
            }
            c_variant_builder_add_value(&builder, subvalue);
            if (!(subvalue = ast_get_value(dict->values[i], val, error))) {
                c_variant_builder_clear(&builder);
                return NULL;
            }
            c_variant_builder_add_value(&builder, subvalue);
            c_variant_builder_close(&builder);
        }
        return c_variant_builder_end(&builder);
    }
}

static void dictionary_free(AST *ast)
{
    Dictionary *dict = (Dictionary *)ast;
    cint n_children;

    if (dict->n_children > -1) {
        n_children = dict->n_children;
    }
    else {
        n_children = 1;
    }

    ast_array_free(dict->keys, n_children);
    ast_array_free(dict->values, n_children);
    c_free(dict);
}

static AST *dictionary_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error)
{
    static const ASTClass dictionary_class = {dictionary_get_pattern, maybe_wrapper, dictionary_get_value,
                                              dictionary_free};
    cint n_keys, n_values;
    bool only_one;
    Dictionary *dict;
    AST *first;

    dict = c_malloc0(sizeof(Dictionary));
    dict->ast.class = &dictionary_class;
    dict->keys = NULL;
    dict->values = NULL;
    n_keys = n_values = 0;

    token_stream_assert(stream, "{");

    if (token_stream_consume(stream, "}")) {
        dict->n_children = 0;
        return (AST *)dict;
    }

    if ((first = parse(stream, max_depth - 1, app, error)) == NULL) {
        goto error;
    }

    ast_array_append(&dict->keys, &n_keys, first);

    only_one = token_stream_consume(stream, ",");
    if (!only_one && !token_stream_require(stream, ":", " or ',' to follow dictionary entry key", error)) {
        goto error;
    }

    if ((first = parse(stream, max_depth - 1, app, error)) == NULL) {
        goto error;
    }

    ast_array_append(&dict->values, &n_values, first);

    if (only_one) {
        if (!token_stream_require(stream, "}", " at end of dictionary entry", error)) {
            goto error;
        }
        c_assert(n_keys == 1 && n_values == 1);
        dict->n_children = -1;
        return (AST *)dict;
    }

    while (!token_stream_consume(stream, "}")) {
        AST *child;
        if (!token_stream_require(stream, ",", " or '}' to follow dictionary entry", error)) {
            goto error;
        }
        child = parse(stream, max_depth - 1, app, error);
        if (!child) {
            goto error;
        }
        ast_array_append(&dict->keys, &n_keys, child);
        if (!token_stream_require(stream, ":", " to follow dictionary entry key", error)) {
            goto error;
        }
        child = parse(stream, max_depth - 1, app, error);
        if (!child) {
            goto error;
        }
        ast_array_append(&dict->values, &n_values, child);
    }

    c_assert(n_keys == n_values);
    dict->n_children = n_keys;

    return (AST *)dict;

error:
    ast_array_free(dict->keys, n_keys);
    ast_array_free(dict->values, n_values);
    c_free(dict);

    return NULL;
}


// string
static cchar *string_get_pattern(AST *ast, CError **error) { return c_strdup("MS"); }

static CVariant *string_get_value(AST *ast, const CVariantType *type, CError **error)
{
    String *string = (String *)ast;

    if (c_variant_type_equal(type, C_VARIANT_TYPE_STRING)) {
        return c_variant_new_string(string->string);
    }
    else if (c_variant_type_equal(type, C_VARIANT_TYPE_OBJECT_PATH)) {
        if (!c_variant_is_object_path(string->string)) {
            ast_set_error(ast, error, NULL, C_VARIANT_PARSE_ERROR_INVALID_OBJECT_PATH, "not a valid object path");
            return NULL;
        }
        return c_variant_new_object_path(string->string);
    }
    else if (c_variant_type_equal(type, C_VARIANT_TYPE_SIGNATURE)) {
        if (!c_variant_is_signature(string->string)) {
            ast_set_error(ast, error, NULL, C_VARIANT_PARSE_ERROR_INVALID_SIGNATURE, "not a valid signature");
            return NULL;
        }
        return c_variant_new_signature(string->string);
    }
    else {
        return ast_type_error(ast, type, error);
    }
}

static void string_free(AST *ast)
{
    String *str = (String *)ast;

    c_free(str->string);
    c_free0(str);
}

static bool unicode_unescape(const cchar *src, cint *src_ofs, cchar *dest, cint *dest_ofs, csize length, SourceRef *ref,
                             CError **error)
{
    cchar buffer[9];
    cuint64 value = 0;
    cchar *end = NULL;
    csize n_valid_chars;

    (*src_ofs)++;

    c_assert(length < sizeof(buffer));
    strncpy(buffer, src + *src_ofs, length);
    buffer[length] = '\0';

    for (n_valid_chars = 0; n_valid_chars < length; n_valid_chars++) {
        if (!c_ascii_isxdigit(buffer[n_valid_chars])) {
            break;
        }
    }

    if (n_valid_chars == length) {
        value = c_ascii_strtoull(buffer, &end, 0x10);
    }

    if (value == 0 || end != buffer + length) {
        SourceRef escape_ref;
        escape_ref = *ref;
        escape_ref.start += *src_ofs;
        escape_ref.end = escape_ref.start + n_valid_chars;
        parser_set_error(error, &escape_ref, NULL, C_VARIANT_PARSE_ERROR_INVALID_CHARACTER,
                         "invalid %ld-character unicode escape", length);
        return false;
    }

    c_assert(value <= C_MAX_UINT32);

    *dest_ofs += c_unichar_to_utf8(value, dest + *dest_ofs);
    *src_ofs += length;

    return true;
}

static AST *string_parse(TokenStream *stream, va_list *app, CError **error)
{
    static const ASTClass string_class = {string_get_pattern, maybe_wrapper, string_get_value, string_free};
    String *str0;
    SourceRef ref;
    cchar *token;
    csize length;
    cchar quote;
    cchar *str;
    cint i, j;

    token_stream_start_ref(stream, &ref);
    token = token_stream_get(stream);
    token_stream_end_ref(stream, &ref);
    length = strlen(token);
    quote = token[0];

    str = c_malloc0(length);
    c_assert(quote == '"' || quote == '\'');
    j = 0;
    i = 1;
    while (token[i] != quote) {
        switch (token[i]) {
        case '\0':
            parser_set_error(error, &ref, NULL, C_VARIANT_PARSE_ERROR_UNTERMINATED_STRING_CONSTANT,
                             "unterminated string constant");
            c_free(token);
            c_free(str);
            return NULL;
        case '\\':
            switch (token[++i]) {
            case '\0':
                parser_set_error(error, &ref, NULL, C_VARIANT_PARSE_ERROR_UNTERMINATED_STRING_CONSTANT,
                                 "unterminated string constant");
                c_free(token);
                c_free(str);
                return NULL;
            case 'u':
                if (!unicode_unescape(token, &i, str, &j, 4, &ref, error)) {
                    c_free(token);
                    c_free(str);
                    return NULL;
                }
                continue;
            case 'U':
                if (!unicode_unescape(token, &i, str, &j, 8, &ref, error)) {
                    c_free(token);
                    c_free(str);
                    return NULL;
                }
                continue;
            case 'a':
                str[j++] = '\a';
                i++;
                continue;
            case 'b':
                str[j++] = '\b';
                i++;
                continue;
            case 'f':
                str[j++] = '\f';
                i++;
                continue;
            case 'n':
                str[j++] = '\n';
                i++;
                continue;
            case 'r':
                str[j++] = '\r';
                i++;
                continue;
            case 't':
                str[j++] = '\t';
                i++;
                continue;
            case 'v':
                str[j++] = '\v';
                i++;
                continue;
            case '\n':
                i++;
                continue;
            }
            C_FALLTHROUGH;
        default:
            str[j++] = token[i++];
        }
    }
    str[j++] = '\0';
    c_free(token);
    str0 = c_malloc0(sizeof(String));
    str0->ast.class = &string_class;
    str0->string = str;

    token_stream_next(stream);

    return (AST *)str0;
}


// byte string
static cchar *bytestring_get_pattern(AST *ast, CError **error) { return c_strdup("May"); }

static CVariant *bytestring_get_value(AST *ast, const CVariantType *type, CError **error)
{
    ByteString *string = (ByteString *)ast;

    if (!c_variant_type_equal(type, C_VARIANT_TYPE_BYTESTRING)) {
        return ast_type_error(ast, type, error);
    }

    return c_variant_new_bytestring(string->string);
}

static void bytestring_free(AST *ast)
{
    ByteString *str = (ByteString *)ast;

    c_free(str->string);
    c_free(str);
}

static AST *bytestring_parse(TokenStream *stream, va_list *app, CError **error)
{
    static const ASTClass bytestring_class = {bytestring_get_pattern, maybe_wrapper, bytestring_get_value,
                                              bytestring_free};
    ByteString *string;
    SourceRef ref;
    cchar *token;
    csize length;
    cchar quote;
    cchar *str;
    cint i, j;

    token_stream_start_ref(stream, &ref);
    token = token_stream_get(stream);
    token_stream_end_ref(stream, &ref);
    c_assert(token[0] == 'b');
    length = strlen(token);
    quote = token[1];

    str = c_malloc0(length);
    c_assert(quote == '"' || quote == '\'');
    j = 0;
    i = 2;
    while (token[i] != quote) {
        switch (token[i]) {
        case '\0':
            parser_set_error(error, &ref, NULL, C_VARIANT_PARSE_ERROR_UNTERMINATED_STRING_CONSTANT,
                             "unterminated string constant");
            c_free(str);
            c_free(token);
            return NULL;
        case '\\':
            switch (token[++i]) {
            case '\0':
                parser_set_error(error, &ref, NULL, C_VARIANT_PARSE_ERROR_UNTERMINATED_STRING_CONSTANT,
                                 "unterminated string constant");
                c_free(str);
                c_free(token);
                return NULL;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                /* up to 3 characters */
                cuchar val = token[i++] - '0';
                if ('0' <= token[i] && token[i] < '8') {
                    val = (val << 3) | (token[i++] - '0');
                }
                if ('0' <= token[i] && token[i] < '8') {
                    val = (val << 3) | (token[i++] - '0');
                }
                str[j++] = val;
            }
                continue;
            case 'a':
                str[j++] = '\a';
                i++;
                continue;
            case 'b':
                str[j++] = '\b';
                i++;
                continue;
            case 'f':
                str[j++] = '\f';
                i++;
                continue;
            case 'n':
                str[j++] = '\n';
                i++;
                continue;
            case 'r':
                str[j++] = '\r';
                i++;
                continue;
            case 't':
                str[j++] = '\t';
                i++;
                continue;
            case 'v':
                str[j++] = '\v';
                i++;
                continue;
            case '\n':
                i++;
                continue;
            }
            C_FALLTHROUGH;
        default:
            str[j++] = token[i++];
        }
    }
    str[j++] = '\0';
    c_free(token);
    string = c_malloc0(sizeof(ByteString));
    string->ast.class = &bytestring_class;
    string->string = str;

    token_stream_next(stream);

    return (AST *)string;
}


// number
static cchar *number_get_pattern(AST *ast, CError **error)
{
    Number *number = (Number *)ast;

    if (strchr(number->token, '.') || (!c_str_has_prefix(number->token, "0x") && strchr(number->token, 'e')) ||
        strstr(number->token, "inf") || strstr(number->token, "nan")) {
        return c_strdup("Md");
    }

    return c_strdup("MN");
}

static CVariant *number_overflow(AST *ast, const CVariantType *type, CError **error)
{
    ast_set_error(ast, error, NULL, C_VARIANT_PARSE_ERROR_NUMBER_OUT_OF_RANGE, "number out of range for type '%c'",
                  c_variant_type_peek_string(type)[0]);
    return NULL;
}

static CVariant *number_get_value(AST *ast, const CVariantType *type, CError **error)
{
    Number *number = (Number *)ast;
    const cchar *token;
    bool negative;
    bool floating;
    cuint64 abs_val;
    cdouble dbl_val;
    cchar *end;

    token = number->token;

    if (c_variant_type_equal(type, C_VARIANT_TYPE_DOUBLE)) {
        floating = true;
        errno = 0;
        dbl_val = c_ascii_strtod(token, &end);
        if (dbl_val != 0.0 && errno == ERANGE) {
            ast_set_error(ast, error, NULL, C_VARIANT_PARSE_ERROR_NUMBER_TOO_BIG, "number too big for any type");
            return NULL;
        }
        /* silence uninitialised warnings... */
        negative = false;
        abs_val = 0;
    }
    else {
        floating = false;
        negative = token[0] == '-';
        if (token[0] == '-') {
            token++;
        }
        errno = 0;
        abs_val = c_ascii_strtoull(token, &end, 0);
        if (abs_val == C_MAX_UINT64 && errno == ERANGE) {
            ast_set_error(ast, error, NULL, C_VARIANT_PARSE_ERROR_NUMBER_TOO_BIG, "integer too big for any type");
            return NULL;
        }

        if (abs_val == 0) {
            negative = false;
        }
        dbl_val = 0.0;
    }

    if (*end != '\0') {
        SourceRef ref;
        ref = ast->source_ref;
        ref.start += end - number->token;
        ref.end = ref.start + 1;
        parser_set_error(error, &ref, NULL, C_VARIANT_PARSE_ERROR_INVALID_CHARACTER, "invalid character in number");
        return NULL;
    }

    if (floating) {
        return c_variant_new_double(dbl_val);
    }

    switch (*c_variant_type_peek_string(type)) {
    case 'y':
        if (negative || abs_val > C_MAX_UINT8) {
            return number_overflow(ast, type, error);
        }
        return c_variant_new_byte(abs_val);
    case 'n':
        if (abs_val - negative > C_MAX_INT16) {
            return number_overflow(ast, type, error);
        }
        if (negative && abs_val > C_MAX_INT16) {
            return c_variant_new_int16(C_MIN_INT16);
        }
        return c_variant_new_int16(negative ? -((cint16)abs_val) : ((cint16)abs_val));
    case 'q':
        if (negative || abs_val > C_MAX_UINT16)
            return number_overflow(ast, type, error);
        return c_variant_new_uint16(abs_val);
    case 'i':
        if (abs_val - negative > C_MAX_INT32)
            return number_overflow(ast, type, error);
        if (negative && abs_val > C_MAX_INT32)
            return c_variant_new_int32(C_MIN_INT32);
        return c_variant_new_int32(negative ? -((cint32)abs_val) : ((cint32)abs_val));
    case 'u':
        if (negative || abs_val > C_MAX_UINT32) {
            return number_overflow(ast, type, error);
        }
        return c_variant_new_uint32(abs_val);
    case 'x':
        if (abs_val - negative > C_MAX_INT64)
            return number_overflow(ast, type, error);
        if (negative && abs_val > C_MAX_INT64)
            return c_variant_new_int64(C_MIN_INT64);
        return c_variant_new_int64(negative ? -((cint64)abs_val) : ((cint64)abs_val));
    case 't':
        if (negative)
            return number_overflow(ast, type, error);
        return c_variant_new_uint64(abs_val);
    case 'h':
        if (abs_val - negative > C_MAX_INT32)
            return number_overflow(ast, type, error);
        if (negative && abs_val > C_MAX_INT32)
            return c_variant_new_handle(C_MIN_INT32);
        return c_variant_new_handle(negative ? -((cint32)abs_val) : ((cint32)abs_val));
    default:
        return ast_type_error(ast, type, error);
    }
}

static void number_free(AST *ast)
{
    Number *number = (Number *)ast;

    c_free(number->token);
    c_free(number);
}

static AST *number_parse(TokenStream *stream, va_list *app, CError **error)
{
    static const ASTClass number_class = {number_get_pattern, maybe_wrapper, number_get_value, number_free};
    Number *number;

    number = c_malloc0(sizeof(Number));
    number->ast.class = &number_class;
    number->token = token_stream_get(stream);
    token_stream_next(stream);

    return (AST *)number;
}


// bool
static cchar *boolean_get_pattern(AST *ast, CError **error) { return c_strdup("Mb"); }

static CVariant *boolean_get_value(AST *ast, const CVariantType *type, CError **error)
{
    Boolean *boolean = (Boolean *)ast;

    if (!c_variant_type_equal(type, C_VARIANT_TYPE_BOOLEAN)) {
        return ast_type_error(ast, type, error);
    }

    return c_variant_new_boolean(boolean->value);
}

static void boolean_free(AST *ast)
{
    Boolean *boolean = (Boolean *)ast;

    c_free0(boolean);
}

static AST *boolean_new(bool value)
{
    static const ASTClass boolean_class = {boolean_get_pattern, maybe_wrapper, boolean_get_value, boolean_free};
    Boolean *boolean;

    boolean = c_malloc0(sizeof(Boolean));
    boolean->ast.class = &boolean_class;
    boolean->value = value;

    return (AST *)boolean;
}

// positional
static cchar *positional_get_pattern(AST *ast, CError **error)
{
    Positional *positional = (Positional *)ast;

    return c_strdup(c_variant_get_type_string(positional->value));
}

static CVariant *positional_get_value(AST *ast, const CVariantType *type, CError **error)
{
    Positional *positional = (Positional *)ast;
    CVariant *value;

    c_assert(positional->value != NULL);

    if C_UNLIKELY (!c_variant_is_of_type(positional->value, type)) {
        return ast_type_error(ast, type, error);
    }

    c_assert(positional->value != NULL);
    value = positional->value;
    positional->value = NULL;

    return value;
}

static void positional_free(AST *ast)
{
    Positional *positional = (Positional *)ast;

    c_free(positional);
}

static AST *positional_parse(TokenStream *stream, va_list *app, CError **error)
{
    static const ASTClass positional_class = {positional_get_pattern, positional_get_value, NULL, positional_free};
    Positional *positional;
    const cchar *endptr;
    cchar *token;

    token = token_stream_get(stream);
    c_assert(token[0] == '%');

    positional = c_malloc0(sizeof(Positional));
    positional->ast.class = &positional_class;
    positional->value = c_variant_new_va(token + 1, &endptr, app);

    if (*endptr || positional->value == NULL) {
        token_stream_set_error(stream, error, true, C_VARIANT_PARSE_ERROR_INVALID_FORMAT_STRING,
                               "invalid GVariant format string");
        /* memory management doesn't matter in case of programmer error. */
        return NULL;
    }

    token_stream_next(stream);
    c_free(token);

    return (AST *)positional;
}

// type decl
static cchar *typedecl_get_pattern(AST *ast, CError **error)
{
    TypeDecl *decl = (TypeDecl *)ast;
    return c_variant_type_dup_string(decl->type);
}

static CVariant *typedecl_get_value(AST *ast, const CVariantType *type, CError **error)
{
    TypeDecl *decl = (TypeDecl *)ast;

    return ast_get_value(decl->child, type, error);
}

static void typedecl_free(AST *ast)
{
    TypeDecl *decl = (TypeDecl *)ast;

    ast_free(decl->child);
    c_variant_type_free(decl->type);
    c_free(decl);
}

static AST *typedecl_parse(TokenStream *stream, cuint max_depth, va_list *app, CError **error)
{
    static const ASTClass typedecl_class = {typedecl_get_pattern, typedecl_get_value, NULL, typedecl_free};
    CVariantType *type;
    TypeDecl *decl;
    AST *child;

    if (token_stream_peek(stream, '@')) {
        cchar *token;
        token = token_stream_get(stream);
        if (!c_variant_type_string_is_valid(token + 1)) {
            token_stream_set_error(stream, error, true, C_VARIANT_PARSE_ERROR_INVALID_TYPE_STRING,
                                   "invalid type declaration");
            c_free(token);
            return NULL;
        }

        if (c_variant_type_string_get_depth_(token + 1) > max_depth) {
            token_stream_set_error(stream, error, true, C_VARIANT_PARSE_ERROR_RECURSION,
                                   "type declaration recurses too deeply");
            c_free(token);
            return NULL;
        }
        type = c_variant_type_new(token + 1);
        if (!c_variant_type_is_definite(type)) {
            token_stream_set_error(stream, error, true, C_VARIANT_PARSE_ERROR_DEFINITE_TYPE_EXPECTED,
                                   "type declarations must be definite");
            c_variant_type_free(type);
            c_free(token);
            return NULL;
        }
        token_stream_next(stream);
        c_free(token);
    }
    else {
        if (token_stream_consume(stream, "boolean")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_BOOLEAN);
        }
        else if (token_stream_consume(stream, "byte")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_BYTE);
        }
        else if (token_stream_consume(stream, "int16")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_INT16);
        }
        else if (token_stream_consume(stream, "uint16")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_UINT16);
        }
        else if (token_stream_consume(stream, "int32")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_INT32);
        }
        else if (token_stream_consume(stream, "handle")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_HANDLE);
        }
        else if (token_stream_consume(stream, "uint32")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_UINT32);
        }
        else if (token_stream_consume(stream, "int64")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_INT64);
        }
        else if (token_stream_consume(stream, "uint64")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_UINT64);
        }
        else if (token_stream_consume(stream, "double")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_DOUBLE);
        }
        else if (token_stream_consume(stream, "string")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_STRING);
        }
        else if (token_stream_consume(stream, "objectpath")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_OBJECT_PATH);
        }
        else if (token_stream_consume(stream, "signature")) {
            type = c_variant_type_copy(C_VARIANT_TYPE_SIGNATURE);
        }
        else {
            token_stream_set_error(stream, error, true, C_VARIANT_PARSE_ERROR_UNKNOWN_KEYWORD, "unknown keyword");
            return NULL;
        }
    }

    if ((child = parse(stream, max_depth - 1, app, error)) == NULL) {
        c_variant_type_free(type);
        return NULL;
    }

    decl = c_malloc0(sizeof(TypeDecl));
    decl->ast.class = &typedecl_class;
    decl->type = type;
    decl->child = child;

    return (AST *)decl;
}

// static
static bool valid_format_string(const cchar *format_string, bool single, CVariant *value)
{
    const cchar *endptr;
    CVariantType *type;

    if C_LIKELY (value == NULL && c_variant_format_string_scan(format_string, NULL, &endptr) &&
                 (single || *endptr == '\0')) {
        return true;
    }

    type = c_variant_format_string_scan_type(format_string, NULL, &endptr);

    if C_UNLIKELY (type == NULL || (single && *endptr != '\0')) {
        if (single) {
            C_LOG_CRIT("'%s' is not a valid GVariant format string", format_string);
        }
        else {
            C_LOG_CRIT("'%s' does not have a valid GVariant format string as a prefix", format_string);
        }

        if (type != NULL) {
            c_variant_type_free(type);
        }
        return false;
    }

    if C_UNLIKELY (value && !c_variant_is_of_type(value, type)) {
        cchar *fragment;
        cchar *typestr;

        fragment = c_strndup(format_string, endptr - format_string);
        typestr = c_variant_type_dup_string(type);

        C_LOG_CRIT("the GVariant format string '%s' has a type of '%s' but the given value has a type of '%s'",
                   fragment, typestr, c_variant_get_type_string(value));

        c_variant_type_free(type);
        c_free(fragment);
        c_free(typestr);

        return false;
    }

    c_variant_type_free(type);

    return true;
}

static void c_variant_valist_get(const cchar **str, CVariant *value, bool free, va_list *app)
{
    if (c_variant_format_string_is_leaf(*str)) {
        c_variant_valist_get_leaf(str, value, free, app);
    }
    else if (**str == 'm') {
        (*str)++;
        if (value != NULL) {
            value = c_variant_get_maybe(value);
        }

        if (!c_variant_format_string_is_nnp(*str)) {
            bool *ptr = va_arg(*app, bool *);
            if (ptr != NULL) {
                *ptr = value != NULL;
            }
        }
        c_variant_valist_get(str, value, free, app);
        if (value != NULL) {
            c_variant_unref(value);
        }
    }
    else {
        cint index = 0;
        c_assert(**str == '(' || **str == '{');
        (*str)++;
        while (**str != ')' && **str != '}') {
            if (value != NULL) {
                CVariant *child = c_variant_get_child_value(value, index++);
                c_variant_valist_get(str, child, free, app);
                c_variant_unref(child);
            }
            else {
                c_variant_valist_get(str, NULL, free, app);
            }
        }
        (*str)++;
    }
}

static bool c_variant_format_string_is_leaf(const cchar *str)
{
    return str[0] != 'm' && str[0] != '(' && str[0] != '{';
}

static bool c_variant_format_string_is_nnp(const cchar *str)
{
    return str[0] == 'a' || str[0] == 's' || str[0] == 'o' || str[0] == 'g' || str[0] == '^' || str[0] == '@' ||
        str[0] == '*' || str[0] == '?' || str[0] == 'r' || str[0] == 'v' || str[0] == '&';
}

/* Single non-null pointer ("nnp") {{{2 */
static void c_variant_valist_free_nnp(const cchar *str, void *ptr)
{
    switch (*str) {
    case 'a':
        c_variant_iter_free(ptr);
        break;

    case '^':
        if (c_str_has_suffix(str, "y")) {
            if (str[2] != 'a') /* '^a&ay', '^ay' */
                c_free(ptr);
            else if (str[1] == 'a') /* '^aay' */
                c_strfreev(ptr);
            break; /* '^&ay' */
        }
        else if (str[2] != '&') /* '^as', '^ao' */
            c_strfreev(ptr);
        else /* '^a&s', '^a&o' */
            c_free(ptr);
        break;

    case 's':
    case 'o':
    case 'g':
        c_free(ptr);
        break;

    case '@':
    case '*':
    case '?':
    case 'v':
        c_variant_unref(ptr);
        break;

    case '&':
        break;

    default:
        c_assert_not_reached();
    }
}

static cchar c_variant_scan_convenience(const cchar **str, bool *constant, cuint *arrays)
{
    *constant = false;
    *arrays = 0;

    for (;;) {
        char c = *(*str)++;

        if (c == '&')
            *constant = true;

        else if (c == 'a')
            (*arrays)++;

        else
            return c;
    }
}

static CVariant *c_variant_valist_new_nnp(const cchar **str, void *ptr)
{
    if (**str == '&') {
        (*str)++;
    }

    switch (*(*str)++) {
    case 'a':
        if (ptr != NULL) {
            const CVariantType *type;
            CVariant *value;

            value = c_variant_builder_end(ptr);
            type = c_variant_get_type(value);
            if C_UNLIKELY (!c_variant_type_is_array(type)) {
                C_LOG_ERROR("c_variant_new: expected array GVariantBuilder but "
                            "the built value has type '%s'",
                            c_variant_get_type_string(value));
            }
            type = c_variant_type_element(type);

            if C_UNLIKELY (!c_variant_type_is_subtype_of(type, (CVariantType *)*str)) {
                cchar *type_string = c_variant_type_dup_string((CVariantType *)*str);
                C_LOG_ERROR("c_variant_new: expected GVariantBuilder array element "
                            "type '%s' but the built value has element type '%s'",
                            type_string, c_variant_get_type_string(value) + 1);
                c_free(type_string);
            }
            c_variant_type_string_scan(*str, NULL, str);
            return value;
        }
        else {
            /* special case: NULL pointer for empty array */
            const CVariantType *type = (CVariantType *)*str;
            c_variant_type_string_scan(*str, NULL, str);
            if C_UNLIKELY (!c_variant_type_is_definite(type))
                C_LOG_ERROR("c_variant_new: NULL pointer given with indefinite "
                            "array type; unable to determine which type of empty "
                            "array to construct.");
            return c_variant_new_array(type, NULL, 0);
        }
    case 's': {
        CVariant *value;
        value = c_variant_new_string(ptr);
        if (value == NULL) {
            value = c_variant_new_string("[Invalid UTF-8]");
        }
        return value;
    }
    case 'o':
        return c_variant_new_object_path(ptr);
    case 'g':
        return c_variant_new_signature(ptr);
    case '^': {
        bool constant;
        cuint arrays;
        cchar type;
        type = c_variant_scan_convenience(str, &constant, &arrays);
        if (type == 's') {
            return c_variant_new_strv(ptr, -1);
        }
        if (type == 'o') {
            return c_variant_new_objv(ptr, -1);
        }
        if (arrays > 1) {
            return c_variant_new_bytestring_array(ptr, -1);
        }
        return c_variant_new_bytestring(ptr);
    }
    case '@':
        if C_UNLIKELY (!c_variant_is_of_type(ptr, (CVariantType *)*str)) {
            cchar *type_string = c_variant_type_dup_string((CVariantType *)*str);
            C_LOG_ERROR("c_variant_new: expected GVariant of type '%s' but "
                        "received value has type '%s'",
                        type_string, c_variant_get_type_string(ptr));
            c_free(type_string);
        }
        c_variant_type_string_scan(*str, NULL, str);
        return ptr;
    case '*':
        return ptr;
    case '?':
        if C_UNLIKELY (!c_variant_type_is_basic(c_variant_get_type(ptr))) {
            C_LOG_ERROR("c_variant_new: format string '?' expects basic-typed "
                        "GVariant, but received value has type '%s'",
                        c_variant_get_type_string(ptr));
        }
        return ptr;
    case 'r':
        if C_UNLIKELY (!c_variant_type_is_tuple(c_variant_get_type(ptr)))
            C_LOG_ERROR("c_variant_new: format string 'r' expects tuple-typed "
                        "CVariant, but received value has type '%s'",
                        c_variant_get_type_string(ptr));
        return ptr;
    case 'v':
        return c_variant_new_variant(ptr);
    default:
        c_assert_not_reached();
    }
}

static void *c_variant_valist_get_nnp(const cchar **str, CVariant *value)
{
    switch (*(*str)++) {
    case 'a':
        c_variant_type_string_scan(*str, NULL, str);
        return c_variant_iter_new(value);
    case '&':
        (*str)++;
        return (cchar *)c_variant_get_string(value, NULL);

    case 's':
    case 'o':
    case 'g':
        return c_variant_dup_string(value, NULL);

    case '^': {
        bool constant;
        cuint arrays;
        cchar type;

        type = c_variant_scan_convenience(str, &constant, &arrays);

        if (type == 's') {
            if (constant)
                return c_variant_get_strv(value, NULL);
            else
                return c_variant_dup_strv(value, NULL);
        }

        else if (type == 'o') {
            if (constant)
                return c_variant_get_objv(value, NULL);
            else
                return c_variant_dup_objv(value, NULL);
        }

        else if (arrays > 1) {
            if (constant)
                return c_variant_get_bytestring_array(value, NULL);
            else
                return c_variant_dup_bytestring_array(value, NULL);
        }

        else {
            if (constant)
                return (cchar *)c_variant_get_bytestring(value);
            else
                return c_variant_dup_bytestring(value, NULL);
        }
    }

    case '@':
        c_variant_type_string_scan(*str, NULL, str);
        C_FALLTHROUGH;

    case '*':
    case '?':
    case 'r':
        return c_variant_ref(value);

    case 'v':
        return c_variant_get_variant(value);

    default:
        c_assert_not_reached();
    }
}

/* Leaves {{{2 */
static void c_variant_valist_skip_leaf(const cchar **str, va_list *app)
{
    if (c_variant_format_string_is_nnp(*str)) {
        c_variant_format_string_scan(*str, NULL, str);
        va_arg(*app, void *);
        return;
    }

    switch (*(*str)++) {
    case 'b':
    case 'y':
    case 'n':
    case 'q':
    case 'i':
    case 'u':
    case 'h':
        va_arg(*app, int);
        return;

    case 'x':
    case 't':
        va_arg(*app, cuint64);
        return;

    case 'd':
        va_arg(*app, cdouble);
        return;

    default:
        c_assert_not_reached();
    }
}

static CVariant *c_variant_valist_new_leaf(const cchar **str, va_list *app)
{
    if (c_variant_format_string_is_nnp(*str)) {
        return c_variant_valist_new_nnp(str, va_arg(*app, void *));
    }

    switch (*(*str)++) {
    case 'b':
        return c_variant_new_boolean((1 == va_arg(*app, int32_t)));
    case 'y':
        return c_variant_new_byte(va_arg(*app, cuint));
    case 'n':
        return c_variant_new_int16(va_arg(*app, cint));
    case 'q':
        return c_variant_new_uint16(va_arg(*app, cuint));
    case 'i':
        return c_variant_new_int32(va_arg(*app, cint));
    case 'u':
        return c_variant_new_uint32(va_arg(*app, cuint));
    case 'x':
        return c_variant_new_int64(va_arg(*app, cint64));
    case 't':
        return c_variant_new_uint64(va_arg(*app, cuint64));
    case 'h':
        return c_variant_new_handle(va_arg(*app, cint));
    case 'd':
        return c_variant_new_double(va_arg(*app, cdouble));
    default:
        c_assert_not_reached();
    }
}

/* The code below assumes this */
// C_STATIC_ASSERT (sizeof (bool) == sizeof (cuint32));
C_STATIC_ASSERT(sizeof(cdouble) == sizeof(cuint64));

static void c_variant_valist_get_leaf(const cchar **str, CVariant *value, bool free, va_list *app)
{
    void *ptr = va_arg(*app, void *);

    if (ptr == NULL) {
        c_variant_format_string_scan(*str, NULL, str);
        return;
    }

    if (c_variant_format_string_is_nnp(*str)) {
        void **nnp = (void **)ptr;

        if (free && *nnp != NULL) {
            c_variant_valist_free_nnp(*str, *nnp);
        }

        *nnp = NULL;

        if (value != NULL) {
            *nnp = c_variant_valist_get_nnp(str, value);
        }
        else {
            c_variant_format_string_scan(*str, NULL, str);
        }
        return;
    }

    if (value != NULL) {
        switch (*(*str)++) {
        case 'b':
            *(bool *)ptr = c_variant_get_boolean(value);
            return;

        case 'y':
            *(cuint8 *)ptr = c_variant_get_byte(value);
            return;

        case 'n':
            *(cint16 *)ptr = c_variant_get_int16(value);
            return;

        case 'q':
            *(cuint16 *)ptr = c_variant_get_uint16(value);
            return;

        case 'i':
            *(cint32 *)ptr = c_variant_get_int32(value);
            return;

        case 'u':
            *(cuint32 *)ptr = c_variant_get_uint32(value);
            return;

        case 'x':
            *(cint64 *)ptr = c_variant_get_int64(value);
            return;

        case 't':
            *(cuint64 *)ptr = c_variant_get_uint64(value);
            return;

        case 'h':
            *(cint32 *)ptr = c_variant_get_handle(value);
            return;

        case 'd':
            *(cdouble *)ptr = c_variant_get_double(value);
            return;
        }
    }
    else {
        switch (*(*str)++) {
        case 'y':
            *(cuint8 *)ptr = 0;
            return;

        case 'n':
        case 'q':
            *(cuint16 *)ptr = 0;
            return;

        case 'i':
        case 'u':
        case 'h':
        case 'b':
            *(cuint32 *)ptr = 0;
            return;

        case 'x':
        case 't':
        case 'd':
            *(cuint64 *)ptr = 0;
            return;
        }
    }

    c_assert_not_reached();
}

/* Generic (recursive) {{{2 */
static void c_variant_valist_skip(const cchar **str, va_list *app)
{
    if (c_variant_format_string_is_leaf(*str)) {
        c_variant_valist_skip_leaf(str, app);
    }
    else if (**str == 'm') {
        (*str)++;
        if (!c_variant_format_string_is_nnp(*str)) {
            va_arg(*app, int32_t);
        }
        c_variant_valist_skip(str, app);
    }
    else {
        c_assert(**str == '(' || **str == '{');
        (*str)++;
        while (**str != ')' && **str != '}') {
            c_variant_valist_skip(str, app);
        }
        (*str)++;
    }
}

static CVariant *c_variant_valist_new(const cchar **str, va_list *app)
{
    if (c_variant_format_string_is_leaf(*str))
        return c_variant_valist_new_leaf(str, app);

    if (**str == 'm') {
        CVariantType *type = NULL;
        CVariant *value = NULL;
        (*str)++;
        if (c_variant_format_string_is_nnp(*str)) {
            void *nnp = va_arg(*app, void *);
            if (nnp != NULL) {
                value = c_variant_valist_new_nnp(str, nnp);
            }
            else {
                type = c_variant_format_string_scan_type(*str, NULL, str);
            }
        }
        else {
            bool just = (1 == va_arg(*app, int32_t));
            if (just) {
                value = c_variant_valist_new(str, app);
            }
            else {
                type = c_variant_format_string_scan_type(*str, NULL, NULL);
                c_variant_valist_skip(str, app);
            }
        }

        value = c_variant_new_maybe(type, value);
        if (type != NULL) {
            c_variant_type_free(type);
        }
        return value;
    }
    else {
        CVariantBuilder b;
        if (**str == '(') {
            c_variant_builder_init_static(&b, C_VARIANT_TYPE_TUPLE);
        }
        else {
            c_assert(**str == '{');
            c_variant_builder_init_static(&b, C_VARIANT_TYPE_DICT_ENTRY);
        }

        (*str)++; /* '(' */
        while (**str != ')' && **str != '}') {
            c_variant_builder_add_value(&b, c_variant_valist_new(str, app));
        }
        (*str)++; /* ')' */

        return c_variant_builder_end(&b);
    }
}

static bool ensure_valid_builder(CVariantBuilder *builder)
{
    if (builder == NULL)
        return false;
    else if (is_valid_builder(builder))
        return true;
    if (builder->u.s.partial_magic == CVSB_MAGIC_PARTIAL) {
        static CVariantBuilder cleared_builder;

        /* Make sure that only first two fields were set and the rest is
         * zeroed to avoid messing up the builder that had parent
         * address equal to GVSB_MAGIC_PARTIAL. */
        if (memcmp(cleared_builder.u.s.y, builder->u.s.y, sizeof cleared_builder.u.s.y))
            return false;

        c_variant_builder_init_static(builder, builder->u.s.type);
    }
    return is_valid_builder(builder);
}

static void c_variant_builder_make_room(struct stack_builder *builder)
{
    if (builder->offset == builder->allocated_children) {
        builder->allocated_children *= 2;
        builder->children = c_realloc(builder->children, sizeof(CVariant *) * builder->allocated_children);
    }
}

static void _c_variant_builder_init(CVariantBuilder *builder, const CVariantType *type, bool type_owned)
{
    c_return_if_fail(type != NULL);
    c_return_if_fail(c_variant_type_is_container(type));

    memset(builder, 0, sizeof(CVariantBuilder));

    CVSB(builder)->type = (CVariantType *)type;
    CVSB(builder)->magic = CVSB_MAGIC;
    CVSB(builder)->trusted = true;
    CVSB(builder)->type_owned = type_owned;

    switch (*(const cchar *)type) {
    case C_VARIANT_CLASS_VARIANT:
        CVSB(builder)->uniform_item_types = true;
        CVSB(builder)->allocated_children = 1;
        CVSB(builder)->expected_type = NULL;
        CVSB(builder)->min_items = 1;
        CVSB(builder)->max_items = 1;
        break;

    case C_VARIANT_CLASS_ARRAY:
        CVSB(builder)->uniform_item_types = true;
        CVSB(builder)->allocated_children = 8;
        CVSB(builder)->expected_type = c_variant_type_element(CVSB(builder)->type);
        CVSB(builder)->min_items = 0;
        CVSB(builder)->max_items = -1;
        break;

    case C_VARIANT_CLASS_MAYBE:
        CVSB(builder)->uniform_item_types = true;
        CVSB(builder)->allocated_children = 1;
        CVSB(builder)->expected_type = c_variant_type_element(CVSB(builder)->type);
        CVSB(builder)->min_items = 0;
        CVSB(builder)->max_items = 1;
        break;

    case C_VARIANT_CLASS_DICT_ENTRY:
        CVSB(builder)->uniform_item_types = false;
        CVSB(builder)->allocated_children = 2;
        CVSB(builder)->expected_type = c_variant_type_key(CVSB(builder)->type);
        CVSB(builder)->min_items = 2;
        CVSB(builder)->max_items = 2;
        break;

    case 'r': /* G_VARIANT_TYPE_TUPLE was given */
        CVSB(builder)->uniform_item_types = false;
        CVSB(builder)->allocated_children = 8;
        CVSB(builder)->expected_type = NULL;
        CVSB(builder)->min_items = 0;
        CVSB(builder)->max_items = -1;
        break;

    case C_VARIANT_CLASS_TUPLE: /* a definite tuple type was given */
        CVSB(builder)->allocated_children = c_variant_type_n_items(type);
        CVSB(builder)->expected_type = c_variant_type_first(CVSB(builder)->type);
        CVSB(builder)->min_items = CVSB(builder)->allocated_children;
        CVSB(builder)->max_items = CVSB(builder)->allocated_children;
        CVSB(builder)->uniform_item_types = false;
        break;

    default:
        c_assert_not_reached();
    }

#if G_ANALYZER_ANALYZING
    CVSB(builder)->children = c_malloc0(sizeof(CVariant *) * CVSB(builder)->allocated_children);
#else
    CVSB(builder)->children = c_malloc0(sizeof(CVariant *) * CVSB(builder)->allocated_children);
#endif
}

static CVariant *c_variant_deep_copy(CVariant *value, bool byteswap)
{
    switch (c_variant_classify(value)) {
    case C_VARIANT_CLASS_MAYBE:
    case C_VARIANT_CLASS_TUPLE:
    case C_VARIANT_CLASS_DICT_ENTRY:
    case C_VARIANT_CLASS_VARIANT: {
        CVariantBuilder builder;
        csize i, n_children;

        c_variant_builder_init_static(&builder, c_variant_get_type(value));

        for (i = 0, n_children = c_variant_n_children(value); i < n_children; i++) {
            CVariant *child = c_variant_get_child_value(value, i);
            c_variant_builder_add_value(&builder, c_variant_deep_copy(child, byteswap));
            c_variant_unref(child);
        }

        return c_variant_builder_end(&builder);
    }

    case C_VARIANT_CLASS_ARRAY: {
        CVariantBuilder builder;
        csize i, n_children;
        CVariant *first_invalid_child_deep_copy = NULL;

        /* Arrays are in theory treated the same as maybes, tuples, dict entries
         * and variants, and could be another case in the above block of code.
         *
         * However, they have the property that when dealing with non-normal
         * data (which is the only time g_variant_deep_copy() is currently
         * called) in a variable-sized array, the code above can easily end up
         * creating many default child values in order to return an array which
         * is of the right length and type, but without containing non-normal
         * data. This can happen if the offset table for the array is malformed.
         *
         * In this case, the code above would end up allocating the same default
         * value for each one of the child indexes beyond the first malformed
         * entry in the offset table. This can end up being a lot of identical
         * allocations of default values, particularly if the non-normal array
         * is crafted maliciously.
         *
         * Avoid that problem by returning a new reference to the same default
         * value for every child after the first invalid one. This results in
         * returning an equivalent array, in normal form and trusted — but with
         * significantly fewer memory allocations.
         *
         * See https://gitlab.gnome.org/GNOME/glib/-/issues/2540 */

        c_variant_builder_init_static(&builder, c_variant_get_type(value));

        for (i = 0, n_children = c_variant_n_children(value); i < n_children; i++) {
            /* Try maybe_get_child_value() first; if it returns NULL, this child
             * is non-normal. get_child_value() would have constructed and
             * returned a default value in that case. */
            CVariant *child = c_variant_maybe_get_child_value(value, i);

            if (child != NULL) {
                /* Non-normal children may not always be contiguous, as they may
                 * be non-normal for reasons other than invalid offset table
                 * entries. As they are all the same type, they will all have
                 * the same default value though, so keep that around. */
                c_variant_builder_add_value(&builder, c_variant_deep_copy(child, byteswap));
            }
            else if (child == NULL && first_invalid_child_deep_copy != NULL) {
                c_variant_builder_add_value(&builder, first_invalid_child_deep_copy);
            }
            else if (child == NULL) {
                child = c_variant_get_child_value(value, i);
                first_invalid_child_deep_copy = c_variant_ref_sink(c_variant_deep_copy(child, byteswap));
                c_variant_builder_add_value(&builder, first_invalid_child_deep_copy);
            }

            c_clear_pointer((void *)&child, (CDestroyNotify)c_variant_unref);
        }

        c_clear_pointer((void *)&first_invalid_child_deep_copy, (CDestroyNotify)c_variant_unref);

        return c_variant_builder_end(&builder);
    }

    case C_VARIANT_CLASS_BOOLEAN:
        return c_variant_new_boolean(c_variant_get_boolean(value));

    case C_VARIANT_CLASS_BYTE:
        return c_variant_new_byte(c_variant_get_byte(value));

    case C_VARIANT_CLASS_INT16:
        if (byteswap)
            return c_variant_new_int16(C_UINT16_SWAP_LE_BE(c_variant_get_int16(value)));
        else
            return c_variant_new_int16(c_variant_get_int16(value));

    case C_VARIANT_CLASS_UINT16:
        if (byteswap)
            return c_variant_new_uint16(C_UINT16_SWAP_LE_BE(c_variant_get_uint16(value)));
        else
            return c_variant_new_uint16(c_variant_get_uint16(value));

    case C_VARIANT_CLASS_INT32:
        if (byteswap)
            return c_variant_new_int32(C_UINT32_SWAP_LE_BE(c_variant_get_int32(value)));
        else
            return c_variant_new_int32(c_variant_get_int32(value));

    case C_VARIANT_CLASS_UINT32:
        if (byteswap)
            return c_variant_new_uint32(C_UINT32_SWAP_LE_BE(c_variant_get_uint32(value)));
        else
            return c_variant_new_uint32(c_variant_get_uint32(value));

    case C_VARIANT_CLASS_INT64:
        if (byteswap)
            return c_variant_new_int64(C_UINT64_SWAP_LE_BE(c_variant_get_int64(value)));
        else
            return c_variant_new_int64(c_variant_get_int64(value));

    case C_VARIANT_CLASS_UINT64:
        if (byteswap)
            return c_variant_new_uint64(C_UINT64_SWAP_LE_BE(c_variant_get_uint64(value)));
        else
            return c_variant_new_uint64(c_variant_get_uint64(value));

    case C_VARIANT_CLASS_HANDLE:
        if (byteswap)
            return c_variant_new_handle(C_UINT32_SWAP_LE_BE(c_variant_get_handle(value)));
        else
            return c_variant_new_handle(c_variant_get_handle(value));

    case C_VARIANT_CLASS_DOUBLE:
        if (byteswap) {
            /* We have to convert the double to a uint64 here using a union,
             * because a cast will round it numerically. */
            union
            {
                cuint64 u64;
                cdouble dbl;
            } u1, u2;
            u1.dbl = c_variant_get_double(value);
            u2.u64 = C_UINT64_SWAP_LE_BE(u1.u64);
            return c_variant_new_double(u2.dbl);
        }
        else
            return c_variant_new_double(c_variant_get_double(value));

    case C_VARIANT_CLASS_STRING:
        return c_variant_new_string(c_variant_get_string(value, NULL));

    case C_VARIANT_CLASS_OBJECT_PATH:
        return c_variant_new_object_path(c_variant_get_string(value, NULL));

    case C_VARIANT_CLASS_SIGNATURE:
        return c_variant_new_signature(c_variant_get_string(value, NULL));
    }

    c_assert_not_reached();
}
