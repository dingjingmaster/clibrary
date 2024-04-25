
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-4-25.
//

#include "source.h"

#include <pwd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <linux/wait.h>
#include <sys/syscall.h>


#include "log.h"
#include "str.h"
#include "hash.h"
#include "hook.h"
#include "error.h"
#include "thread.h"
#include "atomic.h"
#include "wakeup.h"

#ifndef W_STOPCODE
#define W_STOPCODE(sig)      ((sig) << 8 | 0x7f)
#endif

#ifndef W_EXITCODE
#define W_EXITCODE(ret, sig) ((ret) << 8 | (sig))
#endif

C_DEFINE_QUARK (c-unix-error-quark, c_unix_error)

typedef struct _CIdleSource             CIdleSource;
typedef struct _CTimeoutSource          CTimeoutSource;
typedef struct _CChildWatchSource       CChildWatchSource;
typedef struct _CUnixSignalWatchSource  CUnixSignalWatchSource;
typedef struct _CPollRec                CPollRec;
typedef struct _CSourceCallback         CSourceCallback;
typedef struct _CUnixSignalWatchSource  CUnixSignalWatchSource;
typedef struct _CSourceList             CSourceList;
typedef struct _CMainWaiter             CMainWaiter;
typedef struct _CMainDispatch           CMainDispatch;


typedef enum
{
    C_SOURCE_READY          = 1 << (C_HOOK_FLAG_USER_SHIFT),
    C_SOURCE_CAN_RECURSE    = 1 << (C_HOOK_FLAG_USER_SHIFT + 1),
    C_SOURCE_BLOCKED        = 1 << (C_HOOK_FLAG_USER_SHIFT + 2)
} CSourceFlags;

struct _CMainWaiter
{
    CCond*  cond;
    CMutex* mutex;
};

struct _CMainDispatch
{
    cint        depth;
    CSource*    source;
};

struct _CSourceList
{
    CSource *head, *tail;
    cint priority;
};

typedef struct
{
    CSource     source;
    cint        fd;
    void*       tag;
} CUnixFDSource;

struct _CUnixSignalWatchSource
{
    CSource     source;
    int         signum;
    bool        pending; /* (atomic) */
};

struct _CMainContext
{
    CMutex              mutex;
    CCond               cond;
    CThread*            owner;
    cuint               ownerCount;
    CMainContextFlags   flags;
    CSList*             waiters;
    cint                refCount;               // (atomic)
    CHashTable*         sources;                // guint -> CSource
    CPtrArray*          pendingDispatches;
    cint                timeout;                // Timeout for current iteration
    cuint               nextId;
    CList*              sourceLists;
    cint                inCheckOrPrepare;
    CPollRec*           pollRecords;
    cuint               nPollRecords;
    CPollFD*            cachedPollArray;
    cuint               cachedPollArraySize;
    CWakeup*            wakeup;
    CPollFD             wakeupRec;
    bool                pollChanged;
    CPollFunc           pollFunc;
    cint64              time;
    bool                timeIsFresh;
};


C_LOCK_DEFINE_STATIC (gsUnixSignalLock);
static cuint gsUnixSignalRefcount[NSIG];
static CSList* gsUnixSignalWatches;
static CSList* gsUnixChildWatches;


static const char*  signum_to_string (int signum);
CSource*            _c_main_create_unix_signal_watch (int signum);
static bool         c_unix_signal_watch_check (CSource* source);
static void         ref_unix_signal_handler_unlocked (int signum);
static void         unref_unix_signal_handler_unlocked (int signum);
static bool         c_unix_set_error_from_errno (CError** error, int savedErrno);
static bool         c_unix_signal_watch_prepare (CSource* source, cint* timeout);
bool                c_unix_fd_source_dispatch (CSource* source, CSourceFunc callback, void* udata);
static bool         c_unix_signal_watch_dispatch (CSource* source, CSourceFunc callback, void* udata);


CSourceFuncs c_unix_fd_source_funcs = {
    NULL, NULL, c_unix_fd_source_dispatch, NULL, NULL, NULL
};







bool c_unix_open_pipe (cint* fds, cint flags, CError** error)
{
    int eCode;

    c_return_val_if_fail ((flags & (FD_CLOEXEC)) == flags, false);

    eCode = pipe (fds);
    if (eCode == -1) {
        return c_unix_set_error_from_errno (error, errno);
    }

    if (flags == 0) {
        return true;
    }

    eCode = fcntl (fds[0], F_SETFD, flags);
    if (eCode == -1) {
        int savedErrno = errno;
        close (fds[0]);
        close (fds[1]);
        return c_unix_set_error_from_errno (error, savedErrno);
    }
    eCode = fcntl (fds[1], F_SETFD, flags);
    if (eCode == -1) {
        int savedErrno = errno;
        close (fds[0]);
        close (fds[1]);
        return c_unix_set_error_from_errno (error, savedErrno);
    }

    return true;
}

bool c_unix_set_fd_nonblocking (cint fd, bool nonblock, CError** error)
{
#ifdef F_GETFL
    clong fcntlFlags = fcntl (fd, F_GETFL);
    if (fcntlFlags == -1) {
        return c_unix_set_error_from_errno (error, errno);
    }

    if (nonblock) {
#ifdef O_NONBLOCK
        fcntlFlags |= O_NONBLOCK;
#else
        fcntlFlags |= O_NDELAY;
#endif
    }
    else {
#ifdef O_NONBLOCK
        fcntlFlags &= ~O_NONBLOCK;
#else
        fcntlFlags &= ~O_NDELAY;
#endif
    }

    if (fcntl (fd, F_SETFL, fcntlFlags) == -1) {
        return c_unix_set_error_from_errno (error, errno);
    }
    return true;
#else
    return c_unix_set_error_from_errno (error, EINVAL);
#endif
}

CSource* c_unix_signal_source_new (cint signum)
{
    c_return_val_if_fail (signum == SIGHUP || signum == SIGINT || signum == SIGTERM || signum == SIGUSR1 || signum == SIGUSR2 || signum == SIGWINCH, NULL);

    return _c_main_create_unix_signal_watch (signum);
}

cuint c_unix_signal_add_full (cint priority, cint signum, CSourceFunc handler, void* udata, CDestroyNotify notify)
{
    CSource *source = c_unix_signal_source_new (signum);

    if (priority != C_PRIORITY_DEFAULT) {
        c_source_set_priority (source, priority);
    }

    c_source_set_callback (source, handler, udata, notify);
    cuint id = c_source_attach (source, NULL);
    c_source_unref (source);

    return id;
}

cuint c_unix_signal_add (cint signum, CSourceFunc handler, void* udata)
{
    return c_unix_signal_add_full (C_PRIORITY_DEFAULT, signum, handler, udata, NULL);
}

CSource* c_unix_fd_source_new (cint fd, CIOCondition condition)
{
    CUnixFDSource* fdSource;
    CSource* source = c_source_new (&c_unix_fd_source_funcs, sizeof (CUnixFDSource));

    fdSource = (CUnixFDSource*) source;

    fdSource->fd = fd;
    fdSource->tag = c_source_add_unix_fd (source, fd, condition);

    return source;
}

cuint c_unix_fd_add_full (cint priority, cint fd, CIOCondition condition, CUnixFDSourceFunc function, void* udata, CDestroyNotify notify)
{
    CSource *source;
    cuint id;

    c_return_val_if_fail (function != NULL, 0);

    source = c_unix_fd_source_new (fd, condition);

    if (priority != C_PRIORITY_DEFAULT) {
        c_source_set_priority (source, priority);
    }

    c_source_set_callback (source, (CSourceFunc) function, udata, notify);
    id = c_source_attach (source, NULL);
    c_source_unref (source);

    return id;
}

cuint c_unix_fd_add (cint fd, CIOCondition condition, CUnixFDSourceFunc function, void* udata)
{
    return c_unix_fd_add_full (C_PRIORITY_DEFAULT, fd, condition, function, udata, NULL);
}

struct passwd* c_unix_get_passwd_entry (const char* userName, CError** error)
{
    struct passwd* passwdFileEntry;
    struct
    {
        struct passwd pwd;
        char stringBuffer[];
    } *buffer = NULL;
    csize stringBufferSize = 0;
    CError* localError = NULL;

    c_return_val_if_fail (userName != NULL, NULL);
    c_return_val_if_fail (error == NULL || *error == NULL, NULL);

#ifdef _SC_GETPW_R_SIZE_MAX
    {
        clong stringBufferSizeLong = sysconf (_SC_GETPW_R_SIZE_MAX);
        if (stringBufferSizeLong > 0) {
            stringBufferSize = stringBufferSizeLong;
        }
    }
#endif /* _SC_GETPW_R_SIZE_MAX */

    /* Default starting size. */
    if (stringBufferSize == 0) {
        stringBufferSize = 64;
    }

    do {
        int retVal;
        c_free (buffer);
        buffer = c_malloc0 (sizeof (*buffer) + stringBufferSize + 6);
        retVal = getpwnam_r (userName, &buffer->pwd, buffer->stringBuffer, stringBufferSize, &passwdFileEntry);
        if (passwdFileEntry != NULL) {
            break;
        }
        else if (retVal == 0 || retVal == ENOENT || retVal == ESRCH || retVal == EBADF || retVal == EPERM) {
            c_unix_set_error_from_errno (&localError, retVal);
            break;
        }
        else if (retVal == ERANGE) {
            if (stringBufferSize > 32 * 1024) {
                c_unix_set_error_from_errno (&localError, retVal);
                break;
            }
            stringBufferSize *= 2;
            continue;
        }
        else {
            c_unix_set_error_from_errno (&localError, retVal);
            break;
        }
    }
    while (passwdFileEntry == NULL);

    c_assert (passwdFileEntry == NULL || (void*) passwdFileEntry == (void*) buffer);

    /* Success or error. */
    if (localError != NULL) {
        c_clear_pointer ((void**) &buffer, c_free0);
        c_propagate_error (error, c_steal_pointer (&localError));
    }

    return (struct passwd *) c_steal_pointer (&buffer);
}

CSource* _c_main_create_unix_signal_watch (int signum)
{
    CSource *source = c_source_new (&c_unix_signal_funcs, sizeof (CUnixSignalWatchSource));

    CUnixSignalWatchSource* unixSignalSource = (CUnixSignalWatchSource*) source;

    unixSignalSource->signum = signum;
    unixSignalSource->pending = false;

    /* Set a default name on the source, just in case the caller does not. */
    c_source_set_static_name (source, signum_to_string (signum));

    C_LOCK (gsUnixSignalLock);
    ref_unix_signal_handler_unlocked (signum);
    gsUnixSignalWatches = c_slist_prepend (gsUnixSignalWatches, unix_signal_source);
    dispatch_unix_signals_unlocked ();
    C_UNLOCK (gsUnixSignalLock);

    return source;
}



//
bool c_unix_fd_source_dispatch (CSource* source, CSourceFunc callback, void* udata)
{
    CUnixFDSource* fdSource = (CUnixFDSource*) source;
    CUnixFDSourceFunc func = (CUnixFDSourceFunc) callback;

    if (!callback) {
        C_LOG_WARNING_CONSOLE("GUnixFDSource dispatched without callback. You must call g_source_set_callback().")
        return false;
    }

    return (* func) (fdSource->fd, c_source_query_unix_fd (source, fdSource->tag), udata);
}

static bool c_unix_set_error_from_errno (CError** error, int savedErrno)
{
    c_set_error_literal (error, C_UNIX_ERROR, 0, c_strerror (savedErrno));
    errno = savedErrno;

    return false;
}

static void unref_unix_signal_handler_unlocked (int signum)
{
    gsUnixSignalRefcount[signum]--;
    if (gsUnixSignalRefcount[signum] == 0) {
        struct sigaction action;
        memset (&action, 0, sizeof (action));
        action.sa_handler = SIG_DFL;
        sigemptyset (&action.sa_mask);
        sigaction (signum, &action, NULL);
    }
}

static void ref_unix_signal_handler_unlocked (int signum)
{
    /* Ensure we have the worker context */
    c_get_worker_context ();
    gsUnixSignalRefcount[signum]++;
    if (gsUnixSignalRefcount[signum] == 1) {
        struct sigaction action;
        action.sa_handler = c_unix_signal_handler;
        sigemptyset (&action.sa_mask);
#ifdef SA_RESTART
        action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
#else
        action.sa_flags = SA_NOCLDSTOP;
#endif
        sigaction (signum, &action, NULL);
    }
}

static const char* signum_to_string (int signum)
{
    /* See `man 0P signal.h` */
#define SIGNAL(s) \
    case (s): \
        return ("CUnixSignalSource: " #s);
    switch (signum) {
        /* These signals are guaranteed to exist by POSIX. */
        SIGNAL (SIGABRT)
        SIGNAL (SIGFPE)
        SIGNAL (SIGILL)
        SIGNAL (SIGINT)
        SIGNAL (SIGSEGV)
        SIGNAL (SIGTERM)
        /* Frustratingly, these are not, and hence for brevity the list is
         * incomplete. */
#ifdef SIGALRM
        SIGNAL (SIGALRM)
#endif
#ifdef SIGCHLD
        SIGNAL (SIGCHLD)
#endif
#ifdef SIGHUP
        SIGNAL (SIGHUP)
#endif
#ifdef SIGKILL
        SIGNAL (SIGKILL)
#endif
#ifdef SIGPIPE
        SIGNAL (SIGPIPE)
#endif
#ifdef SIGQUIT
        SIGNAL (SIGQUIT)
#endif
#ifdef SIGSTOP
        SIGNAL (SIGSTOP)
#endif
#ifdef SIGUSR1
        SIGNAL (SIGUSR1)
#endif
#ifdef SIGUSR2
        SIGNAL (SIGUSR2)
#endif
#ifdef SIGPOLL
        SIGNAL (SIGPOLL)
#endif
#ifdef SIGPROF
        SIGNAL (SIGPROF)
#endif
#ifdef SIGTRAP
        SIGNAL (SIGTRAP)
#endif
        default:
            return "CUnixSignalSource: Unrecognized signal";
    }
#undef SIGNAL
}

static bool c_unix_signal_watch_dispatch (CSource* source, CSourceFunc callback, void* udata)
{
    bool again;

    CUnixSignalWatchSource* unixSignalSource = (CUnixSignalWatchSource*) source;

    if (!callback) {
        C_LOG_WARNING_CONSOLE("Unix signal source dispatched without callback. You must call g_source_set_callback().")
        return false;
    }

    c_atomic_int_set (&unixSignalSource->pending, false);

    again = (callback) (udata);

    return again;
}

static bool c_unix_signal_watch_check (CSource* source)
{
    CUnixSignalWatchSource* unixSignalSource = (CUnixSignalWatchSource*) source;

    return c_atomic_int_get (&unixSignalSource->pending);
}

static bool c_unix_signal_watch_prepare (CSource* source, cint* timeout)
{
    CUnixSignalWatchSource *unixSignalSource = (CUnixSignalWatchSource*) source;

    return c_atomic_int_get (&unixSignalSource->pending);
}

static bool c_child_watch_check (CSource* source)
{
    CChildWatchSource* childWatchSource = (CChildWatchSource*) source;

#ifdef HAVE_PIDFD
    if (child_watch_source->using_pidfd) {
      gboolean child_exited = child_watch_source->poll.revents & G_IO_IN;

      if (child_exited)
        {
          siginfo_t child_info = { 0, };

          /* Get the exit status */
          if (waitid (P_PIDFD, child_watch_source->poll.fd, &child_info, WEXITED | WNOHANG) >= 0 &&
              child_info.si_pid != 0)
            {
              /* waitid() helpfully provides the wait status in a decomposed
               * form which is quite useful. Unfortunately we have to report it
               * to the #GChildWatchFunc as a waitpid()-style platform-specific
               * wait status, so that the user code in #GChildWatchFunc can then
               * call WIFEXITED() (etc.) on it. That means re-composing the
               * status information. */
              child_watch_source->child_status = siginfo_t_to_wait_status (&child_info);
              child_watch_source->child_exited = TRUE;
            }
        }

      return child_exited;
    }
#endif  /* HAVE_PIDFD */

    return c_atomic_int_get (&childWatchSource->child_exited);
}
