//=== main function ===//
#include <lexy/action/parse_as_tree.hpp>
#include <lexy/input/file.hpp>
#include <lexy/visualize.hpp>
#include <lexy_ext/report_error.hpp>

int main()
{
    auto input = lexy::read_stdin<lexy::utf8_encoding>().buffer();

    lexy::parse_tree_for<decltype(input)> tree;
    auto                                  result
        = lexy::parse_as_tree<LEXY_PLAYGROUND_PRODUCTION>(tree, input, lexy_ext::report_error);

    lexy::visualize(stdout, tree, {lexy::visualize_fancy});

    if (!result)
        return 1;
}

