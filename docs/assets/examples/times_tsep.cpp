#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
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

    static constexpr auto value
        = lexy::callback<int>([](int a, int b, int c) { return a + b + c; });
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
