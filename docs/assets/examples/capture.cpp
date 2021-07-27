#include <string>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
// The type of a lexy::lexeme depends on the input.
using file_lexeme = lexy::lexeme_for<lexy::read_file_result<>>;

struct production
{
    static constexpr auto rule = [] {
        auto name = dsl::capture(dsl::while_(dsl::ascii::alpha));
        return LEXY_LIT("My name is ") + name + dsl::period;
    }();

    // Same as `lexy::as_string<std::string>`.
    static constexpr auto value = lexy::callback<std::string>(
        [](file_lexeme lex) { return std::string(lex.begin(), lex.end()); });
};
//}

int main()
{
    auto input  = lexy_ext::read_file<>(stdin);
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("Hello %s!\n", result.value().c_str());
}

