#include "../src/unicode.hpp"
#include <iutest.hpp>
#include <sstream>
#include <string_view>

IUTEST(unicode, test_read_utf8_stream) {
    std::stringstream ss(u8"\1\x7F\u0080\uFFFF\U00010000\U0010FFFF");
    IUTEST_ASSERT_EQ(U'\1', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\x7F', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\u0080', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\uFFFF', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\U00010000', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\U0010FFFF', fafnir::read_utf8_stream(ss));
}

IUTEST(unicode, test_read_bad_utf8_stream) {
    std::stringstream ss("\x80\xF0\x80\x80\x80\xED\x9F\xBF\xED\xA0\x80\xED\xBF\xBF\xEE\x80\x80");
    IUTEST_ASSERT_EQ(U'\uFFFD', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\uFFFD', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\uD7FF', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\uFFFD', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\uFFFD', fafnir::read_utf8_stream(ss));
    IUTEST_ASSERT_EQ(U'\uE000', fafnir::read_utf8_stream(ss));
}

IUTEST(unicode, test_read_utf16_stream) {
    std::u16string_view str = u"\1\uFFFF\U00010000\U0010FFFF";
    std::stringstream ss({
        reinterpret_cast<const char*>(str.data()),
        reinterpret_cast<const char*>(str.data() + str.size())
    });
    IUTEST_ASSERT_EQ(U'\1', fafnir::read_utf16_stream(ss));
    IUTEST_ASSERT_EQ(U'\uFFFF', fafnir::read_utf16_stream(ss));
    IUTEST_ASSERT_EQ(U'\U00010000', fafnir::read_utf16_stream(ss));
    IUTEST_ASSERT_EQ(U'\U0010FFFF', fafnir::read_utf16_stream(ss));
}

IUTEST(unicode, test_read_bad_stream) {
    char str[] = "\xFF\xD7\x00\xD8\x01\x00\xFF\xDF\x00\xE0";
    std::stringstream ss({str, str + sizeof(str) - 1});
    IUTEST_ASSERT_EQ(U'\uD7FF', fafnir::read_utf16_stream(ss));
    IUTEST_ASSERT_EQ(U'\uFFFD', fafnir::read_utf16_stream(ss));
    IUTEST_ASSERT_EQ(U'\uFFFD', fafnir::read_utf16_stream(ss));
    IUTEST_ASSERT_EQ(U'\uE000', fafnir::read_utf16_stream(ss));
}

IUTEST(unicode, test_write_utf8_stream) {
    std::stringstream ss;
    fafnir::write_utf8_stream(ss, U'\1');
    fafnir::write_utf8_stream(ss, U'\x7F');
    fafnir::write_utf8_stream(ss, U'\u0080');
    fafnir::write_utf8_stream(ss, U'\uFFFF');
    fafnir::write_utf8_stream(ss, U'\U00010000');
    fafnir::write_utf8_stream(ss, U'\U0010FFFF');
    IUTEST_ASSERT_EQ(u8"\1\x7F\u0080\uFFFF\U00010000\U0010FFFF", ss.str());
}

IUTEST(unicode, test_write_bad_utf8_stream) {
    std::stringstream ss;
    fafnir::write_utf8_stream(ss, U'\uD7FF');
    fafnir::write_utf8_stream(ss, 0xD800);
    fafnir::write_utf8_stream(ss, 0xDFFF);
    fafnir::write_utf8_stream(ss, U'\uE000');
    fafnir::write_utf8_stream(ss, U'\U0010FFFF');
    fafnir::write_utf8_stream(ss, 0x00110000);
    IUTEST_ASSERT_EQ(u8"\uD7FF\uFFFD\uFFFD\uE000\U0010FFFF\uFFFD", ss.str());
}

IUTEST(unicode, test_write_utf16_stream) {
    std::stringstream ss;
    fafnir::write_utf16_stream(ss, U'\1');
    fafnir::write_utf16_stream(ss, U'\uFFFF');
    fafnir::write_utf16_stream(ss, U'\U00010000');
    fafnir::write_utf16_stream(ss, U'\U0010FFFF');
    auto str = ss.str();
    IUTEST_ASSERT_EQ(
        u"\1\uFFFF\U00010000\U0010FFFF",
        std::u16string(
            reinterpret_cast<const char16_t*>(str.data()),
            reinterpret_cast<const char16_t*>(str.data() + str.size())
        )
    );
}

IUTEST(unicode, test_write_bad_utf16_stream) {
    std::stringstream ss;
    fafnir::write_utf16_stream(ss, U'\uD7FF');
    fafnir::write_utf16_stream(ss, 0xD800);
    fafnir::write_utf16_stream(ss, 0xDFFF);
    fafnir::write_utf16_stream(ss, U'\uE000');
    fafnir::write_utf16_stream(ss, U'\U0010FFFF');
    fafnir::write_utf16_stream(ss, 0x00110000);
    auto str = ss.str();
    IUTEST_ASSERT_EQ(
        u"\uD7FF\uFFFD\uFFFD\uE000\U0010FFFF\uFFFD",
        std::u16string(
            reinterpret_cast<const char16_t*>(str.data()),
            reinterpret_cast<const char16_t*>(str.data() + str.size())
        )
    );
}

int main(int argc, char** argv) {
    IUTEST_INIT(&argc, argv);
    return IUTEST_RUN_ALL_TESTS();
}
