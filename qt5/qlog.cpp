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

