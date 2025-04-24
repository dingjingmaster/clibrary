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
#include "bytes.h"
#include "cstring.h"
#include "hash-table.h"
#include "log.h"
#include "str.h"
#include "thread.h"
#include "unicode.h"

#define STATE_LOCKED     1
#define STATE_SERIALISED 2
#define STATE_TRUSTED    4
#define STATE_FLOATING   8

#define TYPE_CHECK(value, TYPE, val) \
    if C_UNLIKELY (!c_variant_is_of_type (value, TYPE)) { \
        c_return_if_fail("c_variant_is_of_type (" #value ", " #TYPE ")"); \
        return val; \
    }

#define NUMERIC_TYPE(TYPE, type, ctype) \
    CVariant* c_variant_new_##type (ctype value) { \
        return c_variant_new_from_trusted (C_VARIANT_TYPE_##TYPE, &value, sizeof value); \
    } \
    ctype c_variant_get_##type (CVariant *value) { \
        const ctype *data; \
        TYPE_CHECK (value, C_VARIANT_TYPE_ ## TYPE, 0); \
        data = c_variant_get_data (value); \
        return data != NULL ? *data : 0; \
    }

struct _CVariant
{
    CVariantTypeInfo *type_info;
    csize size;

    union {
        struct {
            CBytes *bytes;
            const void* data;
            csize ordered_offsets_up_to;
            csize checked_offsets_up_to;
        } serialised;

        struct {
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
C_STATIC_ASSERT (C_STRUCT_OFFSET (CVariant, suffix) % 8 == 0);

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

/* == array == */
#define CV_ARRAY_INFO_CLASS 'a'
static void array_info_free (CVariantTypeInfo *info);
static ArrayInfo * CV_ARRAY_INFO (CVariantTypeInfo *info);
static ContainerInfo* array_info_new (const CVariantType *type);

/* == tuple == */
#define CV_TUPLE_INFO_CLASS 'r'
static void tuple_set_base_info (TupleInfo *info);
static void tuple_generate_table (TupleInfo *info);
static void tuple_info_free (CVariantTypeInfo *info);
static csize tuple_align (csize offset, cuint alignment);
static TupleInfo* CV_TUPLE_INFO (CVariantTypeInfo *info);
static ContainerInfo * tuple_info_new (const CVariantType *type);
static bool tuple_get_item (TupleInfo* info, CVariantMemberInfo* item, csize* d, csize* e);
static void tuple_table_append (CVariantMemberInfo **items, csize i, csize a, csize b, csize c);
static void tuple_allocate_members (const CVariantType* type, CVariantMemberInfo **members, csize* n_members);

/* == new/ref/unref == */
static CRecMutex c_variant_type_info_lock;
static CHashTable *c_variant_type_info_table;
static CPtrArray *c_variant_type_info_gc;

#define CC_THRESHOLD 32
static void cc_while_locked (void);

static CVariant* c_variant_new_from_trusted (const CVariantType* type, const void* data, csize size);

static CVariantType* c_variant_make_dict_entry_type (CVariant *key, CVariant *val);
static CVariantType* c_variant_make_tuple_type (CVariant * const *children, csize n_children);

static cuint _c_variant_type_hash (void const* type);
static bool c_variant_type_check (const CVariantType *type);
static bool _c_variant_type_equal (const CVariantType *type1, const CVariantType *type2);
static void c_variant_type_info_check (const CVariantTypeInfo *info, char container_class);
static CVariantType* c_variant_type_new_tuple_slow (const CVariantType * const *items, cint length);
static bool variant_type_string_scan_internal(const cchar *string, const cchar *limit, const cchar **endPtr,
                                              csize *depth, csize depth_limit);

static void c_variant_lock (CVariant *value);
static void c_variant_unlock (CVariant *value);

static const char c_variant_type_info_basic_chars[24][2] = {
    "b", " ", "d", " ", " ", "g", "h", "i", " ", " ", " ", " ",
    "n", "o", " ", "q", " ", "s", "t", "u", "v", " ", "x", "y"
  };

static const CVariantTypeInfo c_variant_type_info_basic_table[24] = {
#define fixed_aligned(x)  x, x - 1, 0
#define not_a_type        0,     0, 0
#define unaligned         0,     0, 0
#define aligned(x)        0, x - 1, 0
    /* 'b' */ { fixed_aligned(1) },   /* boolean */
    /* 'c' */ { not_a_type },
    /* 'd' */ { fixed_aligned(8) },   /* double */
    /* 'e' */ { not_a_type },
    /* 'f' */ { not_a_type },
    /* 'g' */ { unaligned        },   /* signature string */
    /* 'h' */ { fixed_aligned(4) },   /* file handle (int32) */
    /* 'i' */ { fixed_aligned(4) },   /* int32 */
    /* 'j' */ { not_a_type },
    /* 'k' */ { not_a_type },
    /* 'l' */ { not_a_type },
    /* 'm' */ { not_a_type },
    /* 'n' */ { fixed_aligned(2) },   /* int16 */
    /* 'o' */ { unaligned        },   /* object path string */
    /* 'p' */ { not_a_type },
    /* 'q' */ { fixed_aligned(2) },   /* uint16 */
    /* 'r' */ { not_a_type },
    /* 's' */ { unaligned        },   /* string */
    /* 't' */ { fixed_aligned(8) },   /* uint64 */
    /* 'u' */ { fixed_aligned(4) },   /* uint32 */
    /* 'v' */ { aligned(8)       },   /* variant */
    /* 'w' */ { not_a_type },
    /* 'x' */ { fixed_aligned(8) },   /* int64 */
    /* 'y' */ { fixed_aligned(1) },   /* byte */
  #undef fixed_aligned
  #undef not_a_type
  #undef unaligned
  #undef aligned
};

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
    c_return_if_fail (type == NULL || c_variant_type_check (type));

    c_free(type);
}

CVariantType *c_variant_type_copy(const CVariantType *type)
{
    csize length = 0;
    cchar *new = NULL;

    c_return_val_if_fail (c_variant_type_check (type), NULL);

    length = c_variant_type_get_string_length (type);
    new = c_malloc0 (length + 1);

    memcpy (new, type, length);
    new[length] = '\0';

    return (CVariantType*) new;
}

CVariantType *c_variant_type_new(const cchar *typeStr)
{
    c_return_val_if_fail (typeStr != NULL, NULL);

    return c_variant_type_copy (C_VARIANT_TYPE (typeStr));
}

csize c_variant_type_get_string_length(const CVariantType *type)
{
    const cchar *type_string = (const cchar *) type;
    cint brackets = 0;
    csize index = 0;

    c_return_val_if_fail (c_variant_type_check (type), 0);

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
    } while (brackets);

    return index;
}

const cchar *c_variant_type_peek_string(const CVariantType *type)
{
    c_return_val_if_fail (c_variant_type_check (type), NULL);

    return (const cchar *) type;
}

cchar *c_variant_type_dup_string(const CVariantType *type)
{
    c_return_val_if_fail (c_variant_type_check (type), NULL);

    return c_strndup (c_variant_type_peek_string (type), c_variant_type_get_string_length (type));
}

bool c_variant_type_is_definite(const CVariantType *type)
{
    const cchar *type_string;
    csize type_length;
    csize i;

    c_return_val_if_fail (c_variant_type_check (type), false);

    type_length = c_variant_type_get_string_length (type);
    type_string = c_variant_type_peek_string (type);

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

    c_return_val_if_fail (c_variant_type_check (type), false);

    first_char = c_variant_type_peek_string (type)[0];
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

    c_return_val_if_fail (c_variant_type_check (type), false);

    first_char = c_variant_type_peek_string (type)[0];
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
    c_return_val_if_fail (c_variant_type_check (type), false);

    return c_variant_type_peek_string (type)[0] == 'm';
}

bool c_variant_type_is_array(const CVariantType *type)
{
    c_return_val_if_fail (c_variant_type_check (type), false);

    return c_variant_type_peek_string (type)[0] == 'a';
}

bool c_variant_type_is_tuple(const CVariantType *type)
{
    c_return_val_if_fail (c_variant_type_check (type), false);

    const cchar typeChar = c_variant_type_peek_string (type)[0];

    return typeChar == 'r' || typeChar == '(';
}

bool c_variant_type_is_dict_entry(const CVariantType *type)
{
    c_return_val_if_fail (c_variant_type_check (type), false);

    return c_variant_type_peek_string (type)[0] == '{';
}

bool c_variant_type_is_variant(const CVariantType *type)
{
    c_return_val_if_fail (c_variant_type_check (type), false);

    return c_variant_type_peek_string (type)[0] == 'v';
}

cuint c_variant_type_hash(void const *type)
{
    c_return_val_if_fail (c_variant_type_check (type), 0);

    return _c_variant_type_hash (type);
}

bool c_variant_type_equal(void const *type1, void const *type2)
{
    c_return_val_if_fail (c_variant_type_check (type1), false);
    c_return_val_if_fail (c_variant_type_check (type2), false);

    return _c_variant_type_equal (type1, type2);
}

bool c_variant_type_is_subtype_of(const CVariantType *type, const CVariantType *supertype)
{
    const cchar *supertype_string;
    const cchar *supertype_end;
    const cchar *type_string;

    c_return_val_if_fail (c_variant_type_check (type), false);
    c_return_val_if_fail (c_variant_type_check (supertype), false);

    supertype_string = c_variant_type_peek_string (supertype);
    type_string = c_variant_type_peek_string (type);

    /* fast path for the basic determinate types */
    if (type_string[0] == supertype_string[0]) {
        switch (type_string[0]) {
        case 'b': case 'y':
        case 'n': case 'q':
        case 'i': case 'h': case 'u':
        case 't': case 'x':
        case 's': case 'o': case 'g':
        case 'd':
            return true;
        default:
            break;
        }
    }

    supertype_end = supertype_string + c_variant_type_get_string_length (supertype);

    while (supertype_string < supertype_end) {
        char supertype_char = *supertype_string++;
        if (supertype_char == *type_string) {
            type_string++;
        }
        else if (*type_string == ')') {
            return false;
        }
        else {
            const CVariantType *target_type = (CVariantType*) type_string;
            switch (supertype_char) {
            case 'r':
                if (!c_variant_type_is_tuple (target_type)) {
                    return false;
                }
                break;
            case '*':
                break;
            case '?':
                if (!c_variant_type_is_basic (target_type)) {
                    return false;
                }
                break;
            default:
                return false;
            }
            type_string += c_variant_type_get_string_length (target_type);
        }
    }

    return true;
}

const CVariantType *c_variant_type_element(const CVariantType *type)
{
    const cchar *type_string;

    c_return_val_if_fail (c_variant_type_check (type), NULL);

    type_string = c_variant_type_peek_string (type);

    c_assert (type_string[0] == 'a' || type_string[0] == 'm');

    return (const CVariantType *) &type_string[1];
}

const CVariantType *c_variant_type_first(const CVariantType *type)
{
    const cchar *type_string;

    c_return_val_if_fail (c_variant_type_check (type), NULL);

    type_string = c_variant_type_peek_string (type);
    c_assert (type_string[0] == '(' || type_string[0] == '{');

    if (type_string[1] == ')') {
        return NULL;
    }

    return (const CVariantType *) &type_string[1];
}

const CVariantType *c_variant_type_next(const CVariantType *type)
{
    const cchar *type_string;

    c_return_val_if_fail (c_variant_type_check (type), NULL);

    type_string = c_variant_type_peek_string (type);
    type_string += c_variant_type_get_string_length (type);

    if (*type_string == ')' || *type_string == '}') {
        return NULL;
    }

    return (const CVariantType *) type_string;
}

csize c_variant_type_n_items(const CVariantType *type)
{
    csize count = 0;

    c_return_val_if_fail (c_variant_type_check (type), 0);

    for (type = c_variant_type_first (type); type; type = c_variant_type_next (type)) {
        count++;
    }

    return count;
}

const CVariantType *c_variant_type_key(const CVariantType *type)
{
    const cchar *type_string;

    c_return_val_if_fail (c_variant_type_check (type), NULL);

    type_string = c_variant_type_peek_string (type);
    c_assert (type_string[0] == '{');

    return (const CVariantType *) &type_string[1];
}

const CVariantType *c_variant_type_value(const CVariantType *type)
{
#ifndef G_DISABLE_ASSERT
    const cchar *type_string;
#endif

    c_return_val_if_fail (c_variant_type_check (type), NULL);

#ifndef G_DISABLE_ASSERT
    type_string = c_variant_type_peek_string (type);
    c_assert (type_string[0] == '{');
#endif

    return c_variant_type_next (c_variant_type_key (type));
}

CVariantType *c_variant_type_new_array(const CVariantType *element)
{
    csize size;
    cchar *new;

    c_return_val_if_fail (c_variant_type_check (element), NULL);

    size = c_variant_type_get_string_length (element);
    new = c_malloc0 (size + 1);

    new[0] = 'a';
    memcpy (new + 1, element, size);

    return (CVariantType*) new;
}

CVariantType *c_variant_type_new_maybe(const CVariantType *element)
{
    csize size;
    cchar *new;

    c_return_val_if_fail (c_variant_type_check (element), NULL);

    size = c_variant_type_get_string_length (element);
    new = c_malloc0 (size + 1);

    new[0] = 'm';
    memcpy (new + 1, element, size);

    return (CVariantType*) new;
}

CVariantType *c_variant_type_new_tuple(const CVariantType *const *items, cint length)
{
    cchar buffer[1024];
    csize i;
    csize offset;
    csize length_unsigned;

    c_return_val_if_fail (length == 0 || items != NULL, NULL);

    if (length < 0) {
        for (length_unsigned = 0; items[length_unsigned] != NULL; length_unsigned++);
    }
    else {
        length_unsigned = (csize) length;
    }

    offset = 0;
    buffer[offset++] = '(';

    for (i = 0; i < length_unsigned; i++) {
        const CVariantType *type;
        csize size;
        c_return_val_if_fail (c_variant_type_check (items[i]), NULL);

        type = items[i];
        size = c_variant_type_get_string_length (type);

        if (offset + size >= sizeof buffer) {
            return c_variant_type_new_tuple_slow (items, length_unsigned);
        }
        memcpy (&buffer[offset], type, size);
        offset += size;
    }

    c_assert (offset < sizeof buffer);
    buffer[offset++] = ')';

    return (CVariantType *) c_strndup(buffer, offset);
}

CVariantType *c_variant_type_new_dict_entry(const CVariantType *key, const CVariantType *value)
{
    csize keysize, valsize;
    cchar *new;

    c_return_val_if_fail (c_variant_type_check (key), NULL);
    c_return_val_if_fail (c_variant_type_check (value), NULL);

    keysize = c_variant_type_get_string_length (key);
    valsize = c_variant_type_get_string_length (value);

    new = c_malloc0 (1 + keysize + valsize + 1);

    new[0] = '{';
    memcpy (new + 1, key, keysize);
    memcpy (new + 1 + keysize, value, valsize);
    new[1 + keysize + valsize] = '}';

    return (CVariantType*) new;
}

const CVariantType *c_variant_type_checked_(const cchar *typeStr)
{
    c_return_val_if_fail (c_variant_type_string_is_valid (typeStr), NULL);
    return (const CVariantType *) typeStr;
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
    c_variant_type_info_check (info, 0);

    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *) info;

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

void c_variant_type_info_query(CVariantTypeInfo* info, cuint *alignment, csize *size)
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
    c_variant_type_info_check (info, 0);

    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *) info;
        return c_variant_type_string_get_depth_ (container->type_string);
    }

    return 1;
}

CVariantTypeInfo *c_variant_type_info_element(CVariantTypeInfo *info)
{
    return CV_ARRAY_INFO (info)->element;
}

void c_variant_type_info_query_element(CVariantTypeInfo *info, cuint *alignment, csize *size)
{
    c_variant_type_info_query (CV_ARRAY_INFO (info)->element, alignment, size);
}

csize c_variant_type_info_n_members(CVariantTypeInfo *info)
{
    return CV_TUPLE_INFO (info)->n_members;
}

const CVariantMemberInfo *c_variant_type_info_member_info(CVariantTypeInfo *info, csize index)
{
    TupleInfo *tuple_info = CV_TUPLE_INFO (info);

    if (index < tuple_info->n_members) {
        return &tuple_info->members[index];
    }

    return NULL;
}

CVariantTypeInfo *c_variant_type_info_get(const CVariantType *type)
{
    const cchar *type_string = c_variant_type_peek_string (type);
    const char type_char = type_string[0];

    if (type_char == C_VARIANT_TYPE_INFO_CHAR_MAYBE
        || type_char == C_VARIANT_TYPE_INFO_CHAR_ARRAY
        || type_char == C_VARIANT_TYPE_INFO_CHAR_TUPLE
        || type_char == C_VARIANT_TYPE_INFO_CHAR_DICT_ENTRY) {
        CVariantTypeInfo *info;
        c_rec_mutex_lock (&c_variant_type_info_lock);
        if (c_variant_type_info_table == NULL) {
            c_variant_type_info_table = c_hash_table_new ((CHashFunc)_c_variant_type_hash, (CEqualFunc)_c_variant_type_equal);
            c_ignore_leak(c_variant_type_info_table);
        }
        info = c_hash_table_lookup (c_variant_type_info_table, type_string);

        if (info == NULL) {
            ContainerInfo *container;
            if (type_char == C_VARIANT_TYPE_INFO_CHAR_MAYBE || type_char == C_VARIANT_TYPE_INFO_CHAR_ARRAY) {
                container = array_info_new (type);
            }
            else {
                container = tuple_info_new (type);
            }

            info = (CVariantTypeInfo *) container;
            container->type_string = c_variant_type_dup_string (type);
            c_atomic_ref_count_init (&container->ref_count);
            c_hash_table_replace (c_variant_type_info_table, container->type_string, info);
        }
        else {
            c_variant_type_info_ref (info);
        }
        c_rec_mutex_unlock (&c_variant_type_info_lock);
        c_variant_type_info_check (info, 0);
        return info;
    }
    else {
        const CVariantTypeInfo *info;
        int index;

        index = type_char - 'b';
        c_assert (C_N_ELEMENTS (c_variant_type_info_basic_table) == 24);
        c_assert_cmpint (0, <=, index);
        c_assert_cmpint (index, <, 24);

        info = c_variant_type_info_basic_table + index;
        c_variant_type_info_check (info, 0);

        return (CVariantTypeInfo *) info;
    }
}

CVariantTypeInfo *c_variant_type_info_ref(CVariantTypeInfo *info)
{
    c_variant_type_info_check (info, 0);

    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *) info;
        c_atomic_ref_count_inc (&container->ref_count);
    }

    return info;
}

void c_variant_type_info_unref(CVariantTypeInfo *info)
{
    c_variant_type_info_check (info, 0);
    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *) info;
        c_rec_mutex_lock (&c_variant_type_info_lock);
        if (c_atomic_ref_count_dec (&container->ref_count)) {
            if (c_variant_type_info_gc == NULL) {
                c_variant_type_info_gc = c_ptr_array_new ();
                c_ignore_leak (c_variant_type_info_gc);
            }

            c_atomic_ref_count_init (&container->ref_count);
            c_ptr_array_add (c_variant_type_info_gc, info);

            if (c_variant_type_info_gc->len > CC_THRESHOLD) {
                cc_while_locked ();
            }
        }
        c_rec_mutex_unlock (&c_variant_type_info_lock);
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

CVariantTypeInfo *c_variant_get_type_info(CVariant *value)
{
    return value->type_info;
}


NUMERIC_TYPE(BYTE, byte, cuint8)
NUMERIC_TYPE (INT16, int16, cint16)
NUMERIC_TYPE (UINT16, uint16, cuint16)
NUMERIC_TYPE (INT32, int32, cint32)
NUMERIC_TYPE (UINT32, uint32, cuint32)
NUMERIC_TYPE (INT64, int64, cint64)
NUMERIC_TYPE (UINT64, uint64, cuint64)
NUMERIC_TYPE (HANDLE, handle, cint32)
NUMERIC_TYPE (DOUBLE, double, cdouble)


void c_variant_unref(CVariant *value)
{}

CVariant *c_variant_ref(CVariant *value) {}
CVariant *c_variant_ref_sink(CVariant *value) {}
bool c_variant_is_floating(CVariant *value) {}
CVariant *c_variant_take_ref(CVariant *value) {}

const CVariantType *c_variant_get_type(CVariant *value)
{
    CVariantTypeInfo *type_info;

    c_return_val_if_fail (value != NULL, NULL);

    type_info = c_variant_get_type_info (value);

    return (CVariantType*) c_variant_type_info_get_type_string (type_info);
}

const cchar *c_variant_get_type_string(CVariant *value)
{
    CVariantTypeInfo *type_info;

    c_return_val_if_fail (value != NULL, NULL);

    type_info = c_variant_get_type_info (value);

    return c_variant_type_info_get_type_string (type_info);
}

bool c_variant_is_of_type(CVariant *value, const CVariantType *type)
{
    return c_variant_type_is_subtype_of (c_variant_get_type (value), type);
}

bool c_variant_is_container(CVariant *value)
{
    return c_variant_type_is_container (c_variant_get_type (value));
}

CVariantClass c_variant_classify(CVariant *value)
{
    c_return_val_if_fail (value != NULL, 0);

    return* c_variant_get_type_string (value);
}

CVariant *c_variant_new_boolean(bool value)
{
    cuchar v = value;

    return c_variant_new_from_trusted (C_VARIANT_TYPE_BOOLEAN, &v, 1);
}

CVariant *c_variant_new_string(const cchar *str)
{
    const char *endptr = NULL;

    c_return_val_if_fail (str != NULL, NULL);

    if C_LIKELY (c_utf8_validate (str, -1, &endptr)) {
        return c_variant_new_from_trusted (C_VARIANT_TYPE_STRING, str, endptr - str + 1);
    }

    C_LOG_CRIT("c_variant_new_string(): requires valid UTF-8");

    return NULL;
}

CVariant *c_variant_new_take_string(cchar *str)
{
    const char *end = NULL;

    c_return_val_if_fail (str != NULL, NULL);

    if C_LIKELY (c_utf8_validate (str, -1, &end)) {
        CBytes *bytes = c_bytes_new_take (str, end - str + 1);
        return c_variant_new_take_bytes (C_VARIANT_TYPE_STRING, c_steal_pointer (&bytes), true);
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

    c_return_val_if_fail (format_string != NULL, NULL);

    va_start (ap, format_string);
    string = c_strdup_vprintf (format_string, ap);
    va_end (ap);

    bytes = c_bytes_new_take (string, strlen (string) + 1);
    value = c_variant_new_take_bytes (C_VARIANT_TYPE_STRING, c_steal_pointer (&bytes), true);

    return value;
}

CVariant *c_variant_new_object_path(const cchar *object_path)
{
    c_return_val_if_fail (c_variant_is_object_path (object_path), NULL);

    return c_variant_new_from_trusted (C_VARIANT_TYPE_OBJECT_PATH, object_path, strlen (object_path) + 1);
}

bool c_variant_is_object_path(const cchar *str)
{
    c_return_val_if_fail (str != NULL, false);

    return c_variant_serialiser_is_object_path (str, strlen (str) + 1);
}

CVariant *c_variant_new_signature(const cchar *signature)
{
    c_return_val_if_fail (c_variant_is_signature (signature), NULL);

    return c_variant_new_from_trusted (C_VARIANT_TYPE_SIGNATURE, signature, strlen (signature) + 1);
}

bool c_variant_is_signature(const cchar *str)
{
    c_return_val_if_fail (str != NULL, false);

    return c_variant_serialiser_is_signature (str, strlen (str) + 1);
}

CVariant *c_variant_new_variant(CVariant *value)
{
    c_return_val_if_fail (value != NULL, NULL);

    c_variant_ref_sink (value);

    return c_variant_new_from_children (C_VARIANT_TYPE_VARIANT, c_memdup(&value, sizeof value), 1, c_variant_is_trusted (value));
}

CVariant *c_variant_new_strv(const cchar *const *strv, cssize len)
{
    CVariant **strings;
    csize i, length_unsigned;

    c_return_val_if_fail (len == 0 || strv != NULL, NULL);

    if (len < 0)
        len = c_strv_length ((cchar**) strv);
    length_unsigned = len;

    strings = c_malloc0(sizeof(CVariant*) * length_unsigned);
    for (i = 0; i < length_unsigned; i++) {
        strings[i] = c_variant_ref_sink (c_variant_new_string (strv[i]));
    }

    return c_variant_new_from_children (C_VARIANT_TYPE_STRING_ARRAY, strings, length_unsigned, true);
}

CVariant *c_variant_new_objv(const cchar *const *strv, cssize len)
{
    CVariant **strings;
    csize i, length_unsigned;

    c_return_val_if_fail (len == 0 || strv != NULL, NULL);

    if (len < 0)
        len = c_strv_length ((cchar **) strv);
    length_unsigned = len;

    strings = c_malloc0(sizeof(CVariant*) * length_unsigned);
    for (i = 0; i < length_unsigned; i++) {
        strings[i] = c_variant_ref_sink (c_variant_new_object_path (strv[i]));
    }

    return c_variant_new_from_children (C_VARIANT_TYPE_OBJECT_PATH_ARRAY, strings, length_unsigned, true);
}

CVariant *c_variant_new_bytestring(const cchar *str)
{
    c_return_val_if_fail (str != NULL, NULL);

    return c_variant_new_from_trusted (C_VARIANT_TYPE_BYTESTRING, str, strlen (str) + 1);
}

CVariant *c_variant_new_bytestring_array(const cchar *const *strv, cssize len)
{
    CVariant **strings;
    csize i, length_unsigned;

    c_return_val_if_fail (len == 0 || strv != NULL, NULL);

    if (len < 0)
        len = c_strv_length ((cchar **) strv);
    length_unsigned = len;

    strings = c_malloc0(sizeof(CVariant*) * length_unsigned);
    for (i = 0; i < length_unsigned; i++)
        strings[i] = c_variant_ref_sink (c_variant_new_bytestring (strv[i]));

    return c_variant_new_from_children (C_VARIANT_TYPE_BYTESTRING_ARRAY, strings, length_unsigned, true);
}

CVariant *c_variant_new_fixed_array(const CVariantType *element_type, const void *elements, csize n_elements, csize element_size)
{
    CVariantType *array_type;
    csize array_element_size;
    CVariantTypeInfo *array_info;
    CVariant *value;
    void* data;

    c_return_val_if_fail (c_variant_type_is_definite (element_type), NULL);
    c_return_val_if_fail (element_size > 0, NULL);

    array_type = c_variant_type_new_array (element_type);
    array_info = c_variant_type_info_get (array_type);
    c_variant_type_info_query_element (array_info, NULL, &array_element_size);
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
        culong size = sizeof(void*) * n_elements * element_size;
        data = c_malloc0(size);
        memcpy(data, elements, size);
    }
    else {
        data = NULL;
    }
    value = c_variant_new_from_data (array_type, data, n_elements * element_size, false, c_free0, data);

    c_variant_type_free (array_type);
    c_variant_type_info_unref (array_info);

    return value;
}

bool c_variant_get_boolean(CVariant *value)
{
    const cuchar *data;

    TYPE_CHECK (value, C_VARIANT_TYPE_BOOLEAN, false);

    data = c_variant_get_data (value);

    return data != NULL ? *data != 0 : false;
}

CVariant *c_variant_get_variant(CVariant *value)
{
    TYPE_CHECK (value, C_VARIANT_TYPE_VARIANT, NULL);

    return c_variant_get_child_value (value, 0);
}

const cchar *c_variant_get_string(CVariant *value, csize *length)
{
    const void* data;
    csize size;

    c_return_val_if_fail (value != NULL, NULL);
    c_return_val_if_fail (c_variant_is_of_type (value, C_VARIANT_TYPE_STRING)
        || c_variant_is_of_type (value, C_VARIANT_TYPE_OBJECT_PATH)
        || c_variant_is_of_type (value, C_VARIANT_TYPE_SIGNATURE), NULL);

    data = c_variant_get_data (value);
    size = c_variant_get_size (value);

    if (!c_variant_is_trusted (value)) {
        switch (c_variant_classify (value)) {
        case C_VARIANT_CLASS_STRING:
            if (c_variant_serialiser_is_string (data, size)) {
                break;
            }
            data = "";
            size = 1;
            break;
        case C_VARIANT_CLASS_OBJECT_PATH:
            if (c_variant_serialiser_is_object_path (data, size)) {
                break;
            }

            data = "/";
            size = 2;
            break;

        case C_VARIANT_CLASS_SIGNATURE:
            if (c_variant_serialiser_is_signature (data, size)) {
                break;
            }
            data = "";
            size = 1;
            break;

        default:
            c_assert_not_reached ();
        }
    }

    if (length)
        *length = size - 1;

    return data;
}

cchar *c_variant_dup_string(CVariant *value, csize *length)
{
    return c_strdup (c_variant_get_string (value, length));
}

const cchar **c_variant_get_strv(CVariant *value, csize *length)
{
    const cchar **strv;
    csize n;
    csize i;

    TYPE_CHECK (value, C_VARIANT_TYPE_STRING_ARRAY, NULL);

    c_variant_get_data (value);
    n = c_variant_n_children (value);
    strv = c_malloc0(sizeof(const cchar*) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *str = c_variant_get_child_value (value, i);
        strv[i] = c_variant_get_string (str, NULL);
        c_variant_unref (str);
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

    TYPE_CHECK (value, C_VARIANT_TYPE_STRING_ARRAY, NULL);

    n = c_variant_n_children (value);
    strv = c_malloc0(sizeof(cchar*) * (n + 1));
    for (i = 0; i < n; i++) {
        CVariant *str = c_variant_get_child_value (value, i);
        strv[i] = c_variant_dup_string (str, NULL);
        c_variant_unref (str);
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

    TYPE_CHECK (value, C_VARIANT_TYPE_OBJECT_PATH_ARRAY, NULL);

    c_variant_get_data (value);
    n = c_variant_n_children (value);
    strv = c_malloc0(sizeof(const cchar*) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *str = c_variant_get_child_value (value, i);
        strv[i] = c_variant_get_string (str, NULL);
        c_variant_unref (str);
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

    TYPE_CHECK (value, C_VARIANT_TYPE_OBJECT_PATH_ARRAY, NULL);

    n = c_variant_n_children (value);
    strv = c_malloc0(sizeof(cchar*) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *str = c_variant_get_child_value (value, i);
        strv[i] = c_variant_dup_string (str, NULL);
        c_variant_unref (str);
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

    TYPE_CHECK (value, C_VARIANT_TYPE_BYTESTRING, NULL);

    /* Won't be NULL since this is an array type */
    str = c_variant_get_data (value);
    size = c_variant_get_size (value);

    if (size && str[size - 1] == '\0') {
        return str;
    }

    return "";
}

cchar *c_variant_dup_bytestring(CVariant *value, csize *length)
{
    const cchar *original = c_variant_get_bytestring (value);
    csize size;

    /* don't crash in case get_bytestring() had an assert failure */
    if (original == NULL)
        return NULL;

    size = strlen (original);

    if (length)
        *length = size;

    return c_memdup(original, size + 1);
}

const cchar **c_variant_get_bytestring_array(CVariant *value, csize *length)
{
    const cchar **strv;
    csize n;
    csize i;

    TYPE_CHECK (value, C_VARIANT_TYPE_BYTESTRING_ARRAY, NULL);

    c_variant_get_data (value);
    n = c_variant_n_children (value);
    strv = c_malloc0(sizeof(const cchar*) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *string;
        string = c_variant_get_child_value (value, i);
        strv[i] = c_variant_get_bytestring (string);
        c_variant_unref (string);
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

    TYPE_CHECK (value, C_VARIANT_TYPE_BYTESTRING_ARRAY, NULL);

    c_variant_get_data (value);
    n = c_variant_n_children (value);
    strv = c_malloc0(sizeof(cchar*) * (n + 1));

    for (i = 0; i < n; i++) {
        CVariant *string;
        string = c_variant_get_child_value (value, i);
        strv[i] = c_variant_dup_bytestring (string, NULL);
        c_variant_unref (string);
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

    c_return_val_if_fail (child_type == NULL || c_variant_type_is_definite (child_type), 0);
    c_return_val_if_fail (child_type != NULL || child != NULL, NULL);
    c_return_val_if_fail (child_type == NULL || child == NULL || c_variant_is_of_type (child, child_type), NULL);

    if (child_type == NULL) {
        child_type = c_variant_get_type (child);
    }

    maybe_type = c_variant_type_new_maybe (child_type);

    if (child != NULL) {
        CVariant **children;
        bool trusted;

        children = c_malloc0(sizeof(CVariant*));
        children[0] = c_variant_ref_sink (child);
        trusted = c_variant_is_trusted (children[0]);
        value = c_variant_new_from_children (maybe_type, children, 1, trusted);
    }
    else {
        value = c_variant_new_from_children (maybe_type, NULL, 0, true);
    }

    c_variant_type_free (maybe_type);

    return value;
}

CVariant *c_variant_new_array(const CVariantType *child_type, CVariant *const *children, csize n_children)
{
    CVariantType *array_type;
    CVariant **my_children;
    bool trusted;
    CVariant *value;
    csize i;

    c_return_val_if_fail (n_children > 0 || child_type != NULL, NULL);
    c_return_val_if_fail (n_children == 0 || children != NULL, NULL);
    c_return_val_if_fail (child_type == NULL || c_variant_type_is_definite (child_type), NULL);

    my_children = c_malloc0(sizeof(CVariant*) * n_children);
    trusted = true;

    if (child_type == NULL) {
        child_type = c_variant_get_type (children[0]);
    }
    array_type = c_variant_type_new_array (child_type);

    for (i = 0; i < n_children; i++) {
        bool is_of_child_type = c_variant_is_of_type (children[i], child_type);
        if C_UNLIKELY (!is_of_child_type) {
            while (i != 0) {
                c_variant_unref (my_children[--i]);
            }
            c_free (my_children);
            c_return_val_if_fail (is_of_child_type, NULL);
        }
        my_children[i] = c_variant_ref_sink (children[i]);
        trusted &= c_variant_is_trusted (children[i]);
    }

    value = c_variant_new_from_children (array_type, my_children, n_children, trusted);
    c_variant_type_free (array_type);

    return value;
}

CVariant *c_variant_new_tuple(CVariant *const *children, csize n_children)
{
    CVariantType *tuple_type;
    CVariant **my_children;
    bool trusted;
    CVariant *value;
    csize i;

    c_return_val_if_fail (n_children == 0 || children != NULL, NULL);

    my_children = c_malloc0(sizeof(CVariant*) * n_children);
    trusted = true;

    for (i = 0; i < n_children; i++) {
        my_children[i] = c_variant_ref_sink (children[i]);
        trusted &= c_variant_is_trusted (children[i]);
    }

    tuple_type = c_variant_make_tuple_type (children, n_children);
    value = c_variant_new_from_children (tuple_type, my_children, n_children, trusted);
    c_variant_type_free (tuple_type);

    return value;
}

CVariant *c_variant_new_dict_entry(CVariant *key, CVariant *value)
{
    CVariantType *dict_type;
    CVariant **children;
    bool trusted;

    c_return_val_if_fail (key != NULL && value != NULL, NULL);
    c_return_val_if_fail (!c_variant_is_container (key), NULL);

    children = c_malloc0(sizeof(CVariant*) * 2);
    children[0] = c_variant_ref_sink (key);
    children[1] = c_variant_ref_sink (value);
    trusted = c_variant_is_trusted (key) && c_variant_is_trusted (value);

    dict_type = c_variant_make_dict_entry_type (key, value);
    value = c_variant_new_from_children (dict_type, children, 2, trusted);
    c_variant_type_free (dict_type);

    return value;
}

CVariant *c_variant_get_maybe(CVariant *value)
{
    TYPE_CHECK (value, C_VARIANT_TYPE_MAYBE, NULL);

    if (c_variant_n_children (value)) {
        return c_variant_get_child_value (value, 0);
    }

    return NULL;
}

csize c_variant_n_children(CVariant *value) {}
void c_variant_get_child(CVariant *value, csize idx, const cchar *formatStr, ...) {}
CVariant *c_variant_get_child_value(CVariant *value, csize idx) {}

bool c_variant_lookup(CVariant *dictionary, const cchar *key, const cchar *format_string, ...)
{
    CVariantType *type;
    CVariant *value;

    /* flatten */
    c_variant_get_data (dictionary);

    type = c_variant_format_string_scan_type (format_string, NULL, NULL);
    value = c_variant_lookup_value (dictionary, key, type);
    c_variant_type_free (type);

    if (value) {
        va_list ap;

        va_start (ap, format_string);
        c_variant_get_va (value, format_string, NULL, &ap);
        c_variant_unref (value);
        va_end (ap);

        return true;
    }

    return false;
}

CVariant *c_variant_lookup_value(CVariant *dictionary, const cchar *key, const CVariantType *expected_type)
{
    CVariantIter iter;
    CVariant *entry;
    CVariant *value;

    c_return_val_if_fail (c_variant_is_of_type (dictionary, C_VARIANT_TYPE ("a{s*}")) || c_variant_is_of_type (dictionary, C_VARIANT_TYPE ("a{o*}")), NULL);
    c_variant_iter_init (&iter, dictionary);

    while ((entry = c_variant_iter_next_value (&iter))) {
        CVariant *entry_key;
        bool matches;

        entry_key = c_variant_get_child_value (entry, 0);
        matches = strcmp (c_variant_get_string (entry_key, NULL), key) == 0;
        c_variant_unref (entry_key);

        if (matches) {
            break;
        }
        c_variant_unref (entry);
    }

    if (entry == NULL) {
        return NULL;
    }

    value = c_variant_get_child_value (entry, 1);
    c_variant_unref (entry);

    if (c_variant_is_of_type (value, C_VARIANT_TYPE_VARIANT)) {
        CVariant *tmp;
        tmp = c_variant_get_variant (value);
        c_variant_unref (value);
        if (expected_type && !c_variant_is_of_type (tmp, expected_type)) {
            c_variant_unref (tmp);
            tmp = NULL;
        }
        value = tmp;
    }

    c_return_val_if_fail (expected_type == NULL || value == NULL || c_variant_is_of_type (value, expected_type), NULL);

    return value;
}

const void *c_variant_get_fixed_array(CVariant *value, csize *n_elements, csize element_size)
{
    CVariantTypeInfo *array_info;
    csize array_element_size;
    const void* data;
    csize size;

    TYPE_CHECK (value, C_VARIANT_TYPE_ARRAY, NULL);

    c_return_val_if_fail (n_elements != NULL, NULL);
    c_return_val_if_fail (element_size > 0, NULL);

    array_info = c_variant_get_type_info (value);
    c_variant_type_info_query_element (array_info, NULL, &array_element_size);

    c_return_val_if_fail (array_element_size, NULL);

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

    data = c_variant_get_data (value);
    size = c_variant_get_size (value);

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

csize c_variant_get_size(CVariant *value) {}
const void *c_variant_get_data(CVariant *value) {}
CBytes *c_variant_get_data_as_bytes(CVariant *value) {}
void c_variant_store(CVariant *value, void *data) {}

cchar *c_variant_print(CVariant *value, bool type_annotate)
{
    return c_string_free (c_variant_print_string (value, NULL, type_annotate), false);
}

CString *c_variant_print_string(CVariant *value, CString *string, bool type_annotate)
{
    const cchar *value_type_string = c_variant_get_type_string (value);

    if C_UNLIKELY (string == NULL) {
        string = c_string_new (NULL);
    }

    switch (value_type_string[0]) {
        case C_VARIANT_CLASS_MAYBE:
        if (type_annotate) {
            c_string_append_printf (string, "@%s ", value_type_string);
        }
        if (c_variant_n_children (value)) {
            const CVariantType *base_type;
            cuint i, depth;
            CVariant *element = NULL;
            for (depth = 0, base_type = c_variant_get_type (value);
                c_variant_type_is_maybe (base_type);
                depth++, base_type = c_variant_type_element (base_type));
            element = c_variant_ref (value);
            for (i = 0; i < depth && element != NULL; i++) {
                CVariant *new_element = c_variant_n_children (element) ? c_variant_get_child_value (element, 0) : NULL;
                c_variant_unref (element);
                element = c_steal_pointer (&new_element);
            }
            if (element == NULL) {
                for (; i > 1; i--) {
                    c_string_append (string, "just ");
                }
                c_string_append (string, "nothing");
            }
            else {
                c_variant_print_string (element, string, false);
            }
            c_clear_pointer (&element, c_variant_unref);
        }
        else {
            c_string_append (string, "nothing");
        }
        break;
        case C_VARIANT_CLASS_ARRAY:
        if (value_type_string[1] == 'y') {
            const cchar *str;
            csize size;
            csize i;

            str = c_variant_get_data (value);
            size = c_variant_get_size (value);

            for (i = 0; i < size; i++) {
                if (str[i] == '\0') {
                    break;
                }
            }
            if (i == size - 1) {
                cchar *escaped = c_strescape (str, NULL);

                if (strchr (str, '\''))
                    c_string_append_printf (string, "b\"%s\"", escaped);
                else
                    c_string_append_printf (string, "b'%s'", escaped);

                c_free (escaped);
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

            if ((n = c_variant_n_children (value)) == 0) {
                if (type_annotate) {
                    c_string_append_printf (string, "@%s ", value_type_string);
                }
                c_string_append (string, "{}");
                break;
            }

            c_string_append_c (string, '{');
            for (i = 0; i < n; i++) {
                CVariant *entry, *key, *val;
                c_string_append (string, comma);
                comma = ", ";
                entry = c_variant_get_child_value (value, i);
                key = c_variant_get_child_value (entry, 0);
                val = c_variant_get_child_value (entry, 1);
                c_variant_unref (entry);

                c_variant_print_string (key, string, type_annotate);
                c_variant_unref (key);
                c_string_append (string, ": ");
                c_variant_print_string (val, string, type_annotate);
                c_variant_unref (val);
                type_annotate = false;
            }
            c_string_append_c (string, '}');
        }
        else {
            /* normal (non-dictionary) array */
            const cchar *comma = "";
            csize n, i;
            if ((n = c_variant_n_children (value)) == 0) {
                if (type_annotate) {
                    c_string_append_printf (string, "@%s ", value_type_string);
                }
                c_string_append (string, "[]");
                break;
            }
            c_string_append_c (string, '[');
            for (i = 0; i < n; i++) {
                CVariant *element;
                c_string_append (string, comma);
                comma = ", ";
                element = c_variant_get_child_value (value, i);
                c_variant_print_string (element, string, type_annotate);
                c_variant_unref (element);
                type_annotate = false;
            }
            c_string_append_c (string, ']');
        }
        break;
        case C_VARIANT_CLASS_TUPLE: {
            csize n, i;
            n = c_variant_n_children (value);
            c_string_append_c (string, '(');
            for (i = 0; i < n; i++) {
                CVariant *element;
                element = c_variant_get_child_value (value, i);
                c_variant_print_string (element, string, type_annotate);
                c_string_append (string, ", ");
                c_variant_unref (element);
            }
            c_string_truncate (string, string->len - (n > 0) - (n > 1));
            c_string_append_c (string, ')');
        }
        break;
        case C_VARIANT_CLASS_DICT_ENTRY: {
            CVariant *element;
            c_string_append_c (string, '{');
            element = c_variant_get_child_value (value, 0);
            c_variant_print_string (element, string, type_annotate);
            c_variant_unref (element);
            c_string_append (string, ", ");
            element = c_variant_get_child_value (value, 1);
            c_variant_print_string (element, string, type_annotate);
            c_variant_unref (element);
            c_string_append_c (string, '}');
        }
        break;
        case C_VARIANT_CLASS_VARIANT: {
            CVariant *child = c_variant_get_variant (value);
            c_string_append_c (string, '<');
            c_variant_print_string (child, string, true);
            c_string_append_c (string, '>');
            c_variant_unref (child);
        }
        break;
        case C_VARIANT_CLASS_BOOLEAN:
            if (c_variant_get_boolean (value))
                c_string_append (string, "true");
            else
                c_string_append (string, "false");
            break;
        case C_VARIANT_CLASS_STRING: {
            const cchar *str = c_variant_get_string (value, NULL);
            cunichar quote = strchr (str, '\'') ? '"' : '\'';
            c_string_append_c (string, quote);
            while (*str) {
                cunichar c = c_utf8_get_char (str);
                if (c == quote || c == '\\') {
                    c_string_append_c (string, '\\');
                }
                if (c_unichar_isprint (c)) {
                    c_string_append_unichar (string, c);
                }
                else {
                    c_string_append_c (string, '\\');
                    if (c < 0x10000) {
                        switch (c) {
                            case '\a':
                                c_string_append_c (string, 'a');
                                break;
                            case '\b':
                                c_string_append_c (string, 'b');
                                break;
                            case '\f':
                                c_string_append_c (string, 'f');
                                break;
                            case '\n':
                                c_string_append_c (string, 'n');
                                break;
                            case '\r':
                                c_string_append_c (string, 'r');
                                break;
                            case '\t':
                                c_string_append_c (string, 't');
                                break;
                            case '\v':
                                c_string_append_c (string, 'v');
                                break;
                            default:
                                c_string_append_printf (string, "u%04x", c);
                                break;
                        }
                    }
                    else {
                        c_string_append_printf (string, "U%08x", c);
                    }
                }
                str = c_utf8_next_char (str);
            }
            c_string_append_c (string, quote);
        }
        break;
        case C_VARIANT_CLASS_BYTE:
            if (type_annotate) {
                c_string_append (string, "byte ");
            }
            c_string_append_printf (string, "0x%02x", c_variant_get_byte (value));
        break;
        case C_VARIANT_CLASS_INT16:
            if (type_annotate) {
                c_string_append (string, "int16 ");
            }
            c_string_append_printf (string, "%d", c_variant_get_int16 (value));
        break;
        case C_VARIANT_CLASS_UINT16:
            if (type_annotate) {
                c_string_append (string, "uint16 ");
            }
            c_string_append_printf (string, "%u", c_variant_get_uint16 (value));
            break;
        case C_VARIANT_CLASS_INT32:
            c_string_append_printf (string, "%d", c_variant_get_int32 (value));
            break;
        case C_VARIANT_CLASS_HANDLE:
            if (type_annotate) {
                c_string_append (string, "handle ");
            }
            c_string_append_printf (string, "%d", c_variant_get_handle (value));
            break;
        case C_VARIANT_CLASS_UINT32:
            if (type_annotate) {
                c_string_append (string, "uint32 ");
            }
            c_string_append_printf (string, "%u", c_variant_get_uint32 (value));
            break;

        case C_VARIANT_CLASS_INT64:
            if (type_annotate) {
                c_string_append (string, "int64 ");
            }
            c_string_append_printf (string, "%ld", c_variant_get_int64 (value));
            break;
        case C_VARIANT_CLASS_UINT64:
            if (type_annotate) {
                c_string_append (string, "uint64 ");
            }
            c_string_append_printf (string, "%lu", c_variant_get_uint64 (value));
            break;
        case C_VARIANT_CLASS_DOUBLE: {
            cchar buffer[100];
            cint i;
            c_ascii_dtostr (buffer, sizeof buffer, c_variant_get_double (value));
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
            c_string_append (string, buffer);
            break;
        }
        case C_VARIANT_CLASS_OBJECT_PATH: {
            if (type_annotate) {
                c_string_append (string, "objectpath ");
            }
            c_string_append_printf (string, "\'%s\'", c_variant_get_string (value, NULL));
            break;
        }
        case C_VARIANT_CLASS_SIGNATURE: {
            if (type_annotate) {
                c_string_append (string, "signature ");
            }
            c_string_append_printf (string, "\'%s\'", c_variant_get_string (value, NULL));
            break;
        }
        default: {
            c_assert_not_reached ();
        }
    }

    return string;
}

cuint c_variant_hash(const void *value_)
{
    CVariant *value = (CVariant *) value_;

    switch (c_variant_classify (value))
    {
    case C_VARIANT_CLASS_STRING:
    case C_VARIANT_CLASS_OBJECT_PATH:
    case C_VARIANT_CLASS_SIGNATURE:
        return c_str_hash (c_variant_get_string (value, NULL));

    case C_VARIANT_CLASS_BOOLEAN:
        /* this is a very odd thing to hash... */
        return c_variant_get_boolean (value);

    case C_VARIANT_CLASS_BYTE:
        return c_variant_get_byte (value);

    case C_VARIANT_CLASS_INT16:
    case C_VARIANT_CLASS_UINT16:
    {
        const cuint16 *ptr;

        ptr = c_variant_get_data (value);

        if (ptr)
            return *ptr;
        else
            return 0;
    }

    case C_VARIANT_CLASS_INT32:
    case C_VARIANT_CLASS_UINT32:
    case C_VARIANT_CLASS_HANDLE:
    {
        const cuint *ptr;

        ptr = c_variant_get_data (value);

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

        ptr = c_variant_get_data (value);

        if (ptr)
            return ptr[0] + ptr[1];
        else
            return 0;
    }

    default:
        c_return_val_if_fail (!c_variant_is_container (value), 0);
        c_assert_not_reached ();
    }
}

bool c_variant_equal(const void *one, const void *two)
{
    bool equal;

    c_return_val_if_fail (one != NULL && two != NULL, false);

    if (c_variant_get_type_info ((CVariant*) one) != c_variant_get_type_info ((CVariant*) two)) {
        return false;
    }

    if (c_variant_is_trusted ((CVariant *) one) && c_variant_is_trusted ((CVariant *) two))
    {
        const void* data_one;
        const void* data_two;
        csize size_one, size_two;

        size_one = c_variant_get_size ((CVariant*) one);
        size_two = c_variant_get_size ((CVariant*) two);

        if (size_one != size_two) {
            return false;
        }

        data_one = c_variant_get_data ((CVariant*) one);
        data_two = c_variant_get_data ((CVariant*) two);

        if (size_one)
            equal = memcmp (data_one, data_two, size_one) == 0;
        else
            equal = true;
    }
    else {
        cchar *strone, *strtwo;
        strone = c_variant_print ((CVariant *) one, false);
        strtwo = c_variant_print ((CVariant *) two, false);
        equal = strcmp (strone, strtwo) == 0;
        c_free (strone);
        c_free (strtwo);
    }

    return equal;
}

CVariant *c_variant_get_normal_form(CVariant *value) {}
bool c_variant_is_normal_form(CVariant *value) {}
CVariant *c_variant_byteswap(CVariant *value) {}
CVariant *c_variant_new_from_bytes(const CVariantType *type, CBytes *bytes, bool trusted) {}
CVariant *c_variant_new_from_data(const CVariantType *type, const void *data, csize size, bool trusted,
                                  CDestroyNotify notify, void *user_data)
{
}
CVariantIter *c_variant_iter_new(CVariant *value) {}
csize c_variant_iter_init(CVariantIter *iter, CVariant *value) {}
CVariantIter *c_variant_iter_copy(CVariantIter *iter) {}
csize c_variant_iter_n_children(CVariantIter *iter) {}
void c_variant_iter_free(CVariantIter *iter) {}
CVariant *c_variant_iter_next_value(CVariantIter *iter) {}
bool c_variant_iter_next(CVariantIter *iter, const cchar *formatStr, ...) {}
bool c_variant_iter_loop(CVariantIter *iter, const cchar *formatStr, ...) {}
CVariantBuilder *c_variant_builder_new(const CVariantType *type) {}
void c_variant_builder_unref(CVariantBuilder *builder) {}
CVariantBuilder *c_variant_builder_ref(CVariantBuilder *builder) {}
void c_variant_builder_init(CVariantBuilder *builder, const CVariantType *type) {}
void c_variant_builder_init_static(CVariantBuilder *builder, const CVariantType *type) {}
CVariant *c_variant_builder_end(CVariantBuilder *builder) {}
void c_variant_builder_clear(CVariantBuilder *builder) {}
void c_variant_builder_open(CVariantBuilder *builder, const CVariantType *type) {}
void c_variant_builder_close(CVariantBuilder *builder) {}
void c_variant_builder_add_value(CVariantBuilder *builder, CVariant *value) {}
void c_variant_builder_add(CVariantBuilder *builder, const cchar *formatStr, ...) {}
void c_variant_builder_add_parsed(CVariantBuilder *builder, const cchar *format, ...) {}
CVariant *c_variant_new(const cchar *formatStr, ...) {}
void c_variant_get(CVariant *value, const cchar *formatStr, ...) {}
CVariant *c_variant_new_va(const cchar *formatStr, const cchar **endPtr, va_list *app) {}
void c_variant_get_va(CVariant *value, const cchar *formatStr, const cchar **endPtr, va_list *app) {}
bool c_variant_check_format_string(CVariant *value, const cchar *formatStr, bool copyOnly) {}
CVariant *c_variant_parse(const CVariantType *type, const cchar *text, const cchar *limit, const cchar **endPtr,
                          CError **error)
{
}
CVariant *c_variant_new_parsed(const cchar *format, ...) {}
CVariant *c_variant_new_parsed_va(const cchar *format, va_list *app) {}
cchar *c_variant_parse_error_print_context(CError *error, const cchar *sourceStr) {}
cint c_variant_compare(const void *one, const void *two) {}
CVariantDict *c_variant_dict_new(CVariant *fromAsv) {}
void c_variant_dict_init(CVariantDict *dict, CVariant *fromAsv) {}
bool c_variant_dict_lookup(CVariantDict *dict, const cchar *key, const cchar *formatStr, ...) {}
CVariant *c_variant_dict_lookup_value(CVariantDict *dict, const cchar *key, const CVariantType *expectedType) {}
bool c_variant_dict_contains(CVariantDict *dict, const cchar *key) {}
void c_variant_dict_insert(CVariantDict *dict, const cchar *key, const cchar *formatStr, ...) {}
void c_variant_dict_insert_value(CVariantDict *dict, const cchar *key, CVariant *value) {}
bool c_variant_dict_remove(CVariantDict *dict, const cchar *key) {}
void c_variant_dict_clear(CVariantDict *dict) {}
CVariant *c_variant_dict_end(CVariantDict *dict) {}
CVariantDict *c_variant_dict_ref(CVariantDict *dict) {}
void c_variant_dict_unref(CVariantDict *dict) {}


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

static bool c_variant_type_check (const CVariantType *type)
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

static cuint _c_variant_type_hash (void const* type)
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

static bool _c_variant_type_equal (const CVariantType *type1, const CVariantType *type2)
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
    } while (brackets);

    return true;
}

static CVariantType* c_variant_type_new_tuple_slow (const CVariantType * const *items, cint length)
{
    CString *string;
    cint i;

    string = c_string_new ("(");
    for (i = 0; i < length; i++) {
        const CVariantType *type;
        csize size;
        c_return_val_if_fail (c_variant_type_check (items[i]), NULL);
        type = items[i];
        size = c_variant_type_get_string_length (type);
        c_string_append_len (string, (const cchar *) type, size);
    }
    c_string_append_c (string, ')');

    return (CVariantType*) c_string_free (string, false);
}

static void c_variant_type_info_check (const CVariantTypeInfo *info, char container_class)
{
    c_assert (!container_class || info->container_class == container_class);

    /* alignment can only be one of these */
    c_assert (info->alignment == 0 || info->alignment == 1 ||
              info->alignment == 3 || info->alignment == 7);

    if (info->container_class) {
        ContainerInfo *container = (ContainerInfo *) info;

        /* extra checks for containers */
        c_assert (!c_atomic_ref_count_compare (&container->ref_count, 0));
        c_assert (container->type_string != NULL);
    }
    else {
        cint index;

        /* if not a container, then ensure that it is a valid member of
         * the basic types table
         */
        index = info - c_variant_type_info_basic_table;

        c_assert (C_N_ELEMENTS (c_variant_type_info_basic_table) == 24);
        c_assert (C_N_ELEMENTS (c_variant_type_info_basic_chars) == 24);
        c_assert (0 <= index && index < 24);
        c_assert (c_variant_type_info_basic_chars[index][0] != ' ');
    }
}

/* == array == */
static ArrayInfo * CV_ARRAY_INFO (CVariantTypeInfo *info)
{
    c_variant_type_info_check (info, CV_ARRAY_INFO_CLASS);

    return (ArrayInfo *) info;
}


static void array_info_free (CVariantTypeInfo *info)
{
    ArrayInfo *array_info;

    c_assert (info->container_class == CV_ARRAY_INFO_CLASS);
    array_info = (ArrayInfo *) info;

    c_variant_type_info_unref (array_info->element);
    c_free0(array_info);
}

static ContainerInfo* array_info_new (const CVariantType *type)
{
    ArrayInfo *info;

    info = c_malloc0(sizeof(ArrayInfo));
    info->container.info.container_class = CV_ARRAY_INFO_CLASS;

    info->element = c_variant_type_info_get (c_variant_type_element (type));
    info->container.info.alignment = info->element->alignment;
    info->container.info.fixed_size = 0;

    return (ContainerInfo *) info;
}

/* == tuple == */
static TupleInfo* CV_TUPLE_INFO (CVariantTypeInfo *info)
{
    c_variant_type_info_check (info, CV_TUPLE_INFO_CLASS);

    return (TupleInfo *) info;
}

static void tuple_info_free (CVariantTypeInfo *info)
{
    TupleInfo *tuple_info;
    csize i;

    c_assert (info->container_class == CV_TUPLE_INFO_CLASS);
    tuple_info = (TupleInfo *) info;

    for (i = 0; i < tuple_info->n_members; i++) {
        c_variant_type_info_unref (tuple_info->members[i].type_info);
    }

    c_free0(tuple_info->members);
    c_free0(tuple_info);
}

static void tuple_allocate_members (const CVariantType* type, CVariantMemberInfo **members, csize* n_members)
{
    const CVariantType *item_type;
    csize i = 0;

    *n_members = c_variant_type_n_items (type);
    *members = c_malloc0(sizeof (CVariantMemberInfo) * *n_members);

    item_type = c_variant_type_first (type);
    while (item_type) {
        CVariantMemberInfo *member = &(*members)[i++];
        member->type_info = c_variant_type_info_get (item_type);
        item_type = c_variant_type_next (item_type);

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

    c_assert (i == *n_members);
}

static bool tuple_get_item (TupleInfo* info, CVariantMemberInfo* item, csize* d, csize* e)
{
    if (&info->members[info->n_members] == item) {
        return false;
    }

    *d = item->type_info->alignment;
    *e = item->type_info->fixed_size;

    return true;
}

static void tuple_table_append (CVariantMemberInfo **items, csize i, csize a, csize b, csize c)
{
    CVariantMemberInfo *item = (*items)++;

    a += ~b & c;    /* take the "aligned" part of 'c' and add to 'a' */
    c &= b;         /* chop 'c' to contain only the unaligned part */


    item->i = i;
    item->a = a + b;
    item->b = ~b;
    item->c = c;
}

static csize tuple_align (csize offset, cuint alignment)
{
    return offset + ((-offset) & alignment);
}

static void tuple_generate_table (TupleInfo *info)
{
    CVariantMemberInfo *items = info->members;
    csize i = -1, a = 0, b = 0, c = 0, d, e;

    while (tuple_get_item (info, items, &d, &e)) {
        if (d <= b) {
            c = tuple_align (c, d);                   /* rule 1 */
        }
        else {
            a += tuple_align (c, b), b = d, c = 0;    /* rule 2 */
        }

        tuple_table_append (&items, i, a, b, c);

        if (e == 0) {
            i++, a = b = c = 0;
        }
        else {
            c += e;                                                 /* rule 3 */
        }
    }
}

static void tuple_set_base_info (TupleInfo *info)
{
    CVariantTypeInfo *base = &info->container.info;

    if (info->n_members > 0) {
        CVariantMemberInfo *m;
        base->alignment = 0;
        for (m = info->members; m < &info->members[info->n_members]; m++) {
            base->alignment |= m->type_info->alignment;
        }

        m--; /* take 'm' back to the last item */
        if (m->i == (csize) -1 && m->type_info->fixed_size) {
            base->fixed_size = tuple_align (((m->a & m->b) | m->c) + m->type_info->fixed_size, base->alignment);
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

static ContainerInfo * tuple_info_new (const CVariantType *type)
{
    TupleInfo *info;

    info = c_malloc0(sizeof(TupleInfo));
    info->container.info.container_class = CV_TUPLE_INFO_CLASS;

    tuple_allocate_members (type, &info->members, &info->n_members);
    tuple_generate_table (info);
    tuple_set_base_info (info);

    return (ContainerInfo *) info;
}

/* == new/ref/unref == */
static void cc_while_locked (void)
{
    while (c_variant_type_info_gc->len > 0) {
        CVariantTypeInfo *info = c_ptr_array_steal_index_fast (c_variant_type_info_gc, 0);
        ContainerInfo *container = (ContainerInfo *)info;
        if (c_atomic_ref_count_dec (&container->ref_count)) {
            c_hash_table_remove (c_variant_type_info_table, container->type_string);
            c_free (container->type_string);

            if (info->container_class == CV_ARRAY_INFO_CLASS)
                array_info_free (info);
            else if (info->container_class == CV_TUPLE_INFO_CLASS)
                tuple_info_free (info);
            else
                c_assert_not_reached ();
        }
    }
}

//
static CVariant* c_variant_new_from_trusted (const CVariantType* type, const void* data, csize size)
{
    if (size <= C_VARIANT_MAX_PREALLOCATED) {
        return c_variant_new_preallocated_trusted (type, data, size);
    }
    else {
        return c_variant_new_take_bytes (type, c_bytes_new (data, size), true);
    }
}

static CVariantType* c_variant_make_tuple_type (CVariant * const *children, csize n_children)
{
    const CVariantType **types;
    CVariantType *type;
    csize i;

    types = c_malloc0(sizeof(const CVariantType*) * n_children);

    for (i = 0; i < n_children; i++) {
        types[i] = c_variant_get_type (children[i]);
    }

    type = c_variant_type_new_tuple (types, n_children);
    c_free (types);

    return type;
}

static CVariantType* c_variant_make_dict_entry_type (CVariant *key, CVariant *val)
{
    return c_variant_type_new_dict_entry (c_variant_get_type (key), c_variant_get_type (val));
}

static void c_variant_lock (CVariant *value)
{
    c_bit_lock (&value->state, 0);
}

static void c_variant_unlock (CVariant *value)
{
    c_bit_unlock (&value->state, 0);
}