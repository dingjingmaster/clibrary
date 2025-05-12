file(GLOB CX_CORE_SRC
        ${CMAKE_SOURCE_DIR}/cx/base/core/global/flags.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/global/global.h

#        ${CMAKE_SOURCE_DIR}/cx/base/core/global/cx-global.h
#        ${CMAKE_SOURCE_DIR}/cx/base/core/global/cx-global.cpp


        ${CMAKE_SOURCE_DIR}/cx/base/core/text/string.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/text/string.cpp

        ${CMAKE_SOURCE_DIR}/cx/base/core/tools/tools_p.h

        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object_p.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object.cpp

        ${CMAKE_SOURCE_DIR}/cx/base/core/text/bytearray-list.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/text/bytearray-list.cpp


        ${CMAKE_SOURCE_DIR}/cx/base/core/global/define.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/type-info.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/global/name-space.h
)

file(GLOB CX_CORE_HEADER
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object.h

        ${CMAKE_SOURCE_DIR}/cx/base/core/global/define.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/type-info.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/global/name-space.h
)

file(GLOB CX_CORE_PRIVATE_HEADER
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object_p.h
)
