#include <lexy/action/validate.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    static constexpr auto rule = [] {
        auto utf8  = dsl::encode<lexy::utf8_encoding>(LEXY_LIT(u8"ä"));
        auto utf16 = dsl::encode<lexy::utf16_encoding>(LEXY_LIT(u"ä"));
        auto utf32 = dsl::encode<lexy::utf32_encoding>(LEXY_LIT(U"ä"));

        return utf8 + dsl::newline + utf16 + dsl::newline + utf32 + dsl::eof;
    }();
};
//}

int main()
{
    const unsigned char data[]
        = {// UTF-8 BOM (3 bytes), UTF-8 ä (2 bytes), ASCII newline (1 byte)
           0xEF, 0xBB, 0xBF, 0xC3, 0xA4, 0x0A,
           // UTF-16 BOM big endian (2 bytes), UTF-16-BE ä (2 bytes), ASCII newline (1 byte)
           0xFE, 0xFF, 0x00, 0xE4, 0x0A,
           // UTF-32 BOM little endian (4 bytes), UTF-32-LE ä (4 bytes)
           0xFF, 0xFE, 0x00, 0x00, 0xE4, 0x00, 0x00, 0x00};

    auto input  = lexy::string_input<lexy::byte_encoding>(data, sizeof(data));
    auto result = lexy::validate<production>(input, lexy_ext::report_error);
    return result ? 0 : 1;
}

