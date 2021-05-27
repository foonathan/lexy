#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/parse.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    static constexpr auto rule = [] {
        auto item = dsl::integer<int>(dsl::digits<>);
        auto sep  = dsl::trailing_sep(dsl::comma);
        return dsl::times<3>(item, sep);
    }();

    static constexpr auto value = lexy::callback<int>(
        [](lexy::times<3, int> result) { return result[0] + result[1] + result[2]; });
};
//}

int main()
{
    auto input  = lexy_ext::read_file<>(stdin);
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The sum is: %d\n", result.value());
}
