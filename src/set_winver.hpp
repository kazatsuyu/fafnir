#ifndef FAFNIR_SET_WINVER_HPP
#define FAFNIR_SET_WINVER_HPP
#ifdef _WIN32_WINNT
#   if _WIN32_WINNT < 0x0600
#       error "target OS must be Windows Vista or higher"
#   endif
#endif
#ifdef __GNUC__
#   define _WIN32_WINNT 0x0600 // Windows Vista
#endif
#endif //FAFNIR_SET_WINVER_HPP
