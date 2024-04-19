
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-4-19.
//

#include "thread.h"

#include <unistd.h>

#include "log.h"
#include "error.h"
#include "slist.h"
#include "atomic.h"


C_DEFINE_QUARK (c_thread_error, c_thread_error)

struct _CRealThread
{
    CThread         thread;
    int             refCount;
    bool            ours;
    char*           name;
    void*           retVal;
};

typedef struct
{
    struct sched_attr*  attr;
    void*               dummy;
} CThreadSchedulerSettings;



static CMutex       gsOnceMutex;
static CCond        gsOnceCond;
static CSList*      gsOnceInitList = NULL;
static cuint        gsThreadNCreatedCounter = 0;    // atomic


void            c_system_thread_exit                    (void);
cuint           c_thread_n_created                      (void);
void*           c_thread_proxy                          (void* thread);
void            c_system_thread_set_name                (const char* name);
void            c_system_thread_wait                    (CRealThread* thread);
void            c_system_thread_free                    (CRealThread* thread);
void*           c_private_set_alloc0                    (CPrivate* key, csize size);
bool            c_system_thread_get_scheduler_settings  (CThreadSchedulerSettings* schedulerSettings);
bool            c_thread_get_scheduler_settings         (CThreadSchedulerSettings* schedulerSettings);
CRealThread*    c_system_thread_new                     (CThreadFunc proxy, culong stack_size, const CThreadSchedulerSettings *scheduleSsettings, const char* name, CThreadFunc func, void* data, CError** error);
CThread*        c_thread_new_internal                   (const char* name, CThreadFunc proxy, CThreadFunc func, void* data, csize stackSize, const CThreadSchedulerSettings* schedulerSettings, CError** error);


static void c_thread_cleanup (void* data);
static void c_thread_abort (cint status, const char* function);


static CPrivate gsThreadSpecificPrivate = C_PRIVATE_INIT (c_thread_cleanup);


CThread* c_thread_ref (CThread* thread)
{
    CRealThread* real = (CRealThread*) thread;

    c_atomic_int_inc (&real->refCount);

    return thread;
}

void c_thread_unref (CThread* thread)
{
    CRealThread* real = (CRealThread*) thread;

    if (c_atomic_int_dec_and_test (&real->refCount)) {
        if (real->ours) {
            c_system_thread_free (real);
        }
        else {
            c_free (real);
        }
    }
}

CThread* c_thread_new (const char* name, CThreadFunc func, void* data)
{
    CError* error = NULL;

    CThread* thread = c_thread_new_internal (name, c_thread_proxy, func, data, 0, NULL, &error);

    if (C_UNLIKELY (thread == NULL)) {
        C_LOG_ERROR_CONSOLE("creating thread '%s': %s", name ? name : "", error->message);
    }

    return thread;
}

CThread* c_thread_try_new (const char* name, CThreadFunc func, void* data, CError** error)
{
    return c_thread_new_internal (name, c_thread_proxy, func, data, 0, NULL, error);
}

CThread* c_thread_self (void)
{
    CRealThread* thread = c_private_get (&gsThreadSpecificPrivate);

    if (!thread) {
        thread = c_malloc0(sizeof (CRealThread));
        thread->refCount = 1;

        c_private_set (&gsThreadSpecificPrivate, thread);
    }

    return (CThread*) thread;
}

void c_thread_exit (void* retVal)
{
    CRealThread* real = (CRealThread*) c_thread_self ();

    if (C_UNLIKELY (!real->ours)) {
        C_LOG_ERROR_CONSOLE("attempt to c_thread_exit() a thread not created by CLib");
    }

    real->retVal = retVal;

    c_system_thread_exit ();
}

void* c_thread_join (CThread* thread)
{
    CRealThread *real = (CRealThread*) thread;

    c_return_val_if_fail (thread, NULL);
    c_return_val_if_fail (real->ours, NULL);

    c_system_thread_wait (real);

    void* retVal = real->retVal;

    thread->joinable = 0;

    c_thread_unref (thread);

    return retVal;
}

void c_thread_yield (void)
{}

void c_mutex_init (CMutex* mutex)
{}

void c_mutex_clear (CMutex* mutex)
{}

void c_mutex_lock (CMutex* mutex)
{}

bool c_mutex_trylock (CMutex* mutex)
{}

void c_mutex_unlock (CMutex* mutex)
{}

void c_rw_lock_init (CRWLock* rwLock)
{}

void c_rw_lock_clear (CRWLock* rwLock)
{}

void c_rw_lock_writer_lock (CRWLock* rwLock)
{}

bool c_rw_lock_writer_trylock (CRWLock* rwLock)
{}

void c_rw_lock_writer_unlock (CRWLock* rwLock)
{}

void c_rw_lock_reader_lock (CRWLock* rwLock)
{}

bool c_rw_lock_reader_trylock (CRWLock* rwLock)
{}

void c_rw_lock_reader_unlock (CRWLock* rwLock)
{}

void c_rec_mutex_init (CRecMutex* recMutex)
{}

void c_rec_mutex_clear (CRecMutex* recMutex)
{}

void c_rec_mutex_lock (CRecMutex* recMutex)
{}

bool c_rec_mutex_trylock (CRecMutex* recMutex)
{}

void c_rec_mutex_unlock (CRecMutex* recMutex)
{}

void c_cond_init (CCond* cond)
{}

void c_cond_clear (CCond* cond)
{}

void c_cond_wait (CCond* cond, CMutex* mutex)
{}

void c_cond_signal (CCond* cond)
{}

void c_cond_broadcast (CCond* cond)
{}

bool c_cond_wait_until (CCond* cond, CMutex* mutex, cint64 endTime)
{}

void* c_private_get (CPrivate* key)
{}

void c_private_set (CPrivate* key, void* value)
{}

void c_private_replace (CPrivate* key, void* value)
{}

bool c_once_init_enter (volatile void* location)
{
    csize* valueLocation = (csize*) location;
    bool needInit = false;
    c_mutex_lock (&gsOnceMutex);
    if (c_atomic_pointer_get (valueLocation) == 0) {
        if (!c_slist_find (gsOnceInitList, (void*) valueLocation)) {
            needInit = true;
            gsOnceInitList = c_slist_prepend (gsOnceInitList, (void*) valueLocation);
        }
        else {
            do {
                c_cond_wait (&gsOnceCond, &gsOnceMutex);
            }
            while (c_slist_find (gsOnceInitList, (void*) valueLocation));
        }
    }
    c_mutex_unlock (&gsOnceMutex);

    return needInit;
}

void c_once_init_leave (volatile void* location, csize result)
{
    csize* valueLocation = (csize*) location;

    c_return_if_fail (result != 0);

    csize oldValue = (csize) c_atomic_pointer_exchange (valueLocation, (void*) &result);
    c_return_if_fail (oldValue == 0);

    c_mutex_lock (&gsOnceMutex);
    c_return_if_fail (gsOnceInitList != NULL);
    gsOnceInitList = c_slist_remove (gsOnceInitList, (void*) valueLocation);
    c_cond_broadcast (&gsOnceCond);
    c_mutex_unlock (&gsOnceMutex);
}

cuint c_get_num_processors (void)
{
    return 1;
}



void* c_once_impl (COnce* once, CThreadFunc func, void* arg)
{
    c_mutex_lock (&gsOnceMutex);

    while (once->status == C_ONCE_STATUS_PROGRESS) {
        c_cond_wait (&gsOnceCond, &gsOnceMutex);
    }

    if (once->status != C_ONCE_STATUS_READY) {
        once->status = C_ONCE_STATUS_PROGRESS;

        c_mutex_unlock (&gsOnceMutex);
        void* retVal = func (arg);
        c_mutex_lock (&gsOnceMutex);

        once->retVal = retVal;
        once->status = C_ONCE_STATUS_READY;
        c_cond_broadcast (&gsOnceCond);
    }

    c_mutex_unlock (&gsOnceMutex);

    return (void*) once->retVal;
}

void* c_private_set_alloc0 (CPrivate* key, csize size)
{
    void* allocated = c_malloc0 (size);

    c_private_set (key, allocated);

    return c_steal_pointer (&allocated);
}

static void c_thread_cleanup (void* data)
{
    c_thread_unref (data);
}

void* c_thread_proxy (void* thread)
{
    CRealThread* threadT = thread;

    c_assert (thread);
    c_private_set (&gsThreadSpecificPrivate, thread);

    if (threadT->name) {
        c_system_thread_set_name (threadT->name);
        c_free (threadT->name);
        threadT->name = NULL;
    }

    threadT->retVal = threadT->thread.func (threadT->thread.data);

    return NULL;
}

cuint g_thread_n_created (void)
{
    return c_atomic_int_get ((int*) &gsThreadNCreatedCounter);
}

CThread* c_thread_new_internal (const char* name, CThreadFunc proxy, CThreadFunc func, void* data, csize stackSize, const CThreadSchedulerSettings* schedulerSettings, CError** error)
{
    c_return_val_if_fail (func != NULL, NULL);

    c_atomic_int_inc ((int*) &gsThreadNCreatedCounter);

    return (CThread*) c_system_thread_new (proxy, stackSize, schedulerSettings, name, func, data, error);
}

bool c_thread_get_scheduler_settings (CThreadSchedulerSettings* schedulerSettings)
{
    c_return_val_if_fail (schedulerSettings != NULL, false);

    return c_system_thread_get_scheduler_settings (schedulerSettings);
}

static void c_thread_abort (cint status, const char* function)
{
    fprintf (stderr, "CLib: Unexpected error from C library during '%s': %s.  Aborting.\n", function, strerror (status));
    c_abort ();
}