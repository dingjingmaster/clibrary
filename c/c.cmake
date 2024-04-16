file(GLOB C_SRC
        ${CMAKE_SOURCE_DIR}/c/log.h
        ${CMAKE_SOURCE_DIR}/c/log.c

        ${CMAKE_SOURCE_DIR}/c/str.h
        ${CMAKE_SOURCE_DIR}/c/str.c

        ${CMAKE_SOURCE_DIR}/c/test.h
        ${CMAKE_SOURCE_DIR}/c/test.c

        ${CMAKE_SOURCE_DIR}/c/list.h
        ${CMAKE_SOURCE_DIR}/c/list.c

        ${CMAKE_SOURCE_DIR}/c/slist.h
        ${CMAKE_SOURCE_DIR}/c/slist.c

        ${CMAKE_SOURCE_DIR}/c/array.h
        ${CMAKE_SOURCE_DIR}/c/array.c

        ${CMAKE_SOURCE_DIR}/c/base64.h
        ${CMAKE_SOURCE_DIR}/c/base64.c

        ${CMAKE_SOURCE_DIR}/c/macros.h
        ${CMAKE_SOURCE_DIR}/c/macros.c
)

file(GLOB C_HEADERS
        ${CMAKE_SOURCE_DIR}/c/log.h
        ${CMAKE_SOURCE_DIR}/c/str.h
        ${CMAKE_SOURCE_DIR}/c/clib.h
        ${CMAKE_SOURCE_DIR}/c/test.h
        ${CMAKE_SOURCE_DIR}/c/list.h
        ${CMAKE_SOURCE_DIR}/c/array.h
        ${CMAKE_SOURCE_DIR}/c/slist.h
        ${CMAKE_SOURCE_DIR}/c/base64.h
        ${CMAKE_SOURCE_DIR}/c/macros.h
)
