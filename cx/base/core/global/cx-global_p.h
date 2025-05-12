//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_CX_GLOBAL_P_H
#define clibrary_CX_GLOBAL_P_H
#include "cx-global.h"

#ifndef QT_BOOTSTRAPPED
#include <CxCore/private/cx-config_p.h>
#include <CxCore/private/cx-core-config_p.h>
#endif

#if defined(__cplusplus)
#ifdef CX_CC_MINGW
#  include <unistd.h> // Define _POSIX_THREAD_SAFE_FUNCTIONS to obtain localtime_r()
#endif
#include <time.h>

QT_BEGIN_NAMESPACE

// These behave as if they consult the environment, so need to share its locking:
Q_CORE_EXPORT void qTzSet();
Q_CORE_EXPORT time_t qMkTime(struct tm *when);

QT_END_NAMESPACE

#if !__has_builtin(__builtin_available)
#include <initializer_list>
#include <QtCore/qoperatingsystemversion.h>
#include <QtCore/qversionnumber.h>

QT_BEGIN_NAMESPACE

struct qt_clang_builtin_available_os_version_data {
    QOperatingSystemVersion::OSType type;
    const char *version;
};

static inline bool qt_clang_builtin_available(
    const std::initializer_list<qt_clang_builtin_available_os_version_data> &versions)
{
    for (auto it = versions.begin(); it != versions.end(); ++it) {
        if (QOperatingSystemVersion::currentType() == it->type) {
            const auto current = QOperatingSystemVersion::current();
            return QVersionNumber(
                current.majorVersion(),
                current.minorVersion(),
                current.microVersion()) >= QVersionNumber::fromString(
                    QString::fromLatin1(it->version));
        }
    }

    // Result is true if the platform is not any of the checked ones; this matches behavior of
    // LLVM __builtin_available and @available constructs
    return true;
}

QT_END_NAMESPACE

#define QT_AVAILABLE_OS_VER(os, ver) \
    QT_PREPEND_NAMESPACE(qt_clang_builtin_available_os_version_data){\
        QT_PREPEND_NAMESPACE(QOperatingSystemVersion)::os, #ver}
#define QT_AVAILABLE_CAT(L, R) QT_AVAILABLE_CAT_(L, R)
#define QT_AVAILABLE_CAT_(L, R) L ## R
#define QT_AVAILABLE_EXPAND(...) QT_AVAILABLE_OS_VER(__VA_ARGS__)
#define QT_AVAILABLE_SPLIT(os_ver) QT_AVAILABLE_EXPAND(QT_AVAILABLE_CAT(QT_AVAILABLE_SPLIT_, os_ver))
#define QT_AVAILABLE_SPLIT_macOS MacOS,
#define QT_AVAILABLE_SPLIT_iOS IOS,
#define QT_AVAILABLE_SPLIT_tvOS TvOS,
#define QT_AVAILABLE_SPLIT_watchOS WatchOS,
#define QT_BUILTIN_AVAILABLE0(e) \
    QT_PREPEND_NAMESPACE(qt_clang_builtin_available)({})
#define QT_BUILTIN_AVAILABLE1(a, e) \
    QT_PREPEND_NAMESPACE(qt_clang_builtin_available)({QT_AVAILABLE_SPLIT(a)})
#define QT_BUILTIN_AVAILABLE2(a, b, e) \
    QT_PREPEND_NAMESPACE(qt_clang_builtin_available)({QT_AVAILABLE_SPLIT(a), \
                                                      QT_AVAILABLE_SPLIT(b)})
#define QT_BUILTIN_AVAILABLE3(a, b, c, e) \
    QT_PREPEND_NAMESPACE(qt_clang_builtin_available)({QT_AVAILABLE_SPLIT(a), \
                                                      QT_AVAILABLE_SPLIT(b), \
                                                      QT_AVAILABLE_SPLIT(c)})
#define QT_BUILTIN_AVAILABLE4(a, b, c, d, e) \
    QT_PREPEND_NAMESPACE(qt_clang_builtin_available)({QT_AVAILABLE_SPLIT(a), \
                                                      QT_AVAILABLE_SPLIT(b), \
                                                      QT_AVAILABLE_SPLIT(c), \
                                                      QT_AVAILABLE_SPLIT(d)})
#define QT_BUILTIN_AVAILABLE_ARG(arg0, arg1, arg2, arg3, arg4, arg5, ...) arg5
#define QT_BUILTIN_AVAILABLE_CHOOSER(...) QT_BUILTIN_AVAILABLE_ARG(__VA_ARGS__, \
    QT_BUILTIN_AVAILABLE4, \
    QT_BUILTIN_AVAILABLE3, \
    QT_BUILTIN_AVAILABLE2, \
    QT_BUILTIN_AVAILABLE1, \
    QT_BUILTIN_AVAILABLE0, )
#define __builtin_available(...) QT_BUILTIN_AVAILABLE_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#endif // !__has_builtin(__builtin_available)
#endif // defined(__cplusplus)


#endif // clibrary_CX_GLOBAL_P_H
