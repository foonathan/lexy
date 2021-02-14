const api = "https://godbolt.org/api/";
const compiler = "clang_trunk";

export function list_of_productions(source)
{
    var result = [];

    const regex = /(struct|class) ([a-zA-Z0-9_]+)/g;
    var match = undefined;
    while (match = regex.exec(source))
        result.push(match[2]);

    return result;
}

export function preprocess_source(source, production)
{
    {{ $source_prefix := resources.Get "cpp/playground_prefix.cpp" }}
    {{ $source_main := resources.Get "cpp/playground_main.cpp" }}

    const macros = `#define LEXY_PLAYGROUND_PRODUCTION ${production}`
    return [macros, String.raw`{{ $source_prefix.Content }}`, source, String.raw`{{ $source_main.Content }}` ].join("\n");
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
    body.options.libraries = [ { id: 'lexy', version: 'trunk' } ];

    body.lang = "c++";

    const response = await fetch(api + "compiler/" + compiler + "/compile", {
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

