#ifndef FAFNIR_FAFNIR_HPP
#define FAFNIR_FAFNIR_HPP

#include <filesystem>
#include <memory>

#include <Windows.h>

namespace fafnir {

inline std::experimental::filesystem::path get_bin_path() {
    auto module = GetModuleHandleW(L"fafnir_injection.dll");
    for(std::vector<wchar_t> buf(256);;buf.resize(buf.size() * 2)) {
        std::size_t size = GetModuleFileNameW(module, buf.data(), buf.size());
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            std::experimental::filesystem::path path{buf.begin(), buf.begin() + size};
            return path.parent_path();
        }
    }
}

struct handle_delete {
    void operator()(HANDLE handle) {
        CloseHandle(handle);
    }
};

using handle_ptr = std::unique_ptr<void, handle_delete>;

}

#endif
