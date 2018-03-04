cmake_minimum_required(VERSION 3.10.0)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17) # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
add_executable(clang main.cpp custom_api.cpp)
add_library(fafnir_injection SHARED dll_main.cpp custom_api.cpp)

target_link_libraries(fafnir_injection ImageHlp)
if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(clang stdc++fs)
    target_link_libraries(fafnir_injection stdc++fs)
endif()
target_compile_definitions(fafnir_injection PRIVATE FAFNIR_DLL)

install(TARGETS clang RUNTIME DESTINATION bin)
install(TARGETS fafnir_injection RUNTIME DESTINATION bin)
