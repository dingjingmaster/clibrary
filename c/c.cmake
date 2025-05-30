file(GLOB C_SRC
        ${CMAKE_SOURCE_DIR}/c/log.h
        ${CMAKE_SOURCE_DIR}/c/log.c

        ${CMAKE_SOURCE_DIR}/c/str.h
        ${CMAKE_SOURCE_DIR}/c/str.c

        ${CMAKE_SOURCE_DIR}/c/uri.h
        ${CMAKE_SOURCE_DIR}/c/uri.c

        ${CMAKE_SOURCE_DIR}/c/date.h
        ${CMAKE_SOURCE_DIR}/c/date.c

        ${CMAKE_SOURCE_DIR}/c/poll.h
        ${CMAKE_SOURCE_DIR}/c/poll.c

        ${CMAKE_SOURCE_DIR}/c/test.h
        ${CMAKE_SOURCE_DIR}/c/test.c

        ${CMAKE_SOURCE_DIR}/c/list.h
        ${CMAKE_SOURCE_DIR}/c/list.c

        ${CMAKE_SOURCE_DIR}/c/uuid.h
        ${CMAKE_SOURCE_DIR}/c/uuid.c

        ${CMAKE_SOURCE_DIR}/c/hash.h
        ${CMAKE_SOURCE_DIR}/c/hash.c

        ${CMAKE_SOURCE_DIR}/c/hook.h
        ${CMAKE_SOURCE_DIR}/c/hook.c

        ${CMAKE_SOURCE_DIR}/c/rcbox.h
        ${CMAKE_SOURCE_DIR}/c/rcbox.c

        ${CMAKE_SOURCE_DIR}/c/timer.h
        ${CMAKE_SOURCE_DIR}/c/timer.c

        ${CMAKE_SOURCE_DIR}/c/slist.h
        ${CMAKE_SOURCE_DIR}/c/slist.c

        ${CMAKE_SOURCE_DIR}/c/array.h
        ${CMAKE_SOURCE_DIR}/c/array.c

        ${CMAKE_SOURCE_DIR}/c/bytes.h
        ${CMAKE_SOURCE_DIR}/c/bytes.c

        ${CMAKE_SOURCE_DIR}/c/quark.h
        ${CMAKE_SOURCE_DIR}/c/quark.c

        ${CMAKE_SOURCE_DIR}/c/queue.h
        ${CMAKE_SOURCE_DIR}/c/queue.c

        ${CMAKE_SOURCE_DIR}/c/error.h
        ${CMAKE_SOURCE_DIR}/c/error.c

        ${CMAKE_SOURCE_DIR}/c/utils.h
        ${CMAKE_SOURCE_DIR}/c/utils.c

        ${CMAKE_SOURCE_DIR}/c/global.h
        ${CMAKE_SOURCE_DIR}/c/global.c

        ${CMAKE_SOURCE_DIR}/c/thread.h
        ${CMAKE_SOURCE_DIR}/c/thread.c

        ${CMAKE_SOURCE_DIR}/c/atomic.h
        ${CMAKE_SOURCE_DIR}/c/atomic.c

        ${CMAKE_SOURCE_DIR}/c/base64.h
        ${CMAKE_SOURCE_DIR}/c/base64.c

        ${CMAKE_SOURCE_DIR}/c/macros.h
        ${CMAKE_SOURCE_DIR}/c/macros.c

        ${CMAKE_SOURCE_DIR}/c/option.h
        ${CMAKE_SOURCE_DIR}/c/option.c

        ${CMAKE_SOURCE_DIR}/c/wakeup.h
        ${CMAKE_SOURCE_DIR}/c/wakeup.c

        ${CMAKE_SOURCE_DIR}/c/variant.h
        ${CMAKE_SOURCE_DIR}/c/variant.c

        ${CMAKE_SOURCE_DIR}/c/charset.h
        ${CMAKE_SOURCE_DIR}/c/charset.c

        ${CMAKE_SOURCE_DIR}/c/convert.h
        ${CMAKE_SOURCE_DIR}/c/convert.c

        ${CMAKE_SOURCE_DIR}/c/cstring.h
        ${CMAKE_SOURCE_DIR}/c/cstring.c

        ${CMAKE_SOURCE_DIR}/c/unicode.h
        ${CMAKE_SOURCE_DIR}/c/unicode.c

        ${CMAKE_SOURCE_DIR}/c/bit-lock.h
        ${CMAKE_SOURCE_DIR}/c/bit-lock.c

        ${CMAKE_SOURCE_DIR}/c/time-zone.h
        ${CMAKE_SOURCE_DIR}/c/time-zone.c

        ${CMAKE_SOURCE_DIR}/c/hash-table.h
        ${CMAKE_SOURCE_DIR}/c/hash-table.c

        ${CMAKE_SOURCE_DIR}/c/file-utils.h
        ${CMAKE_SOURCE_DIR}/c/file-utils.c

        ${CMAKE_SOURCE_DIR}/c/host-utils.h
        ${CMAKE_SOURCE_DIR}/c/host-utils.c

        ${CMAKE_SOURCE_DIR}/c/mapped-file.h
        ${CMAKE_SOURCE_DIR}/c/mapped-file.c
)

file(GLOB C_HEADERS
        ${CMAKE_SOURCE_DIR}/c/log.h
        ${CMAKE_SOURCE_DIR}/c/str.h
        ${CMAKE_SOURCE_DIR}/c/uri.h
        ${CMAKE_SOURCE_DIR}/c/clib.h
        ${CMAKE_SOURCE_DIR}/c/date.h
        ${CMAKE_SOURCE_DIR}/c/hash.h
        ${CMAKE_SOURCE_DIR}/c/poll.h
        ${CMAKE_SOURCE_DIR}/c/test.h
        ${CMAKE_SOURCE_DIR}/c/list.h
        ${CMAKE_SOURCE_DIR}/c/uuid.h
        ${CMAKE_SOURCE_DIR}/c/hook.h
        ${CMAKE_SOURCE_DIR}/c/error.h
        ${CMAKE_SOURCE_DIR}/c/timer.h
        ${CMAKE_SOURCE_DIR}/c/utils.h
        ${CMAKE_SOURCE_DIR}/c/array.h
        ${CMAKE_SOURCE_DIR}/c/bytes.h
        ${CMAKE_SOURCE_DIR}/c/slist.h
        ${CMAKE_SOURCE_DIR}/c/rcbox.h
        ${CMAKE_SOURCE_DIR}/c/quark.h
        ${CMAKE_SOURCE_DIR}/c/option.h
        ${CMAKE_SOURCE_DIR}/c/thread.h
        ${CMAKE_SOURCE_DIR}/c/atomic.h
        ${CMAKE_SOURCE_DIR}/c/base64.h
        ${CMAKE_SOURCE_DIR}/c/macros.h
        ${CMAKE_SOURCE_DIR}/c/wakeup.h
        ${CMAKE_SOURCE_DIR}/c/convert.h
        ${CMAKE_SOURCE_DIR}/c/cstring.h
        ${CMAKE_SOURCE_DIR}/c/unicode.h
        ${CMAKE_SOURCE_DIR}/c/charset.h
        ${CMAKE_SOURCE_DIR}/c/time-zone.h
        ${CMAKE_SOURCE_DIR}/c/host-utils.h
        ${CMAKE_SOURCE_DIR}/c/hash-table.h
        ${CMAKE_SOURCE_DIR}/c/file-utils.h
        ${CMAKE_SOURCE_DIR}/c/mapped-file.h
)
