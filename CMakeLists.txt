cmake_minimum_required(VERSION 3.10.0)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17) # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...

project(fafnir VERSION 1.0.1.0)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

install(FILES install.bat README.md LICENSE DESTINATION ".")
install(DIRECTORY scripts assets DESTINATION ".")
