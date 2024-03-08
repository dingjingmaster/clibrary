include(./qt5.cmake)
include(../c/c.cmake)

add_library(clibrary-qt5 SHARED ${C_SRC} ${QT5_SRC})
target_link_libraries(clibrary-qt5 PUBLIC ${QT_CORE_LIBRARIES})
target_include_directories(clibrary-qt5 PUBLIC ${QT_CORE_INCLUDE_DIRS})
