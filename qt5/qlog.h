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
        c_log_init (C_LOG_TYPE_FILE, C_LOG_LEVEL, C_LOG_SIZE, C_LOG_DIR, C_LOG_TAG, "log", false); \
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
