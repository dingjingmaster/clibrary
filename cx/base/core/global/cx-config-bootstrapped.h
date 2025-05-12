//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_CX_CONFIG_BOOTSTRAPPED_H
#define clibrary_CX_CONFIG_BOOTSTRAPPED_H

#ifdef CX_BOOTSTRAPPED

#ifndef CX_NO_EXCEPTIONS
#define CX_NO_EXCEPTIONS
#endif

#define CX_NO_USING_NAMESPACE
#define CX_NO_DEPRECATED

// Keep feature-test macros in alphabetic order by feature name:
#define CX_FEATURE_alloca 1
#define CX_FEATURE_alloca_h -1
#ifdef _WIN32
# define CX_FEATURE_alloca_malloc_h 1
#else
# define CX_FEATURE_alloca_malloc_h -1
#endif
#define CX_FEATURE_binaryjson -1
#define CX_FEATURE_cborstreamreader -1
#define CX_FEATURE_cborstreamwriter 1
#define CX_CRYPTOGRAPHICHASH_ONLY_SHA1
#define CX_FEATURE_cxx11_random (__has_include(<random>) ? 1 : -1)
#define CX_NO_DATASTREAM
#define CX_FEATURE_datestring 1
#define CX_FEATURE_datetimeparser -1
#define CX_FEATURE_easingcurve -1
#define CX_FEATURE_etw -1
#define CX_FEATURE_getauxval (__has_include(<sys/auxv.h>) ? 1 : -1)
#define CX_FEATURE_getentropy -1
#define CX_NO_GEOM_VARIANT
#define CX_FEATURE_hijricalendar -1
#define CX_FEATURE_iconv -1
#define CX_FEATURE_icu -1
#define CX_FEATURE_islamiccivilcalendar -1
#define CX_FEATURE_jalalicalendar -1
#define CX_FEATURE_journald -1
#define CX_FEATURE_futimens -1
#define CX_FEATURE_futimes -1
#define CX_FEATURE_itemmodel -1
#define CX_FEATURE_library -1
#ifdef __linux__
# define CX_FEATURE_linkat 1
#else
# define CX_FEATURE_linkat -1
#endif
#define CX_FEATURE_lttng -1
#define CX_NO_QOBJECT
#define CX_FEATURE_process -1
#define CX_FEATURE_regularexpression -1
#ifdef __GLIBC_PREREQ
# define CX_FEATURE_renameat2 (__GLIBC_PREREQ(2, 28) ? 1 : -1)
#else
# define CX_FEATURE_renameat2 -1
#endif
#define CX_FEATURE_sharedmemory -1
#define CX_FEATURE_signaling_nan -1
#define CX_FEATURE_slog2 -1
#ifdef __GLIBC_PREREQ
# define CX_FEATURE_statx (__GLIBC_PREREQ(2, 28) ? 1 : -1)
#else
# define CX_FEATURE_statx -1
#endif
#define CX_FEATURE_syslog -1
#define CX_NO_SYSTEMLOCALE
#define CX_FEATURE_systemsemaphore -1
#define CX_FEATURE_temporaryfile 1
#define CX_FEATURE_textdate 1
#define CX_FEATURE_thread -1
#define CX_FEATURE_timezone -1
#define CX_FEATURE_topleveldomain -1
#define CX_NO_TRANSLATION
#define CX_FEATURE_translation -1

#ifndef CX_FEATURE_zstd
#define CX_FEATURE_zstd -1
#endif

#ifdef CX_BUILD_QMAKE
#define CX_FEATURE_commandlineparser -1
#define CX_NO_COMPRESS
#define CX_JSON_READONLY
#define CX_FEATURE_settings 1
#define CX_NO_STANDARDPATHS
#define CX_FEATURE_textcodec -1
#else
#define CX_FEATURE_codecs -1
#define CX_FEATURE_commandlineparser 1
#define CX_FEATURE_settings -1
#define CX_FEATURE_textcodec 1
#endif

#endif

#endif // clibrary_CX_CONFIG_BOOTSTRAPPED_H
