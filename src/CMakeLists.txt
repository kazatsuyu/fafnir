cmake_minimum_required(VERSION 3.10.0)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17) # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
add_executable(clang main.cpp custom_api.cpp)
add_library(fafnir_injection SHARED dll_main.cpp custom_api.cpp)
# remove "lib" prefix
set_target_properties(fafnir_injection PROPERTIES PREFIX "")

target_link_libraries(fafnir_injection ImageHlp)
if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(clang stdc++fs)
    target_link_libraries(fafnir_injection stdc++fs)
endif()
if(MINGW)
    message(STATUS "Installing system-libraries: MinGW DLLs.")
    get_filename_component(Mingw_Path ${CMAKE_CXX_COMPILER} PATH)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${Mingw_Path}/libstdc++-6.dll ${Mingw_Path}/libwinpthread-1.dll)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # The exception model of msys2-mingw64-gcc is SEH
        list(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${Mingw_Path}/libgcc_s_seh-1.dll)
    else()
        # The exception model of msys2-mingw64-gcc is DWARF-2
        list(APPEND CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${Mingw_Path}/libgcc_s_dw2-1.dll)
    endif()
    include(InstallRequiredSystemLibraries)
    install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION bin COMPONENT System)
endif()

target_compile_definitions(fafnir_injection PRIVATE FAFNIR_DLL)

install(TARGETS clang RUNTIME DESTINATION bin)
install(TARGETS fafnir_injection RUNTIME DESTINATION bin)
