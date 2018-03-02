add_executable(clang main.cpp custom_api.cpp)
add_library(fafnir_injection SHARED dll_main.cpp custom_api.cpp)

target_link_libraries(fafnir_injection ImageHlp)
target_compile_definitions(fafnir_injection PRIVATE FAFNIR_DLL)

install(TARGETS clang RUNTIME DESTINATION bin)
install(TARGETS fafnir_injection RUNTIME DESTINATION bin)
