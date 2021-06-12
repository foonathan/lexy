// INPUT:Hello foo*!
struct name
{
    struct invalid_character
    {
        static constexpr auto name = "invalid character";
    };

    static constexpr auto rule = [] {
        // One or more alpha numeric characters, underscores or hyphens.
        auto name = dsl::while_one(dsl::ascii::alnum / dsl::lit_c<'_'> / dsl::lit_c<'-'>);

        // We know that a name is always followed by an `!`, so ensure it.
        // But note that it is not our responsibility to parse it.
        return name + dsl::require(dsl::exclamation_mark).error<invalid_character>;
    }();
};

struct production
{
    // Allow arbitrary spaces between individual tokens.
    // Note that this includes the individual characters of the name.
    static constexpr auto whitespace = dsl::ascii::space;

    static constexpr auto rule = [] {
        auto greeting = LEXY_LIT("Hello");
        return greeting + dsl::p<name> + dsl::exclamation_mark + dsl::eof;
    }();
};
