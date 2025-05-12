file(GLOB CX_CORE_SRC
        ${CMAKE_SOURCE_DIR}/cx/base/core/global/global.h

#        ${CMAKE_SOURCE_DIR}/cx/base/core/global/cx-global.h
#        ${CMAKE_SOURCE_DIR}/cx/base/core/global/cx-global.cpp


        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object_p.h
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object.cpp

        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/type-info.h
)

file(GLOB CX_CORE_HEADER
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object.h

        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/type-info.h
)

file(GLOB CX_CORE_PRIVATE_HEADER
        ${CMAKE_SOURCE_DIR}/cx/base/core/kernel/object_p.h
)
