#include <memory>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/parse.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct point
{
    int x, y;
};

struct production
{
    static constexpr auto rule  = dsl::twice(dsl::integer<int>(dsl::digits<>));
    static constexpr auto value = lexy::new_<point, std::unique_ptr<point>>;
};
//}

int main()
{
    auto input  = lexy_ext::read_file<>(stdin);
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("The value is: (%d, %d)\n", result.value()->x, result.value()->y);
}

