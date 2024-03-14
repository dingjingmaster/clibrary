include(../c/c.cmake)

# c 测试
add_executable(test-c-str test-c-str.c)
target_link_libraries(test-c-str PUBLIC clibrary-c)
target_link_directories(test-c-str PUBLIC ${CMAKE_BINARY_DIR}/c)
add_test(NAME test-c-str COMMAND test-c-str)


