
add_executable(sample_test sample_test.cpp)

add_test(
    NAME sample_test
    COMMAND $<TARGET_FILE:sample_test>
    CONFIGURATIONS Debug Release
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/tmp
)

set_tests_properties(sample_test PROPERTIES LABELS sample)
