// Copyright (C) 2020 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <cstdio>
#include <string>
#include <vector>

#include <lexy/dsl.hpp>        // lexy::dsl::*
#include <lexy/input/file.hpp> // lexy::read_file
#include <lexy/parse.hpp>      // lexy::parse

#include "report_error.hpp" // lexy_ex::report_error

// Parses an email as defined by a simplified subset of RFC 5322
// (https://tools.ietf.org/html/rfc5322).
// It does not support groups or IPs as email addresses, nor most
// headers or the line width limitations.

namespace ast
{
struct address
{
    std::string display_name;
    std::string local_part;
    std::string domain;

    void print() const
    {
        std::printf("%s <%s@%s> ", display_name.c_str(), local_part.c_str(), domain.c_str());
    }
};

struct message
{
    std::vector<address> from;
    std::vector<address> to;
    std::vector<address> cc;
    std::string          subject;
    std::string          body;

    void print() const
    {
        std::fputs("From: ", stdout);
        for (auto& addr : from)
            addr.print();
        std::putchar('\n');
        std::fputs("To: ", stdout);
        for (auto& addr : to)
            addr.print();
        std::putchar('\n');
        std::fputs("Cc: ", stdout);
        for (auto& addr : cc)
            addr.print();
        std::putchar('\n');
        std::printf("Subject:%s\n", subject.c_str());

        std::putchar('\n');

        std::printf("%s", body.c_str());
    }
};

} // namespace ast
namespace grammar
{
namespace dsl = lexy::dsl;

// We're limiting whitespace to ASCII blank or comments.
// We don't distinguish here between folding and non-folding whitespace for simplicity.
constexpr auto ws = dsl::ascii::blank | dsl::parenthesized(dsl::ascii::print);

//=== https://tools.ietf.org/html/rfc5322#section-3.2.3 ===//
constexpr auto atext = dsl::ascii::alpha / dsl::ascii::digit / LEXY_LIT("!") / LEXY_LIT("#")
                       / LEXY_LIT("$") / LEXY_LIT("%") / LEXY_LIT("&") / LEXY_LIT("'")
                       / LEXY_LIT("*") / LEXY_LIT("+") / LEXY_LIT("-") / LEXY_LIT("/")
                       / LEXY_LIT("=") / LEXY_LIT("?") / LEXY_LIT("^") / LEXY_LIT("_")
                       / LEXY_LIT("`") / LEXY_LIT("{") / LEXY_LIT("|") / LEXY_LIT("}");

// Text of the specified characters surrounded by whitespace.
constexpr auto atom = dsl::while_(ws) + dsl::capture(dsl::while_one(atext)) + dsl::while_(ws);

struct dot_atom
{
    // A list of atom separated by periods which are part of the content and thus captured (unlike
    // the whitespace).
    static constexpr auto rule = dsl::list(atom, dsl::sep(dsl::capture(dsl::period)));
    static constexpr auto list = lexy::as_string<std::string>;
};

//=== https://tools.ietf.org/html/rfc5322#section-3.2.4 ===//
struct quoted_string
{
    static constexpr auto rule = [] {
        // \c, where c is a printable ASCII character, is replaced by c itself.
        auto escape = dsl::backslash_escape.capture(dsl::ascii::print);
        return dsl::quoted[ws](dsl::ascii::print, escape);
    }();
    static constexpr auto list = lexy::as_string<std::string>;
};

//=== https://tools.ietf.org/html/rfc5322#section-3.2.5 ===//
constexpr auto word = dsl::p<quoted_string> | dsl::else_ >> atom;

struct phrase
{
    // A phrase is a list of words which starts with another text character or quote.
    static constexpr auto rule = dsl::list(dsl::peek(atext / LEXY_LIT("\""))[ws] >> word);
    static constexpr auto list = lexy::as_string<std::string>;
};

//=== https://tools.ietf.org/html/rfc5322#section-3.4 ===//
// Note: we're having no group so we merge mailbox and address.
struct address
{
    static constexpr auto rule = [] {
        auto local_part = dsl::p<quoted_string> | dsl::else_ >> dsl::p<dot_atom>;
        auto domain     = dsl::p<dot_atom>; // No domain literal support.

        // Sets the local part and domain member.
        auto addr_spec = local_part + dsl::at_sign + domain;

        // An address spec with an optional name.
        auto angle_addr = dsl::angle_bracketed[ws](addr_spec);
        auto name_addr  = dsl::opt(dsl::p<phrase>) + angle_addr;

        // We're only having a named address if we can match the optional phrase followed by an
        // angle bracket.
        auto name_addr_condition = dsl::opt(dsl::p<phrase>) + LEXY_LIT("<")[ws];

        // An address spec without a name.
        auto unnamed_addr = dsl::nullopt + addr_spec;

        return dsl::peek(name_addr_condition) >> name_addr | dsl::else_ >> unnamed_addr;
    }();
    static constexpr auto value = lexy::construct<ast::address>;
};

struct address_list
{
    static constexpr auto rule = dsl::list(dsl::p<address>, dsl::sep(dsl::comma));
    static constexpr auto list = lexy::as_list<std::vector<ast::address>>;
};

//=== https://tools.ietf.org/html/rfc5322#section-3.5/6 ===//
struct unstructured
{
    static constexpr auto rule  = dsl::capture(dsl::while_(dsl::ascii::print));
    static constexpr auto value = lexy::as_string<std::string>;
};

struct body
{
    // The body is literally anything until the end of the file.
    static constexpr auto rule  = dsl::capture(dsl::any);
    static constexpr auto value = lexy::as_string<std::string>;
};

struct fields
{
    static constexpr auto rule = [] {
        // Some of the fields in the header of a message.
        auto from    = LEXY_LIT("From:") >> dsl::p<address_list> + dsl::newline;
        auto to      = LEXY_LIT("To:") >> dsl::p<address_list> + dsl::newline;
        auto cc      = LEXY_LIT("Cc:") >> dsl::p<address_list> + dsl::newline;
        auto subject = LEXY_LIT("Subject:") >> dsl::p<unstructured> + dsl::newline;

        // We allow a partial combination of the fields (i.e. each field at most once in an
        // arbitrary order). Some fields are mandatory, but this verification is better done
        // elsewhere.
        //
        // With LEXY_MEM() we can assign them to a member of the given name.
        return dsl::partial_combination(LEXY_MEM(from) = from, LEXY_MEM(to) = to, LEXY_MEM(cc) = cc,
                                        LEXY_MEM(subject) = subject);
    }();
    static constexpr auto list = lexy::as_aggregate<ast::message>;
};

struct message
{
    // The fields followed by the body assigned to the body member.
    static constexpr auto rule  = dsl::p<fields> + dsl::newline + (LEXY_MEM(body) = dsl::p<body>);
    static constexpr auto value = lexy::as_aggregate<ast::message>;
};
} // namespace grammar

#ifndef LEXY_TEST
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: %s <filename>", argv[0]);
        return 1;
    }

    // Emails only support ASCII characters.
    auto file = lexy::read_file<lexy::ascii_encoding>(argv[1]);
    if (!file)
    {
        std::fprintf(stderr, "file '%s' not found", argv[1]);
        return 1;
    }

    auto& input   = file.value();
    auto  message = lexy::parse<grammar::message>(input, lexy_ex::report_error);
    if (!message)
        return 2;

    message.value().print();
}
#endif

