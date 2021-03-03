//=== main function ===//
#include <lexy/parse_tree.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/parse_tree_dump.hpp>
#include <lexy_ext/report_error.hpp>

int main()
{
    auto input = lexy_ext::read_file<lexy::utf8_encoding>(stdin);

    lexy::parse_tree_for<decltype(input)> tree;
    auto                                  result
        = lexy::parse_as_tree<LEXY_PLAYGROUND_PRODUCTION>(tree, input, lexy_ext::report_error);
    if (!result)
        return 1;

    lexy_ext::dump_parse_tree(stdout, tree);
}

