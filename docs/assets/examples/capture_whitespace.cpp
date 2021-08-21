#include <string>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto name = dsl::no_whitespace(dsl::capture(dsl::while_(dsl::ascii::alpha)));
        return LEXY_LIT("My name is") + name + dsl::period;
    }();

    static constexpr auto value = lexy::as_string<std::string>;
};
//}

int main()
{
    auto input  = lexy_ext::read_file<>(stdin).buffer();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("Hello %s!\n", result.value().c_str());
}

