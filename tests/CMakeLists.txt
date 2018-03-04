cmake_minimum_required(VERSION 3.10.0)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17) # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...

add_executable(unicode_test unicode_test.cpp)

add_test(
    NAME unicode_test
    COMMAND $<TARGET_FILE:unicode_test>
)

set_tests_properties(unicode_test PROPERTIES LABELS unicode)
