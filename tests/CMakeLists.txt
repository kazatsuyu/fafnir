cmake_minimum_required(VERSION 3.10.0)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17) # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
if(EXISTS ${CMAKE_SOURCE_DIR}/submodules/iutest/include)
    set(IUTEST_ROOT_DIR ${CMAKE_SOURCE_DIR}/submodules/iutest)
endif()
include(FindIutest)
find_package(IUTEST)

add_executable(unicode_test unicode_test.cpp)
include_directories(${IUTEST_INCLUDE_DIR})
target_link_libraries(unicode_test Ws2_32)
add_test(
    NAME unicode_test
    COMMAND $<TARGET_FILE:unicode_test>
)

set_tests_properties(unicode_test PROPERTIES LABELS unicode)
