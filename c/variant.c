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
#include "cstring.h"
#include "hash-table.h"
#include "log.h"
#include "str.h"
#include "thread.h"


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


static cuint _c_variant_type_hash (void const* type);
static bool c_variant_type_check (const CVariantType *type);
static bool _c_variant_type_equal (const CVariantType *type1, const CVariantType *type2);
static void c_variant_type_info_check (const CVariantTypeInfo *info, char container_class);
static CVariantType* c_variant_type_new_tuple_slow (const CVariantType * const *items, cint length);
static bool variant_type_string_scan_internal(const cchar *string, const cchar *limit, const cchar **endPtr,
                                              csize *depth, csize depth_limit);

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

    c_rec_mutex_lock (&c_variant_type_info_lock);
    if (c_variant_type_info_table != NULL) {
        cc_while_locked ();
    }
    empty = (c_variant_type_info_table == NULL || c_hash_table_size (c_variant_type_info_table) == 0);
    c_rec_mutex_unlock (&c_variant_type_info_lock);

    c_assert (empty);
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
