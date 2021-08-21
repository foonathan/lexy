#include <vector>

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
        auto integer = dsl::integer<int>(dsl::digits<>);
        return dsl::list(integer, dsl::sep(dsl::comma));
    }();

    static constexpr auto value = lexy::as_list<std::vector<int>>;
};
//}

int main()
{
    auto input  = lexy_ext::read_file<>(stdin).buffer();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("numbers: ");
    for (auto i : result.value())
        std::printf("%d ", i);
    std::putchar('\n');
}

