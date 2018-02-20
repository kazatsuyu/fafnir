#ifndef FAFNIR_UNICODE_HPP
#define FAFNIR_UNICODE_HPP

#include <iostream>

namespace fafnir {

inline char32_t read_utf8_stream(std::istream& in) noexcept {
    std::uint8_t ch;
    in.read(reinterpret_cast<char*>(&ch), 1);
    if (!in) {
        return std::char_traits<char32_t>::eof();
    }
    if (ch < 0x80) {
        return ch;
    } else if (ch < 0b1100'0000) {
        return U'\ufffd';
    } else if (ch < 0b1110'0000) {
        char32_t tmp = (ch & 0b0001'1111) << 6;
        in.read(reinterpret_cast<char*>(&ch), 1);
        if (!in || (ch & 0b1100'0000) != 0b1000'0000) {
            return U'\ufffd';
        }
        tmp |= (ch & 0b0011'1111);
        if (tmp < 0x80) {
            return U'\ufffd';
        }
        return tmp;
    } else if (ch < 0b1111'0000) {
        char32_t tmp = (ch & 0b0000'1111) << 12;
        in.read(reinterpret_cast<char*>(&ch), 1);
        if (!in || (ch & 0b1100'0000) != 0b1000'0000) {
            return U'\ufffd';
        }
        tmp |= (ch & 0b0011'1111) << 6;
        in.read(reinterpret_cast<char*>(&ch), 1);
        if (!in || (ch & 0b1100'0000) != 0b1000'0000) {
            return U'\ufffd';
        }
        tmp |= (ch & 0b0011'1111);
        if (tmp < 0x800 || (tmp >= 0xD800 && tmp < 0xE000)) {
            return U'\ufffd';
        }
        return tmp;
    } else if (ch < 0b1111'1000) {
        char32_t tmp = (ch & 0b0000'0111) << 18;
        in.read(reinterpret_cast<char*>(&ch), 1);
        if (!in || (ch & 0b1100'0000) != 0b1000'0000) {
            return U'\ufffd';
        }
        tmp |= (ch & 0b0011'1111) << 12;
        in.read(reinterpret_cast<char*>(&ch), 1);
        if (!in || (ch & 0b1100'0000) != 0b1000'0000) {
            return U'\ufffd';
        }
        tmp |= (ch & 0b0011'1111) << 6;
        in.read(reinterpret_cast<char*>(&ch), 1);
        if (!in || (ch & 0b1100'0000) != 0b1000'0000) {
            return U'\ufffd';
        }
        tmp |= (ch & 0b0011'1111);
        if (tmp < 0x10000) {
            return U'\ufffd';
        }
        return tmp;
    }
    return U'\ufffd';
}

inline char32_t read_utf16_stream(std::istream& in) noexcept {
    std::uint16_t ch;
    in.read(reinterpret_cast<char*>(&ch), 2);
    if (!in) {
        return std::char_traits<char32_t>::eof();
    }
    if (ch < 0xD800 || ch >= 0xE000) {
        return ch;
    } else if (ch >= 0xDC00) {
        return U'\ufffd';
    } else {
        char32_t tmp = (ch & 0x3FF) << 10;
        in.read(reinterpret_cast<char*>(&ch), 2);
        if (!in || ch < 0xDC00 || ch >= 0xE000) {
            return U'\ufffd';
        }
        tmp |= (ch & 0x3FF);
        return tmp + U'\U00010000';
    }
}

inline void write_utf8_stream(std::ostream& out, char32_t ch) noexcept {
    if ((ch >= 0xD800 && ch < 0xE000) || ch >= 0x110000) {
        out.write(u8"\ufffd", 3);
    } else if (ch < 0x80) {
        auto tmp = char(ch);
        out.write(&tmp, 1);
    } else if (ch < 0x800) {
        auto tmp = char((ch >> 6) | 0b1100'0000);
        out.write(&tmp, 1);
        tmp = char((ch & 0b0011'1111) | 0b1000'0000);
        out.write(&tmp, 1);
    } else if (ch < 0x10000) {
        auto tmp = char((ch >> 12) | 0b1110'0000);
        out.write(&tmp, 1);
        tmp = char(((ch >> 6) & 0b0011'1111) | 0b1000'0000);
        out.write(&tmp, 1);
        tmp = char((ch & 0b0011'1111) | 0b1000'0000);
        out.write(&tmp, 1);
    } else {
        auto tmp = char((ch >> 18) | 0b1111'0000);
        out.write(&tmp, 1);
        tmp = char(((ch >> 12) & 0b0011'1111) | 0b1000'0000);
        out.write(&tmp, 1);
        tmp = char(((ch >> 6) & 0b0011'1111) | 0b1000'0000);
        out.write(&tmp, 1);
        tmp = char((ch & 0b0011'1111) | 0b1000'0000);
        out.write(&tmp, 1);
    }
}

inline void write_utf16_stream(std::ostream& out, char32_t ch) noexcept {
    if ((ch >= 0xD800 && ch < 0xE000) || ch >= 0x110000) {
        out.write(reinterpret_cast<const char*>(u"\ufffd"), 2);
    } else if (ch < 0x10000) {
        auto tmp = char16_t(ch);
        out.write(reinterpret_cast<const char*>(&tmp), 2);
    } else {
        auto tmp = char16_t(((ch - 0x10000) >> 10) + 0xD800);
        out.write(reinterpret_cast<const char*>(&tmp), 2);
        tmp = char16_t(((ch - 0x10000) & 0x3FF) + 0xDC00);
        out.write(reinterpret_cast<const char*>(&tmp), 2);
    }
}

}

#endif
