#include <fstream>

#include "fafnir.hpp"
#include "custom_api.hpp"

namespace fafnir {

namespace {

auto get_dll_name() {
    return get_bin_path() / L"fafnir_injection.dll";
}

struct view_delete {
    void operator()(const void* ptr) {
        UnmapViewOfFile(ptr);
    }
};

void inject(HANDLE process) {
    const auto dll_path = get_dll_name();
    const auto size = (dll_path.native().size() + 1) * sizeof(wchar_t);
    const auto ptr = VirtualAllocEx(process, nullptr, size, MEM_COMMIT, PAGE_READWRITE);

    WriteProcessMemory(process, ptr, dll_path.c_str(), size, nullptr);
    const auto routine = reinterpret_cast<LPTHREAD_START_ROUTINE>(
        GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW")
    );
    const  handle_ptr thread{CreateRemoteThread(process, nullptr, 0, routine, ptr, 0, nullptr)};
    WaitForSingleObject(thread.get(), INFINITE);
    VirtualFreeEx(process, ptr, size, MEM_RELEASE);
}

const auto orig_CreateProcessW = reinterpret_cast<decltype(CreateProcessW)*>(
    GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateProcessW")
);
const auto orig_CreateProcessA = reinterpret_cast<decltype(CreateProcessA)*>(
    GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateProcessA")
);
const auto orig_MoveFileW = reinterpret_cast<decltype(MoveFileW)*>(
    GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "MoveFileW")
);
const auto orig_MoveFileA = reinterpret_cast<decltype(MoveFileA)*>(
    GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "MoveFileA")
);
const auto orig_MoveFileExW = reinterpret_cast<decltype(MoveFileExW)*>(
    GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "MoveFileExW")
);
const auto orig_MoveFileExA = reinterpret_cast<decltype(MoveFileExA)*>(
    GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "MoveFileExA")
);
const auto orig_MoveFileWithProgressW = reinterpret_cast<decltype(MoveFileWithProgressW)*>(
    GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "MoveFileWithProgressW")
);
const auto orig_MoveFileWithProgressA = reinterpret_cast<decltype(MoveFileWithProgressA)*>(
    GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "MoveFileWithProgressA")
);
const auto orig_SetFileInformationByHandle = reinterpret_cast<decltype(SetFileInformationByHandle)*>(
    GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetFileInformationByHandle")
);

}

BOOL WINAPI create_process_w(
    LPCWSTR application_name,
    LPWSTR command_line,
    LPSECURITY_ATTRIBUTES process_attributes,
    LPSECURITY_ATTRIBUTES thread_attributes,
    BOOL inherit_handles,
    DWORD creation_flags,
    LPVOID environment,
    LPCWSTR current_directory,
    LPSTARTUPINFOW startup_info,
    LPPROCESS_INFORMATION process_information
) {
    auto r = fafnir::orig_CreateProcessW(
        application_name,
        command_line,
        process_attributes,
        thread_attributes,
        inherit_handles,
        creation_flags | CREATE_SUSPENDED,
        environment,
        current_directory,
        startup_info,
        process_information
    );

    if (!r) {
        return r;
    }

    fafnir::inject(process_information->hProcess);

    if (!(creation_flags & CREATE_SUSPENDED)) {
        ResumeThread(process_information->hThread);
    }
    return r;
}

BOOL WINAPI create_process_a(
    LPCSTR application_name,
    LPSTR command_line,
    LPSECURITY_ATTRIBUTES process_attributes,
    LPSECURITY_ATTRIBUTES thread_attributes,
    BOOL inherit_handles,
    DWORD creation_flags,
    LPVOID environment,
    LPCSTR current_directory,
    LPSTARTUPINFOA startup_info,
    LPPROCESS_INFORMATION process_information
) {
    auto r = orig_CreateProcessA(
        application_name,
        command_line,
        process_attributes,
        thread_attributes,
        inherit_handles,
        creation_flags | CREATE_SUSPENDED,
        environment,
        current_directory,
        startup_info,
        process_information
    );

    if (!r) {
        return r;
    }

    inject(process_information->hProcess);

    if (!(creation_flags & CREATE_SUSPENDED)) {
        ResumeThread(process_information->hThread);
    }
    return r;
}

BOOL WINAPI set_file_information_by_handle(
    HANDLE file,
    FILE_INFO_BY_HANDLE_CLASS information_class,
    LPVOID file_information,
    DWORD size
) {
    if (information_class == FileRenameInfo) {
        auto& info = *static_cast<PFILE_RENAME_INFO>(file_information);
        std::ofstream(info.FileName, std::ios::ate | std::ios::binary);
    }
    return orig_SetFileInformationByHandle(file, information_class, file_information, size);
}

} // namespace fafnir
