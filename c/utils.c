
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-6-7.
//

#include "utils.h"

#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>

#include "clib.h"

#define CHAR_IS_SAFE(wc)    (!((wc < 0x20 && wc != '\t' && wc != '\n' && wc != '\r') || (wc == 0x7f) || (wc >= 0x80 && wc < 0xa0)))

static void print_string (FILE* stream, const cchar* str);
static cchar* strdup_convert (const cchar *string, const cchar *charset);

static bool c_unix_set_error_from_errno (CError** error, int savedErrno)
{
    c_set_error_literal (error, C_UNIX_ERROR, 0, c_strerror (savedErrno));
    errno = savedErrno;

    return false;
}

static char* gsTmpDir = NULL;
C_LOCK_DEFINE_STATIC (gsUtilsLocker);

C_LOCK_DEFINE_STATIC (gsPrgname);
static const cchar* gsPrgname = NULL;

const char *c_get_tmp_dir(void)
{
    C_LOCK (gsUtilsLocker);

    if (gsTmpDir == NULL) {
        char *tmp = c_strdup (c_getenv ("G_TEST_TMPDIR"));

        if (tmp == NULL || *tmp == '\0') {
            c_free (tmp);
            tmp = c_strdup (c_getenv ("TMPDIR"));
        }

#ifdef P_tmpdir
        if (tmp == NULL || *tmp == '\0') {
            csize k;
            c_free (tmp);
            tmp = c_strdup (P_tmpdir);
            k = strlen (tmp);
            if (k > 1 && C_IS_DIR_SEPARATOR (tmp[k - 1]))
                tmp[k - 1] = '\0';
        }
#endif /* P_tmpdir */

        if (tmp == NULL || *tmp == '\0') {
            c_free (tmp);
            tmp = c_strdup ("/tmp");
        }

        gsTmpDir = c_steal_pointer (&tmp);
    }

    C_UNLOCK (gsUtilsLocker);

    return gsTmpDir;
}

const cchar* c_get_prgname (void)
{
    const cchar* retval;

    C_LOCK (gsPrgname);
    retval = gsPrgname;
    C_UNLOCK (gsPrgname);

    return retval;
}

void c_set_prgname (const cchar *prgname)
{
    CQuark qprgname = c_quark_from_string (prgname);
    C_LOCK (gsPrgname);
    gsPrgname = c_quark_to_string (qprgname);
    C_UNLOCK (gsPrgname);
}

void c_print (const cchar *format, ...)
{
    va_list args;
    cchar* str;

    c_return_if_fail (format != NULL);

    va_start (args, format);
    str = c_strdup_vprintf (format, args);
    va_end (args);

    print_string (stdout, str);

    c_free (str);
}

int c_drop_permissions(void)
{
    errno = 0;
    bool ok = true;

    do {
        if (setgid(getgid()) < 0) {
            ok = false;
            break;
        }

        if (setuid(getuid()) < 0) {
            ok = false;
            break;
        }
    } while (0);

    return ok ? 0 : (errno ? -errno : -1);
}

bool c_program_check_is_first(const char *appName)
{
    static bool ret = false;
    static cuint inited = 0;

    if (c_once_init_enter(&inited)) {
        do {
            static int fw = 0;
            const cuint m = umask(0);

            cchar* base64 = c_base64_encode((const cuchar*) appName, strlen(appName));
            if (!base64) {
                c_free(base64);
                break;
            }

            cchar* path = c_strdup_printf("%s/%s.lock", c_get_tmp_dir(), base64);
            c_free(base64);
            if (path) {
                fw = open(path, O_RDWR | O_CREAT, 0777);
                umask(m);
                if (-1 == fw) {
                    c_free(path);
                    break;
                }

                if (0 == flock(fw, LOCK_EX | LOCK_NB)) {
                    ret = true;
                    c_free(path);
                    break;
                }
                c_free(path);
            }

        } while (false);
        c_once_init_leave(&inited, 1);
    }

    return ret;
}

static void print_string (FILE* stream, const cchar* str)
{
    const cchar *charset;
    int ret;

    if (c_get_console_charset (&charset)) {
        ret = fputs (str, stream);
    }
    else {
        cchar* convertedString = strdup_convert (str, charset);
        ret = fputs (convertedString, stream);
        c_free (convertedString);
    }

    if (ret == EOF) {
        return;
    }

    fflush (stream);
}

static cchar* strdup_convert (const cchar *str, const cchar *charset)
{
    if (!c_utf8_validate (str, -1, NULL)) {
        CString *gstring = c_string_new ("[Invalid UTF-8] ");
        cuchar *p;
        for (p = (cuchar*)str; *p; p++) {
            if (CHAR_IS_SAFE(*p) && !(*p == '\r' && *(p + 1) != '\n') && *p < 0x80) {
                c_string_append_c (gstring, *p);
            }
            else {
                c_string_append_printf (gstring, "\\x%02x", (cuint)(cuchar)*p);
            }
        }

        return c_string_free (gstring, false);
    }
    else {
        CError *err = NULL;
        cchar *result = c_convert_with_fallback (str, -1, charset, "UTF-8", "?", NULL, NULL, &err);
        if (result) {
            return result;
        }
        else {
            static bool warned = false;
            if (!warned) {
                warned = true;
                _c_fprintf (stderr, "GLib: Cannot convert message: %s\n", err->message);
            }
            c_error_free (err);

            return c_strdup (str);
        }
    }
}

cint64 c_get_monotonic_time (void)
{
    struct timespec ts;
    cint result;

    result = clock_gettime (CLOCK_MONOTONIC, &ts);

    if C_UNLIKELY (result != 0) {
        C_LOG_ERROR_CONSOLE("CLib requires working CLOCK_MONOTONIC");
    }

    return (((cint64) ts.tv_sec) * 1000000) + (ts.tv_nsec / 1000);
}

cint64 c_get_real_time (void)
{
    struct timeval r;

    gettimeofday (&r, NULL);

    return (((cint64) r.tv_sec) * 1000000) + r.tv_usec;
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