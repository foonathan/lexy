#include <string>
#include <vector>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/parse.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/report_error.hpp>

namespace dsl = lexy::dsl;

//{
struct entry
{
    std::string      name;
    std::vector<int> numbers;
};

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto integer = dsl::integer<int>(dsl::digits<>);
        return dsl::square_bracketed.list(integer, dsl::sep(dsl::comma));
    }();

    static constexpr auto value
        // Collect all the numbers in a vector, then turn the result into an entry.
        = lexy::as_list<std::vector<int>> >> lexy::callback<entry>([](auto&& vec) {
              return entry{"foo", std::move(vec)};
          });
};
//}

int main()
{
    auto input  = lexy_ext::read_file<>(stdin);
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::printf("%s: ", result.value().name.c_str());
    for (auto i : result.value().numbers)
        std::printf("%d ", i);
    std::putchar('\n');
}

