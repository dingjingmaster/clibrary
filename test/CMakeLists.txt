include(../c/c.cmake)

add_executable(c-str-test c-str-test.c ${C_SRC})
add_test(NAME c-str-test COMMAND c-str-test)
set_property (TEST c-str-test PROPERTY PASS_REGULAR_EXPRESSION "Test passed! -- c/str")
set_property (TEST c-str-test PROPERTY FAIL_REGULAR_EXPRESSION "Test failed! -- c/str")