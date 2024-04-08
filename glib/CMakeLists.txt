include(../c/c.cmake)
include(./glib.cmake)

add_library(clibrary-glib SHARED ${C_SRC} ${GLIB_SRC})
target_link_libraries(clibrary-glib PUBLIC ${GLIB_LIBRARIES})
target_include_directories(clibrary-glib PUBLIC ${GLIB_INCLUDE_DIRS})

install(TARGETS clibrary-glib DESTINATION /usr/lib)
install(FILES ${C_HEADERS} DESTINATION /usr/include/clibrary/glib/)
