#include <optional>
#include <string>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/parse.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

struct decimal
{
    int                        integer;
    std::optional<std::string> fraction;
};

//{
struct production
{
    struct fraction
    {
        static constexpr auto rule  = dsl::capture(dsl::digits<>);
        static constexpr auto value = lexy::as_string<std::string>;
    };

    static constexpr auto rule = [] {
        auto integer = dsl::integer<int>(dsl::digits<>);

        return integer + dsl::opt(dsl::period >> dsl::p<fraction>);
    }();

    static constexpr auto value = lexy::construct<decimal>;
};
//}

int main()
{
    auto input  = lexy_ext::read_file<>(stdin);
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The value is: %d.%s\n", result.value().integer,
                result.value().fraction.value_or("0").c_str());
}

