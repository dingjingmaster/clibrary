/*************************************************************************
> FileName: atomic.h
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Thu 06 Sep 2022 21:29:27 AM CST
 ************************************************************************/
#ifndef _ATOMIC_H
#define _ATOMIC_H
#include "macros.h"

C_BEGIN_EXTERN_C
int c_atomic_int_get (const volatile int *atomic);              // ok
void c_atomic_int_set (volatile int* atomic, int newval);       // ok
void c_atomic_int_inc (volatile int* atomic);                   // ok
bool c_atomic_int_dec_and_test (volatile int* atomic);          // ok
bool c_atomic_int_compare_and_exchange (volatile int* atomic, int oldval, int newval);  // ok
bool c_atomic_int_compare_and_exchange_full (int* atomic, int oldval, int newval, int* preval); //ok
int c_atomic_int_exchange (int* atomic, int newval);            // ok
int c_atomic_int_add (volatile int* atomic, int val);           // ok
cuint c_atomic_int_and (volatile cuint* atomic, cuint val);     // ok
cuint c_atomic_int_or (volatile cuint* atomic, cuint val);      // ok
cuint c_atomic_int_xor (volatile cuint *atomic, cuint val);     // ok
void* c_atomic_pointer_get (const volatile void* atomic);       // ok
void c_atomic_pointer_set (volatile void* atomic, void* newval);// ok
void* c_atomic_pointer_compare_and_exchange (volatile void* atomic, void* oldval, void* newval); //ok
void* c_atomic_pointer_compare_and_exchange_full (void* atomic, void* oldval, void* newval, void* preval); // ok 
void* c_atomic_pointer_exchange (void* atomic, void* newval);   // ok
culong c_atomic_pointer_add (volatile void* atomic, culong val);// ok
culong c_atomic_pointer_and (volatile void* atomic, culong val);// ok
culong c_atomic_pointer_or (volatile void* atomic, culong val); // ok
culong c_atomic_pointer_xor (volatile void* atomic, culong val);// ok
int c_atomic_int_exchange_and_add (volatile int *atomic, int val); //ok

// gcc c11 atomic 扩展属性支持
#if defined(__ATOMIC_SEQ_CST)
#define c_atomic_int_get(atomic) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                    \
    gint gaig_temp;                                                         \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                 \
    __atomic_load ((gint *)(atomic), &gaig_temp, __ATOMIC_SEQ_CST);         \
    (gint) gaig_temp;                                                       \
  }))
#endif

#define c_atomic_int_set(atomic, newval) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                    \
    gint gais_temp = (gint) (newval);                                       \
    (void) (0 ? *(atomic) ^ (newval) : 1);                                  \
    __atomic_store ((gint *)(atomic), &gais_temp, __ATOMIC_SEQ_CST);        \
  }))

#define c_atomic_int_inc(atomic) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                 \
    (void) __atomic_fetch_add ((atomic), 1, __ATOMIC_SEQ_CST);              \
  }))

#define c_atomic_int_dec_and_test(atomic) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                 \
    __atomic_fetch_sub ((atomic), 1, __ATOMIC_SEQ_CST) == 1;                \
  }))

#define c_atomic_int_compare_and_exchange(atomic, oldval, newval) \
  (C_EXTENSION ({                                                           \
    int gaicae_oldval = (oldval);                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (newval) ^ (oldval) : 1);                       \
    __atomic_compare_exchange_n (                                           \
            (atomic), (void *) (&(gaicae_oldval)),                          \
            (newval), FALSE,                                                \
            __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? TRUE : FALSE;             \
  }))

#define c_atomic_int_compare_and_exchange_full\
(atomic, oldval, newval, preval)\
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    C_STATIC_ASSERT (sizeof *(preval) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (newval) ^ (oldval) ^ *(preval) : 1);           \
    *(preval) = (oldval);                                                   \
    __atomic_compare_exchange_n ((atomic), (preval), (newval), FALSE,       \
                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)        \
                                 ? true : false;                            \
  }))

#define c_atomic_int_exchange(atomic, newval) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (newval) : 1);                                  \
    (int) __atomic_exchange_n ((atomic), (newval), __ATOMIC_SEQ_CST);       \
  }))

#define c_atomic_int_add(atomic, val) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                     \
    (int) __atomic_fetch_add ((atomic), (val), __ATOMIC_SEQ_CST);           \
  }))

#define c_atomic_int_and(atomic, val) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                     \
    (cuint) __atomic_fetch_and ((atomic), (val), __ATOMIC_SEQ_CST);         \
  }))

#define c_atomic_int_or(atomic, val) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                     \
    (cuint) __atomic_fetch_or ((atomic), (val), __ATOMIC_SEQ_CST);          \
  }))

#define c_atomic_int_xor(atomic, val) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (int));                     \
    (void) (0 ? *(atomic) ^ (val) : 1);                                     \
    (cuint) __atomic_fetch_xor ((atomic), (val), __ATOMIC_SEQ_CST);         \
  }))

#define c_atomic_pointer_compare_and_exchange(atomic, oldval, newval) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof (oldval) == sizeof (void*));                    \
    void* gapcae_oldval = (void*)(oldval);                                  \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (void*));                   \
    (void) (0 ? (void*) *(atomic) : NULL);                                  \
    __atomic_compare_exchange_n                                             \
    ((atomic), (void *) (&(gapcae_oldval)), (newval), FALSE,                \
     __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? true : false;                    \
  }))

#define c_atomic_pointer_compare_and_exchange_full\
    (atomic, oldval, newval, preval) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (void*));                   \
    C_STATIC_ASSERT (sizeof *(preval) == sizeof (void*));                   \
    (void) (0 ? (void*) *(atomic) : NULL);                                  \
    (void) (0 ? (void*) *(preval) : NULL);                                  \
    *(preval) = (oldval);                                                   \
    __atomic_compare_exchange_n ((atomic), (preval), (newval), FALSE,       \
                                 __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ?      \
                                 true : false;                              \
  }))

#define c_atomic_pointer_exchange(atomic, newval) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (void*));                   \
    (void) (0 ? (void*) *(atomic) : NULL);                                  \
    (void*) __atomic_exchange_n ((atomic), (newval), __ATOMIC_SEQ_CST);     \
  }))

#define c_atomic_pointer_add(atomic, val) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (void*));                   \
    (void) (0 ? (void*) *(atomic) : NULL);                                  \
    (void) (0 ? (val) ^ (val) : 1);                                         \
    (unsigned long) __atomic_fetch_add ((atomic), (val), __ATOMIC_SEQ_CST); \
  }))

#define c_atomic_pointer_and(atomic, val) \
  (C_EXTENSION ({                                                           \
    unsigned long *gapa_atomic = (gsize *) (atomic);                        \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (void*));                   \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (gsize));                   \
    (void) (0 ? (void*) *(atomic) : NULL);                                  \
    (void) (0 ? (val) ^ (val) : 1);                                         \
    (unsigned long) __atomic_fetch_and (gapa_atomic, (val),                 \
            __ATOMIC_SEQ_CST);                                              \
  }))

#define c_atomic_pointer_or(atomic, val) \
  (C_EXTENSION ({                                                           \
    unsigned long* gapo_atomic = (unsigned long*) (atomic);                 \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (void*));                   \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (unsigned long));           \
    (void) (0 ? (void*) *(atomic) : NULL);                                  \
    (void) (0 ? (val) ^ (val) : 1);                                         \
    (unsigned long) __atomic_fetch_or (gapo_atomic, (val),                  \
            __ATOMIC_SEQ_CST);                                              \
  }))

#define c_atomic_pointer_xor(atomic, val) \
  (C_EXTENSION ({                                                           \
    unsigned long* gapx_atomic = (unsigned long*) (atomic);                 \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (void*));                   \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (unsigned long));           \
    (void) (0 ? (void*) *(atomic) : NULL);                                  \
    (void) (0 ? (val) ^ (val) : 1);                                         \
    (unsigned long) __atomic_fetch_xor (gapx_atomic, (val),                 \
            __ATOMIC_SEQ_CST);                                              \
  }))

#define c_atomic_pointer_get(atomic) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (void*));                   \
    void* gapg_temp_newval;                                                 \
    void** gapg_temp_atomic = (void**)(atomic);                             \
    __atomic_load (gapg_temp_atomic, &gapg_temp_newval, __ATOMIC_SEQ_CST);  \
    gapg_temp_newval;                                                       \
  }))

#define c_atomic_pointer_set(atomic, newval) \
  (C_EXTENSION ({                                                           \
    C_STATIC_ASSERT (sizeof *(atomic) == sizeof (void*));                   \
    void** gaps_temp_atomic = (void**)(atomic);                             \
    void* gaps_temp_newval = (void*)(newval);                               \
    (void) (0 ? (gpointer) *(atomic) : NULL);                               \
    __atomic_store (gaps_temp_atomic, &gaps_temp_newval, __ATOMIC_SEQ_CST); \
  }))


C_END_EXTERN_C

#endif
