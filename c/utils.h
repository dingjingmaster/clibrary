
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*************************************************************************
> FileName: utils.h
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: Thu 08 Sep 2022 23:17:34 AM CST
 ************************************************************************/
#ifndef _UTILS_H
#define _UTILS_H

#if !defined (__CLIB_H_INSIDE__) && !defined (CLIB_COMPILATION)
#error "Only <clib.h> can be included directly."
#endif
#include <c/macros.h>

C_BEGIN_EXTERN_C

static inline int c_steal_fd (int* fdPtr)
{
    int fd = *fdPtr;
    *fdPtr = -1;
    return fd;
}

const char* c_get_tmp_dir       (void);
const char* c_get_prgname       (void);
//const char* c_get_home_dir (void);
//const char* c_get_user_name (void);
//const char* c_get_real_name (void);
//const char* c_get_host_name	(void);
void  c_set_prgname (const char* prgname);
//const char* c_get_application_name (void);
//char* c_get_os_info  (const char* keyName);
//void  c_set_application_name (const char* applicationName);

/**
 * @brief
 *  根据程序名称检查是否只启动此一份实例
 */
bool        c_program_check_is_first        (const char* appName);

void c_print (const cchar *format, ...);

/**
 * @brief 针对使用 sudo 启动的进程进行降权
 * @return 成功返回 0，否则返回错误码
 */
int c_drop_permissions (void);

cint64          c_get_monotonic_time                (void);
cint64          c_get_real_time                     (void);

bool            c_unix_set_fd_nonblocking           (cint fd, bool nonblock, CError** error);
bool            c_unix_open_pipe                    (cint* fds, cint flags, CError** error);


C_END_EXTERN_C

#endif
