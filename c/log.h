//
// Created by dingjing on 24-3-7.
//

#ifndef CLIBRARY_LOG_H
#define CLIBRARY_LOG_H

#include "macros.h"

C_BEGIN_EXTERN_C

#ifndef C_LOG_TAG
#define C_LOG_TAG "clog"
#endif

#define C_LOG_ERROR(...)    c_log_print(C_LOG_LEVEL_ERR,      C_LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define C_LOG_CRIT(...)     c_log_print(C_LOG_LEVEL_CRIT,     C_LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define C_LOG_WARNING(...)  c_log_print(C_LOG_LEVEL_WARNING,  C_LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define C_LOG_INFO(...)     c_log_print(C_LOG_LEVEL_INFO,     C_LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define C_LOG_DEBUG(...)    c_log_print(C_LOG_LEVEL_DEBUG,    C_LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define C_LOG_VERB(...)     c_log_print(C_LOG_LEVEL_VERB,     C_LOG_TAG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define C_LOG_RAW(level, tag, file, line, fun, ...)      c_log_print(level, tag, file, line, fun, __VA_ARGS__)

/**
 * @brief 日志输出模式
 */
typedef enum
{
    C_LOG_TYPE_FILE = 0,
    C_LOG_TYPE_CONSOLE
} CLogType;

/**
 * @brief 日志级别
 */
typedef enum
{
    C_LOG_LEVEL_ERROR       = 0,
    C_LOG_LEVEL_CRIT        = 1,
    C_LOG_LEVEL_WARNING     = 2,
    C_LOG_LEVEL_INFO        = 3,
    C_LOG_LEVEL_DEBUG       = 4,
    C_LOG_LEVEL_VERB        = 5,
} CLogLevel;

/**
 * @brief 初始化 log 参数
 *
 * @param level: 设置 log 输出级别
 * @param rotate: 是否切分文件
 * @param logSize: 每个日志文件的大小
 * @param dir: 日志文件存储文件夹路径
 * @param prefix: 日志文件名
 * @param suffix: 日志文件后缀名
 * @param hasTime: 文件名中是否带时间
 *
 * @return 成功: 0; 失败: -1
 */
bool c_log_init (CLogType type, CLogLevel level, cuint64 logSize, const cchar* dir, const cchar* prefix, const cchar* suffix, bool hasTime);

/**
 * 销毁 log 参数
 */
void c_log_destroy (void);

/**
 * @brief 输出日志到文件
 */
void c_log_print (CLogLevel level, const cchar* tag, const cchar* file, int line, const cchar* func, const cchar* fmt, ...);

/**
 * @brief 是否完成初始化
 */
bool c_log_is_inited ();

C_END_EXTERN_C

#endif //CLIBRARY_LOG_H
