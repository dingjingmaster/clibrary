include(c.cmake)

add_library(clibrary-c STATIC ${C_SRC})
target_include_directories(clibrary-c PUBLIC ${CMAKE_SOURCE_DIR})

install(TARGETS clibrary-c DESTINATION /usr/lib)
install(FILES ${C_HEADERS} DESTINATION /usr/include/clibrary/c/)