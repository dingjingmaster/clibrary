
/*
 * Copyright © 2024 <dingjing@live.cn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

//
// Created by dingjing on 24-3-8.
//

#ifndef CLIBRARY_QLOG_H
#define CLIBRARY_QLOG_H
#include <QDebug>

#include <c/clib.h>

#ifndef C_LOG_TAG
#define C_LOG_TAG       "qlog"
#endif

#ifndef C_LOG_SIZE
#define C_LOG_SIZE      (200 * 2 << 10)         // 200mb
#endif

#ifndef C_LOG_DIR
#define C_LOG_DIR       "/tmp/"
#endif

#ifdef DEBUG
#define C_LOG_LEVEL    C_LOG_LEVEL_VERB
#else
#define C_LOG_LEVEL    C_LOG_LEVEL_INFO
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#define C_QLOG_INIT_IF_NOT_INIT \
{ \
    if (C_UNLIKELY(!c_log_is_inited())) { \
        c_log_init (C_LOG_LEVEL, C_LOG_SIZE, C_LOG_DIR, C_LOG_TAG, "log", false); \
        qInstallMessageHandler(c_qlog_handler); \
    } \
};

void c_qlog_handler (QtMsgType type, const QMessageLogContext &context, const QString &msg);

#elif QT_VERSION >= QT_VERSION_CHECK(4,0,0)
#define C_QLOG_INIT_IF_NOT_INIT \
{ \
    if (C_UNLIKELY(!c_log_is_inited())) { \
        c_log_init (C_LOG_TYPE_FILE, C_LOG_LEVEL, C_LOG_SIZE, C_LOG_DIR, C_LOG_TAG, "log", false); \
        qInstallMsgHandler(c_qlog_handler); \
    } \
};

void c_qlog_handler (QtMsgType type, const QString &msg);

#endif

#endif //CLIBRARY_QLOG_H
