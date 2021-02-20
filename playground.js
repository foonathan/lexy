const api = "https://godbolt.org/api/";
const compiler_id = "clang_trunk";
const lexy_id = { id: 'lexy', version: 'trunk' };

export function list_of_productions(source)
{
    var result = [];

    const regex = /(struct|class) ([a-zA-Z0-9_]+)/g;
    var match = undefined;
    while (match = regex.exec(source))
        result.push(match[2]);

    return result;
}

export function preprocess_source(target, source, production)
{
    
    
    
    

    if (target == 'playground')
    {
        const macros = `#define LEXY_PLAYGROUND_PRODUCTION ${production}`
        const prefix = String.raw`#include <lexy/dsl.hpp>
namespace dsl = lexy::dsl;
#line 0 "grammar.cpp"
`;
        const main = String.raw`#line 1 "playground.cpp"
#include <cctype>
#include <lexy/parse_tree.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/report_error.hpp>

template <typename Reader>
void* get_node_id(const lexy::parse_tree<Reader>&         tree,
                  typename lexy::parse_tree<Reader>::node node)
{
    using ptr_t = lexy::_detail::pt_node_ptr<Reader>;
    // It's okay to reinterpret standard layout type to first member.
    return reinterpret_cast<ptr_t*>(&node)->base();
}

int main()
{
    auto input = lexy_ext::read_file<lexy::utf8_encoding>(stdin).value();

    lexy::parse_tree_for<decltype(input)> tree;
    auto                                  result
        = lexy::parse_as_tree<LEXY_PLAYGROUND_PRODUCTION>(tree, input, lexy_ext::report_error);
    if (!result)
        return 1;

    std::puts("graph \"Parse Tree\" {");
    for (auto [event, node] : tree.traverse())
    {
        switch (event)
        {
        case lexy::traverse_event::enter:
            std::printf("\"node-%p\" [label=\"%.*s\"];\n", get_node_id(tree, node),
                        int(node.kind().name().size()), node.kind().name().data());
            break;

        case lexy::traverse_event::exit:
            // Now we can add all the connections.
            for (auto child : node.children())
                std::printf("\"node-%p\" -- \"node-%p\";\n", get_node_id(tree, node),
                            get_node_id(tree, child));
            break;

        case lexy::traverse_event::leaf:
            std::printf("\"node-%p\" [label=\"", get_node_id(tree, node));
            for (auto c : node.lexeme())
            {
                if (c == '"')
                    std::fputs(R"(\")", stdout);
                else if (c == ' ')
                    std::fputs("␣", stdout);
                else if (c == '\n')
                    std::fputs("⏎", stdout);
                else if (std::iscntrl(c))
                    std::printf("0x%02X", unsigned(c) & 0xFF);
                else
                    std::putchar(c);
            }
            std::puts("\", shape=box];");
            break;
        }
    }
    std::puts("}");
}

`;

        return macros + '\n' + prefix + source + '\n' + main;
    }
    else
    {
        const macros = `#define LEXY_PLAYGROUND_PRODUCTION ${production}`;
        const prefix = String.raw`#include <lexy/dsl.hpp>

namespace dsl = lexy::dsl;

//=== grammar ===//
`;
        const main = String.raw`//=== main function ===//
#include <lexy/parse_tree.hpp>
#include <lexy_ext/cfile.hpp>
#include <lexy_ext/parse_tree_dump.hpp>
#include <lexy_ext/report_error.hpp>

int main()
{
    auto input = lexy_ext::read_file<lexy::utf8_encoding>(stdin).value();

    lexy::parse_tree_for<decltype(input)> tree;
    auto                                  result
        = lexy::parse_as_tree<LEXY_PLAYGROUND_PRODUCTION>(tree, input, lexy_ext::report_error);
    if (!result)
        return 1;

    lexy_ext::dump_parse_tree(stdout, tree);
}

`;

        return macros + '\n' + prefix + source + '\n' + main;
    }
}

export async function compile_and_run(source, input)
{
    var body = {};
    body.source = source;

    body.options = {};
    body.options.userArguments = "-fno-color-diagnostics -std=c++20";
    body.options.executeParameters = { args: [], stdin: input };
    body.options.compilerOptions = { executorRequest: true };
    body.options.filters = { execute: true };
    body.options.tools = [];
    body.options.libraries = [ lexy_id ];

    body.lang = "c++";

    const response = await fetch(api + "compiler/" + compiler_id + "/compile", {
        method: "POST",
        headers: { 'Content-Type': 'application/json', 'Accept': 'application/json' },
        body: JSON.stringify(body)
    });
    const result = await response.json();

    if (result.didExecute)
    {
        var stdout = result.stdout.map(x => x.text).join("\n");
        var stderr = result.stderr.map(x => x.text).join("\n");
        return { success: true, stdout: stdout, stderr: stderr, code: result.code };
    }
    else
    {
        var message = result.buildResult.stderr.map(x => x.text).join("\n");
        return { success: false, message: message };
    }
}

export async function get_godbolt_url(source, input)
{
    var session = {};
    session.id = 1;
    session.language = "c++";
    session.source = source;
    session.compilers = [];

    var compiler = {};
    compiler.id = compiler_id;
    compiler.libs = [ lexy_id ];
    compiler.options = "-std=c++20";
    session.executors = [{ compiler: compiler, stdin: input }];

    const response = await fetch(api + "shortener", {
        method: "POST",
        headers: { "Content-Type": "application/json", "Accept": "application/json" },
        body: JSON.stringify({ sessions: [session] })
    });
    return (await response.json()).url;
}

export async function load_godbolt_url(id)
{
    const response = await fetch(api + "shortlinkinfo/" + id);
    const result = await response.json();

    const session = result.sessions[0];
    const source = session.source;
    const input = session.executors[0].stdin;

    const production_regex = /#define LEXY_PLAYGROUND_PRODUCTION ([a-zA-Z_0-9]+)/;
    const production = production_regex.exec(source)[1];

    const grammar_regex = /\/\/=== grammar ===\/\/([^]*)\/\/=== main function ===\/\//;
    let grammar = grammar_regex.exec(source)[1];
    grammar = grammar.trim();

    return { grammar: grammar, input: input, production: production };
}

