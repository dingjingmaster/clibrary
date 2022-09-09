/*************************************************************************
> FileName: atomic.c
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Thu 07 Sep 2022 22:20:19 PM CST
************************************************************************/

#include "atomic.h"

#include <pthread.h>

static pthread_mutex_t g_atomic_lock = PTHREAD_MUTEX_INITIALIZER;

int (c_atomic_int_get) (const volatile int *atomic)
{
    int value;

    pthread_mutex_lock (&g_atomic_lock);
    value = *atomic;
    pthread_mutex_unlock (&g_atomic_lock);

    return value;
}

void (c_atomic_int_set) (volatile int *atomic, int value)
{
    pthread_mutex_lock (&g_atomic_lock);
    *atomic = value;
    pthread_mutex_unlock (&g_atomic_lock);
}

void (c_atomic_int_inc) (volatile int *atomic)
{
    pthread_mutex_lock (&g_atomic_lock);
    (*atomic)++;
    pthread_mutex_unlock (&g_atomic_lock);
}

bool (c_atomic_int_dec_and_test) (volatile int *atomic)
{
    bool isZero;

    pthread_mutex_lock (&g_atomic_lock);
    isZero = --(*atomic) == 0;
    pthread_mutex_unlock (&g_atomic_lock);

    return isZero;
}

bool (c_atomic_int_compare_and_exchange) (volatile int *atomic, int oldval, int newval)
{
    bool success;

    pthread_mutex_lock (&g_atomic_lock);

    if ((success = (*atomic == oldval)))
        *atomic = newval;

    pthread_mutex_unlock (&g_atomic_lock);

    return success;
}

bool (c_atomic_int_compare_and_exchange_full) (int *atomic, int oldval, int newval, int* preval)
{
    bool     success;

    pthread_mutex_lock (&g_atomic_lock);

    *preval = *atomic;

    if ((success = (*atomic == oldval)))
        *atomic = newval;

    pthread_mutex_unlock (&g_atomic_lock);

    return success;
}

int (c_atomic_int_exchange) (int *atomic, int newval)
{
    int *ptr = atomic;
    int oldval;

    pthread_mutex_lock (&g_atomic_lock);
    oldval = *ptr;
    *ptr = newval;
    pthread_mutex_unlock (&g_atomic_lock);

    return oldval;
}

int (c_atomic_int_add) (volatile int *atomic, int val)
{
    int oldval;

    pthread_mutex_lock (&g_atomic_lock);
    oldval = *atomic;
    *atomic = oldval + val;
    pthread_mutex_unlock (&g_atomic_lock);

    return oldval;
}

cuint (c_atomic_int_and) (volatile cuint *atomic, cuint val)
{
    cuint oldval;

    pthread_mutex_lock (&g_atomic_lock);
    oldval = *atomic;
    *atomic = oldval & val;
    pthread_mutex_unlock (&g_atomic_lock);

    return oldval;
}

cuint (c_atomic_int_or) (volatile cuint *atomic, cuint val)
{
    cuint oldval;

    pthread_mutex_lock (&g_atomic_lock);
    oldval = *atomic;
    *atomic = oldval | val;
    pthread_mutex_unlock (&g_atomic_lock);

    return oldval;
}

cuint (c_atomic_int_xor) (volatile cuint *atomic, cuint val)
{
    cuint oldval;

    pthread_mutex_lock (&g_atomic_lock);
    oldval = *atomic;
    *atomic = oldval ^ val;
    pthread_mutex_unlock (&g_atomic_lock);

    return oldval;
}

void* (c_atomic_pointer_get) (const volatile void* atomic)
{
    const void** ptr = atomic;
    void* value;

    pthread_mutex_lock (&g_atomic_lock);
    value = *ptr;
    pthread_mutex_unlock (&g_atomic_lock);

    return value;
}

void
(g_atomic_pointer_set) (volatile void *atomic,
                        gpointer       newval)
{
  gpointer *ptr = atomic;

  pthread_mutex_lock (&g_atomic_lock);
  *ptr = newval;
  pthread_mutex_unlock (&g_atomic_lock);
}

gboolean
(g_atomic_pointer_compare_and_exchange) (volatile void *atomic,
                                         gpointer       oldval,
                                         gpointer       newval)
{
  gpointer *ptr = atomic;
  gboolean success;

  pthread_mutex_lock (&g_atomic_lock);

  if ((success = (*ptr == oldval)))
    *ptr = newval;

  pthread_mutex_unlock (&g_atomic_lock);

  return success;
}

gboolean
(g_atomic_pointer_compare_and_exchange_full) (void     *atomic,
                                              gpointer  oldval,
                                              gpointer  newval,
                                              void     *preval)
{
  gpointer *ptr = atomic;
  gpointer *pre = preval;
  gboolean success;

  pthread_mutex_lock (&g_atomic_lock);

  *pre = *ptr;
  if ((success = (*ptr == oldval)))
    *ptr = newval;

  pthread_mutex_unlock (&g_atomic_lock);

  return success;
}

gpointer
(g_atomic_pointer_exchange) (void    *atomic,
                             gpointer newval)
{
  gpointer *ptr = atomic;
  gpointer oldval;

  pthread_mutex_lock (&g_atomic_lock);
  oldval = *ptr;
  *ptr = newval;
  pthread_mutex_unlock (&g_atomic_lock);

  return oldval;
}

gssize
(g_atomic_pointer_add) (volatile void *atomic,
                        gssize         val)
{
  gssize *ptr = atomic;
  gssize oldval;

  pthread_mutex_lock (&g_atomic_lock);
  oldval = *ptr;
  *ptr = oldval + val;
  pthread_mutex_unlock (&g_atomic_lock);

  return oldval;
}

gsize
(g_atomic_pointer_and) (volatile void *atomic,
                        gsize          val)
{
  gsize *ptr = atomic;
  gsize oldval;

  pthread_mutex_lock (&g_atomic_lock);
  oldval = *ptr;
  *ptr = oldval & val;
  pthread_mutex_unlock (&g_atomic_lock);

  return oldval;
}

gsize
(g_atomic_pointer_or) (volatile void *atomic,
                       gsize          val)
{
  gsize *ptr = atomic;
  gsize oldval;

  pthread_mutex_lock (&g_atomic_lock);
  oldval = *ptr;
  *ptr = oldval | val;
  pthread_mutex_unlock (&g_atomic_lock);

  return oldval;
}

gsize
(g_atomic_pointer_xor) (volatile void *atomic,
                        gsize          val)
{
  gsize *ptr = atomic;
  gsize oldval;

  pthread_mutex_lock (&g_atomic_lock);
  oldval = *ptr;
  *ptr = oldval ^ val;
  pthread_mutex_unlock (&g_atomic_lock);

  return oldval;
}

#endif

/**
 * g_atomic_int_exchange_and_add:
 * @atomic: a pointer to a #gint
 * @val: the value to add
 *
 * This function existed before g_atomic_int_add() returned the prior
 * value of the integer (which it now does).  It is retained only for
 * compatibility reasons.  Don't use this function in new code.
 *
 * Returns: the value of @atomic before the add, signed
 * Since: 2.4
 * Deprecated: 2.30: Use g_atomic_int_add() instead.
 **/
gint
g_atomic_int_exchange_and_add (volatile gint *atomic,
                               gint           val)
{
  return (g_atomic_int_add) ((gint *) atomic, val);
}

