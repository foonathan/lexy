#include <string>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/file.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    static constexpr auto rule = [] {
        return LEXY_LIT("\\u") >> dsl::code_point_id<4>    // \uXXXX
               | LEXY_LIT("\\U") >> dsl::code_point_id<8>; // \uXXXXXXXX
    }();

    // Encode the resulting code point as UTF-8.
    static constexpr auto value = lexy::as_string<std::string, lexy::utf8_encoding>;
};
//}

int main()
{
    auto input  = lexy::read_stdin<>().buffer();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The code point is: %s\n", result.value().c_str());
}

