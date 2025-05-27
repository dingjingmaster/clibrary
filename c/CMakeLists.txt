include(c.cmake)

add_library(clibrary-c SHARED ${C_SRC})
target_compile_options(clibrary-c PUBLIC -fPIC)
target_include_directories(clibrary-c PUBLIC ${CMAKE_SOURCE_DIR})
set_target_properties(clibrary-c PROPERTIES VERSION "${PROJECT_VERSION}")
target_link_libraries(clibrary-c PUBLIC -lpthread)

add_library(clibrary-c-static STATIC ${C_SRC})
target_compile_options(clibrary-c-static PUBLIC -fPIC)
target_include_directories(clibrary-c-static PUBLIC ${CMAKE_SOURCE_DIR})
set_target_properties(clibrary-c-static PROPERTIES OUTPUT_NAME "clibrary-c")
set_target_properties(clibrary-c-static PROPERTIES VERSION "${PROJECT_VERSION}")

install(TARGETS clibrary-c DESTINATION lib64)
install(TARGETS clibrary-c-static DESTINATION lib64)
install(FILES ${C_HEADERS} DESTINATION include/clibrary/c/)
