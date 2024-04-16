
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

#include "qlog.h"

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
void c_qlog_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
        default:
        case QtDebugMsg: {
            C_LOG_RAW(C_LOG_LEVEL_DEBUG, C_LOG_TAG, file, context.line, function, msg.toUtf8().constData());
            break;
        }
        case QtInfoMsg: {
            C_LOG_RAW(C_LOG_LEVEL_INFO, C_LOG_TAG, file, context.line, function, msg.toUtf8().constData());
            break;
        }
        case QtWarningMsg: {
            C_LOG_RAW(C_LOG_LEVEL_WARNING, C_LOG_TAG, file, context.line, function, msg.toUtf8().constData());
            break;
        }
        case QtCriticalMsg: {
            C_LOG_RAW(C_LOG_LEVEL_CRIT, C_LOG_TAG, file, context.line, function, msg.toUtf8().constData());
            break;
        }
        case QtFatalMsg: {
            C_LOG_RAW(C_LOG_LEVEL_ERROR, C_LOG_TAG, file, context.line, function, msg.toUtf8().constData());
            break;
        }
    }
}
#else
void c_qlog_handler(QtMsgType type, const QString &msg)
{
    switch (type) {
        default:
        case QtDebugMsg: {
            C_LOG_RAW(C_LOG_LEVEL_DEBUG, C_LOG_TAG, "", 0, "", msg.toUtf8().constData());
            break;
        }
        case QtInfoMsg: {
            C_LOG_RAW(C_LOG_LEVEL_INFO, C_LOG_TAG, "", 0, "", msg.toUtf8().constData());
            break;
        }
        case QtWarningMsg: {
            C_LOG_RAW(C_LOG_LEVEL_WARNING, C_LOG_TAG, "", 0, "", msg.toUtf8().constData());
            break;
        }
        case QtCriticalMsg: {
            C_LOG_RAW(C_LOG_LEVEL_CRIT, C_LOG_TAG, "", 0, "", msg.toUtf8().constData());
            break;
        }
        case QtFatalMsg: {
            C_LOG_RAW(C_LOG_LEVEL_ERROR, C_LOG_TAG, "", 0, "", msg.toUtf8().constData());
            break;
        }
    }
}
#endif

