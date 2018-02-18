#include <string_view>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <sstream>

#include <Windows.h>

#include "unicode.hpp"

namespace fafnir {

std::experimental::filesystem::path get_executable_path() {
    for(std::vector<wchar_t> buf(256);;buf.resize(buf.size() * 2)) {
        std::size_t size = GetModuleFileNameW(nullptr, buf.data(), buf.size());
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return {buf.begin(), buf.begin() + size};
        }
    }
}

enum class file_type {
    utf8,
    utf8_with_bom,
    utf16_le,
};

file_type get_file_type(std::istream& in) noexcept {
    char ch;
    in.read(&ch, 1);
    if (in) {
        if (ch == '\xff') {
            in.read(&ch, 1);
            if (in) {
                in.seekg(-2, std::ios::cur);
                if (ch == '\xfe') {
                    return file_type::utf16_le;
                }
            } else {
                in.seekg(-1, std::ios::cur);
            }
        } else if (ch == '\xef') {
            in.read(&ch, 1);
            if (in) {
                if (ch == '\xbb') {
                    in.read(&ch, 1);
                    if (in) {
                        in.seekg(-3, std::ios::cur);
                        if (ch == '\xbf') {
                            return file_type::utf8_with_bom;
                        }
                    } else {
                        in.seekg(-2, std::ios::cur);
                    }
                } else {
                    in.seekg(-2, std::ios::cur);
                }
            } else {
                in.seekg(-1, std::ios::cur);
            }
        } else {
            in.seekg(-1, std::ios::cur);
        }
    }
    in.clear();
    return file_type::utf8;
}

template<auto f>
std::experimental::filesystem::path read_path_impl(std::istream& in) noexcept {
    std::ostringstream ss;

    for (auto ch = f(in); ch != std::char_traits<char32_t>::eof(); ch = f(in)) {
        write_utf16_stream(ss, ch);
    }
    
    auto str = ss.str();
    return {reinterpret_cast<const wchar_t*>(str.data()),
            reinterpret_cast<const wchar_t*>(str.data() + str.size())};
}

std::experimental::filesystem::path read_path(std::istream& in) noexcept {
    auto type = get_file_type(in);
    if (type == file_type::utf8_with_bom) {
        in.seekg(3);
    } else if (type == file_type::utf16_le) {
        in.seekg(2);
        return read_path_impl<read_utf16_stream>(in);
    }
    return read_path_impl<read_utf8_stream>(in);
}

}

int main() {
    using namespace fafnir;

    auto path = get_executable_path().parent_path() / ".target";
    if (!std::experimental::filesystem::exists(path)) {
        std::wcerr << "error: " << path << " doesn't exist." << std::endl;
        return 1;
    }
    std::ifstream target(path, std::ios::binary);
    auto target_path = read_path(target);
    if (!std::experimental::filesystem::exists(target_path)) {
        std::wcerr << "error: " << target_path << " doesn't exist." << std::endl;
        return 1;
    }
    const std::wstring_view cmdline = GetCommandLineW();
    auto itr = cmdline.begin();
    while (itr != cmdline.end() && *itr != ' ') {
        if (*itr == '"') {
            ++itr;
            while (itr != cmdline.end() && *itr != '"') {
                ++itr;
            }
        }
        ++itr;
    }
    std::wstring_view additional_option(L" --rsp-quoting=windows");
    std::vector<wchar_t> cmdbuf;
    auto new_cmdline = cmdline.substr(itr - cmdline.begin());
    cmdbuf.reserve(target_path.native().size() + new_cmdline.size() + additional_option.size() + 3);
    cmdbuf.push_back('"');
    cmdbuf.insert(cmdbuf.end(), target_path.native().begin(), target_path.native().end());
    cmdbuf.push_back('"');
    cmdbuf.insert(cmdbuf.end(), new_cmdline.begin(), new_cmdline.end());
    cmdbuf.insert(cmdbuf.end(), additional_option.begin(), additional_option.end());
    cmdbuf.push_back(L'\0');
    STARTUPINFOW si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    CreateProcessW(target_path.c_str(), cmdbuf.data(), nullptr, nullptr, true, 0, nullptr, nullptr, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    return exit_code;
}
