#include <optional>

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/file.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct version
{
    std::optional<int> major, minor, patch;
};

struct production
{
    static constexpr auto rule = [] {
        // If we don't have an integer, recover by producing nullopt.
        auto number = dsl::try_(dsl::integer<int>(dsl::digits<>), dsl::nullopt);
        auto dot    = dsl::try_(dsl::period);
        return number + dot + number + dot + number;
    }();

    static constexpr auto value = lexy::construct<version>;
};
//}

int main()
{
    auto input  = lexy::read_stdin<>().buffer();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result.has_value())
        return 1;

    auto [major, minor, patch] = result.value();
    std::printf("The value is: %d.%d.%d\n", major.value_or(0), minor.value_or(0),
                patch.value_or(0));
    return result ? 0 : 1;
}

