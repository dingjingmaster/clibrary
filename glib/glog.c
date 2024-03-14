//
// Created by dingjing on 24-3-8.
//
#include "glog.h"

#include <unistd.h>


#ifdef GLIB_VERSION_2_50

GLogWriterOutput c_glog_handler(GLogLevelFlags level, const GLogField *fields, gsize nFields, C_UNUSED gpointer udata)
{
    g_autofree char *msg = NULL;
    g_autofree char *line = NULL;
    const char *file = NULL;
    const char *func = NULL;

    CLogLevel logLevel;
    switch (level) {
        case G_LOG_LEVEL_MESSAGE:
        case G_LOG_LEVEL_INFO: {
            logLevel = C_LOG_LEVEL_INFO;
            break;
        }
        case G_LOG_LEVEL_WARNING: {
            logLevel = C_LOG_LEVEL_WARNING;
            break;
        }
        case G_LOG_LEVEL_CRITICAL: {
            logLevel = C_LOG_LEVEL_CRIT;
            break;
        }
        case G_LOG_LEVEL_ERROR: {
            logLevel = C_LOG_LEVEL_ERROR;
            break;
        }
        case G_LOG_LEVEL_DEBUG:
        default: {
            logLevel = C_LOG_LEVEL_DEBUG;
        }
    }
    for (int i = 0; i < nFields; ++i) {
        if (0 == g_ascii_strcasecmp ("file", fields[i].key)) {
            file = fields[i].value;
        } else if (0 == g_ascii_strcasecmp ("func", fields[i].key)) {
            func = fields[i].value;
        } else if (0 == g_ascii_strcasecmp ("line", fields[i].key)) {
            line = g_strdup_printf ("%u", (*(const int*) (&(fields[i].value))));
        } else if (0 == g_ascii_strcasecmp ("message", fields[i].key)) {
            msg = g_strdup_printf ("%s", (char*) (fields[i].value ? fields[i].value : "<null>"));
        }
#if 0
        else {
            write (2, fields[i].key, strlen (fields[i].key));
            write (2, "\n", 1);
        }
#endif
    }

    C_LOG_RAW(logLevel, C_LOG_TAG, (file ? file : ""), line ? (strtol(line, NULL, 10)) : 0, (func ? func : ""), msg);

    return G_LOG_WRITER_HANDLED;
}

#else

void c_glog_handler(const char *logDomain, GLogLevelFlags level, const char *msg, C_UNUSED gpointer udata)
{
    CLogLevel logLevel;
    switch (level) {
        case G_LOG_LEVEL_MESSAGE:
        case G_LOG_LEVEL_INFO: {
            logLevel = C_LOG_LEVEL_INFO;
            break;
        }
        case G_LOG_LEVEL_WARNING: {
            logLevel = C_LOG_LEVEL_WARNING;
            break;
        }
        case G_LOG_LEVEL_CRITICAL: {
            logLevel = C_LOG_LEVEL_CRIT;
            break;
        }
        case G_LOG_LEVEL_ERROR: {
            logLevel = C_LOG_LEVEL_ERROR;
            break;
        }
        case G_LOG_LEVEL_DEBUG:
        default: {
            logLevel = C_LOG_LEVEL_DEBUG;
        }
    }

    C_LOG_RAW(logLevel, C_LOG_TAG, (""), 0, (""), msg);
}
#endif

