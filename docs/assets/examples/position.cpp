#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input_location.hpp>
#include <lexy_ext/compiler_explorer.hpp>
#include <lexy_ext/report_error.hpp>
#include <vector>

namespace dsl = lexy::dsl;

//{
struct function
{
    // We use `lexy_ext::compiler_explorer_input()`,
    // which is a `lexy::buffer<lexy::utf8_encoding>`:
    // The position is a `const char8_t*`, and the lexeme
    // produced by `dsl::id` is a `lexy::buffer_lexeme<lexy::utf8_encoding>`.
    const LEXY_CHAR8_T*                      position;
    lexy::buffer_lexeme<lexy::utf8_encoding> name;
};

struct function_decl
{
    static constexpr auto rule = [] {
        auto id          = dsl::identifier(dsl::ascii::alpha);
        auto kw_function = LEXY_KEYWORD("function", id);

        auto arguments = dsl::parenthesized(LEXY_LIT("..."));
        auto body      = dsl::curly_bracketed(LEXY_LIT("..."));

        // The position of a function is the first character of the name.
        return kw_function >> dsl::position + id + arguments + body;
    }();

    static constexpr auto value = lexy::construct<function>;
};
//}

struct production
{
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule  = dsl::list(dsl::p<function_decl>);
    static constexpr auto value = lexy::as_list<std::vector<function>>;
};

int main()
{
    auto input  = lexy_ext::compiler_explorer_input();
    auto result = lexy::parse<production>(input, lexy_ext::report_error);
    if (!result)
        return 1;

    for (auto fn : result.value())
    {
        // We can use `lexy::get_input_location` to turn a position into a proper location.
        auto location = lexy::get_input_location(input, fn.position);
        std::printf("The function '%.*s' is at offset %zu, which is line %u and column %u\n",
                    (int)fn.name.size(), (const char*)fn.name.data(), fn.position - input.data(),
                    location.line_nr(), location.column_nr());
    }
}

