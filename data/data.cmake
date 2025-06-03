configure_file(
        ${CMAKE_SOURCE_DIR}/data/clibrary-c-static.pc.in
        ${CMAKE_BINARY_DIR}/data/clibrary-c-static.pc
        @ONLY
)

configure_file(
        ${CMAKE_SOURCE_DIR}/data/clibrary-c.pc.in
        ${CMAKE_BINARY_DIR}/data/clibrary-c.pc
        @ONLY
)

install(FILES
        ${CMAKE_BINARY_DIR}/data/clibrary-c.pc
        ${CMAKE_BINARY_DIR}/data/clibrary-c-static.pc
        DESTINATION lib64/pkgconfig/)
#${CMAKE_SOURCE_DIR}/data/clibrary-qt5.pc
#${CMAKE_SOURCE_DIR}/data/clibrary-glib.pc
