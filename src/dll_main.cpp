#include "set_winver.hpp"
#include <cstddef>
#include <Windows.h>
#include <TlHelp32.h>
#include <Dbghelp.h>

#include "custom_api.hpp"
#include "fafnir.hpp"

namespace fafnir {

namespace {

auto get_image_import_descriptors(HINSTANCE module) {
    ULONG size;
    return reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
        ImageDirectoryEntryToData(module, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size)
    );
}

template<typename F>
auto enum_descriptor(PIMAGE_IMPORT_DESCRIPTOR desc, const F& f) {
    if (!desc) {
        return;
    }
    while (desc->Name) {
        f(*desc);
        desc++;
    }
}

template<typename F>
auto enum_thunk(PIMAGE_THUNK_DATA thunk, const F& f) {
    if (thunk) {
        while (thunk->u1.Function) {
            f(*thunk);
            thunk++;
        }
    }
}

void modify_iat(const char* modname, void *origaddr, void *newaddr, HINSTANCE module) {
    auto import_desc = get_image_import_descriptors(module);
    enum_descriptor(import_desc, [=](const IMAGE_IMPORT_DESCRIPTOR& desc){
        auto base = reinterpret_cast<std::byte*>(module);
        auto name = reinterpret_cast<const char*>(base + desc.Name);
        if (lstrcmpiA(name, modname)) {
            return;
        }
        auto pThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(base + desc.FirstThunk);
        enum_thunk(pThunk, [&](IMAGE_THUNK_DATA& thunk) {
            auto& addr = reinterpret_cast<void*&>(thunk.u1.Function);
            if (addr == origaddr) {
                DWORD old_protect;
                VirtualProtect(&addr, sizeof(addr), PAGE_EXECUTE_READWRITE, &old_protect);
                addr = newaddr;
                VirtualProtect(&addr, sizeof(addr), old_protect, &old_protect);
            }
        });
    });
}

template<typename F>
void enum_modules(HANDLE snap, const F& f) {
    MODULEENTRY32W me{sizeof(me)};
    for (auto r = Module32FirstW(snap, &me); r ;r = Module32NextW(snap, &me)) {
        f(me.hModule);
    }
}

template<typename R, typename ...Args>
void modify_all_iat(const char *modname, R (WINAPI *origaddr)(Args...), R (WINAPI *newaddr)(Args...)) {
    handle_ptr snap{CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId())};
    enum_modules(snap.get(), [&](HINSTANCE module){
        modify_iat(modname, reinterpret_cast<void*>(origaddr), reinterpret_cast<void*>(newaddr), module);
    });
}

} // namespace

} // namespace fafnir


BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if(reason == DLL_PROCESS_ATTACH) {
        fafnir::modify_all_iat("kernel32.dll", CreateProcessW, fafnir::create_process_w);
        fafnir::modify_all_iat("kernel32.dll", CreateProcessA, fafnir::create_process_a);
        fafnir::modify_all_iat("kernel32.dll", SetFileInformationByHandle, fafnir::set_file_information_by_handle);
    }
    return TRUE;
}
