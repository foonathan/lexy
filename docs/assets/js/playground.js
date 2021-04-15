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

export async function preprocess_source(target, source, production)
{
    {{ $playground_headers := resources.Get "cpp/playground_headers.single.hpp" }}
    {{ $playground_prefix  := resources.Get "cpp/playground_prefix.cpp" }}
    {{ $playground_main    := resources.Get "cpp/playground_main.cpp" }}
    {{ $godbolt_prefix     := resources.Get "cpp/godbolt_prefix.cpp" }}
    {{ $godbolt_main       := resources.Get "cpp/godbolt_main.cpp" }}

    if (target == 'playground')
    {
        {{ if hugo.IsProduction }}
            const header = `#include "{{ $playground_headers.Permalink }}"`;
        {{ else }}
            const header = await (await fetch('{{ $playground_headers.Permalink }}')).text();
        {{ end }}
        const macros = `#define LEXY_PLAYGROUND_PRODUCTION ${production}`;
        const prefix = await (await fetch('{{ $playground_prefix.Permalink }}')).text();
        const main = await (await fetch('{{ $playground_main.Permalink }}')).text();

        return header + '\n' + macros + '\n' + prefix + '\n' + source + '\n' + main;
    }
    else
    {
        const macros = `#define LEXY_PLAYGROUND_PRODUCTION ${production}`;
        const prefix = await (await fetch('{{ $godbolt_prefix.Permalink }}')).text();
        const main = await (await fetch('{{ $godbolt_main.Permalink }}')).text();

        return macros + '\n' + prefix + '\n' + source + '\n' + main;
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

