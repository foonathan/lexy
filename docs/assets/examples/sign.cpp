#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct production
{
    // Sign followed by a decimal integer.
    static constexpr auto rule  = dsl::sign + dsl::integer<int>(dsl::digits<>);
    static constexpr auto value = lexy::as_integer<int>;
};
//}

int main()
{
    auto input  = lexy_ext::read_file<>(stdin);
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result.has_value())
        return 1;

    std::printf("The value is: %d\n", result.value());
    return result ? 0 : 1;
}

