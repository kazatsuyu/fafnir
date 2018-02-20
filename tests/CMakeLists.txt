
add_executable(unicode_test unicode_test.cpp)

add_test(
    NAME unicode_test
    COMMAND $<TARGET_FILE:unicode_test>
)

set_tests_properties(unicode_test PROPERTIES LABELS unicode)
