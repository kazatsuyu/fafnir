cmake_minimum_required(VERSION 3.10)

project(fafnir VERSION 1.0.0.1)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GNU|Intel")
    set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS}")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "/std:c++17 ${CMAKE_CXX_FLAGS}")
endif()

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

install(FILES install.bat README.md LICENSE DESTINATION ".")
install(DIRECTORY scripts assets DESTINATION ".")
