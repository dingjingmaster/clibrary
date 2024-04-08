include(./qt5.cmake)
include(../c/c.cmake)

add_library(clibrary-qt5 SHARED ${C_SRC} ${QT5_SRC})
target_link_libraries(clibrary-qt5 PUBLIC ${QT_CORE_LIBRARIES})
target_include_directories(clibrary-qt5 PUBLIC ${QT_CORE_INCLUDE_DIRS})

install(TARGETS clibrary-qt5 DESTINATION /usr/lib)
install(FILES ${QT5_HEADERS} DESTINATION /usr/include/clibrary/qt5/)
