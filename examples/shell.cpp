// Copyright (C) 2020-2021 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <map>
#include <memory>
#include <string>

#include <lexy/dsl.hpp>
#include <lexy/input/shell.hpp>
#include <lexy/parse.hpp>

#include "report_error.hpp"

// A shell with a couple of basic commands.
namespace shell
{
struct interpreter
{
    // Manages I/O.
    lexy::shell<lexy::default_prompt<lexy::utf8_encoding>> shell;
    // The values of variables.
    std::map<std::string, std::string> variables;

    // Retrieves the value of a variable.
    const std::string& lookup_var(const std::string& name) const
    {
        auto iter = variables.find(name);
        if (iter == variables.end())
        {
            static const std::string not_found;
            return not_found;
        }

        return iter->second;
    }
};

class cmd_base
{
public:
    virtual ~cmd_base() = default;

    // Returns true if the shell should exit.
    virtual bool execute(interpreter& intp) const = 0;
};
using command = std::unique_ptr<cmd_base>;

// Exits the shell.
class cmd_exit : public cmd_base
{
public:
    bool execute(interpreter& intp) const override
    {
        intp.shell.write_message()("Goodbye.");
        return true;
    }
};

// Prints output.
class cmd_echo : public cmd_base
{
public:
    explicit cmd_echo(std::string msg) : _msg(LEXY_MOV(msg)) {}

    bool execute(interpreter& intp) const override
    {
        intp.shell.write_message()(_msg.data(), _msg.size());
        return false;
    }

private:
    std::string _msg;
};

// Sets the value of a variable.
class cmd_set : public cmd_base
{
public:
    explicit cmd_set(std::string name, std::string value)
    : _name(LEXY_MOV(name)), _value(LEXY_MOV(value))
    {}

    bool execute(interpreter& intp) const override
    {
        intp.variables[_name] = _value;
        return false;
    }

private:
    std::string _name;
    std::string _value;
};
} // namespace shell

namespace grammar
{
namespace dsl = lexy::dsl;
struct argument;

// Arguments are separated by ' ' or '\t' or by a backslash followed by newline.
constexpr auto arg_sep = dsl::while_one(dsl::ascii::blank | dsl::backslash + dsl::newline);

// Characters allowed in a bare argument or command name.
constexpr auto bare_char = dsl::ascii::alnum;
// The content of a string literal: any unicode code point except for control characters.
constexpr auto str_char = dsl::code_point - dsl::ascii::control;

// An unquoted sequence of characters.
struct arg_bare
{
    static constexpr auto rule  = dsl::capture(dsl::while_(bare_char));
    static constexpr auto value = lexy::as_string<std::string>;
};

// A string without escape characters.
struct arg_string
{
    static constexpr auto rule  = dsl::single_quoted(str_char);
    static constexpr auto value = lexy::as_string<std::string>;
};

// A string with escape characters.
struct arg_quoted
{
    struct interpolation
    {
        static constexpr auto rule
            = dsl::curly_bracketed(dsl::parse_state + dsl::recurse<argument>);
        static constexpr auto value = lexy::callback<std::string>(&shell::interpreter::lookup_var);
    };

    static constexpr auto rule = [] {
        auto escape = dsl::backslash_escape //
                          .lit_c<'"'>()
                          .lit_c<'\\'>()
                          .lit_c<'/'>()
                          .lit_c<'b'>(dsl::value_c<'\b'>)
                          .lit_c<'f'>(dsl::value_c<'\f'>)
                          .lit_c<'n'>(dsl::value_c<'\n'>)
                          .lit_c<'r'>(dsl::value_c<'\r'>)
                          .lit_c<'t'>(dsl::value_c<'\t'>)
                          .rule(dsl::p<interpolation>);

        return dsl::quoted(str_char, escape);
    }();
    static constexpr auto value = lexy::as_string<std::string>;
};

// An argument that expands to the value of a variable.
struct arg_var
{
    static constexpr auto rule = [] {
        auto bare      = dsl::p<arg_bare>;
        auto bracketed = dsl::curly_bracketed(dsl::recurse<argument>);
        auto name      = bracketed | dsl::else_ >> bare;

        return dsl::dollar_sign >> dsl::parse_state + name;
    }();
    static constexpr auto value = lexy::callback<std::string>(&shell::interpreter::lookup_var);
};

// An argument to a command.
struct argument
{
    static constexpr auto rule = dsl::p<arg_string> | dsl::p<arg_quoted> //
                                 | dsl::p<arg_var> | dsl::else_ >> dsl::p<arg_bare>;
    static constexpr auto value = lexy::forward<std::string>;
};

struct cmd_exit
{
    static constexpr auto rule  = LEXY_LIT("exit") / dsl::eof;
    static constexpr auto value = lexy::new_<shell::cmd_exit, shell::command>;
};

struct cmd_echo
{
    static constexpr auto rule  = LEXY_LIT("echo") >> arg_sep + dsl::p<argument>;
    static constexpr auto value = lexy::new_<shell::cmd_echo, shell::command>;
};

struct cmd_set
{
    static constexpr auto rule
        = LEXY_LIT("set") >> arg_sep + dsl::p<argument> + arg_sep + dsl::p<argument>;
    static constexpr auto value = lexy::new_<shell::cmd_set, shell::command>;
};

// Parses one of three commands.
struct command
{
    struct unknown_command
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "unknown command";
        }
    };
    struct trailing_args
    {
        static LEXY_CONSTEVAL auto name()
        {
            return "trailing command arguments";
        }
    };

    static constexpr auto rule = [] {
        auto unknown  = dsl::error<unknown_command>(dsl::while_(bare_char));
        auto commands = dsl::p<cmd_exit> | dsl::p<cmd_echo> //
                        | dsl::p<cmd_set> | dsl::else_ >> unknown;

        return commands + dsl::eol[arg_sep].error<trailing_args>();
    }();
    static constexpr auto value = lexy::forward<shell::command>;
};
} // namespace grammar

#ifndef LEXY_TEST
int main()
{
    for (shell::interpreter intp; intp.shell.is_open();)
    {
        // We repeatedly prompt for a new line.
        // Note: everytime we do it, the memory of the previous line is cleared.
        auto input = intp.shell.prompt_for_input();

        // Then we parse the command.
        auto result = lexy::parse<grammar::command>(input, intp, lexy_ex::report_error);
        if (!result)
            continue;

        // ... and execute it.
        auto exit = result.value()->execute(intp);
        if (exit)
            break;
    }
}
#endif

