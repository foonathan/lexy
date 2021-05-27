// INPUT:"Hello World!\n
struct production
{
    static constexpr auto rule = [] {
        // Arbitrary code points that aren't control characters.
        auto c = dsl::code_point - dsl::ascii::control;

        // If we have an EOL inside our string, we're missing the closing ".
        auto quoted = dsl::quoted.limit(dsl::eol);
        return quoted(c);
    }();
};
