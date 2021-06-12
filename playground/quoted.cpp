// INPUT:"Hello World!"
struct production
{
    static constexpr auto rule = [] {
        // Arbitrary code points that aren't control characters.
        auto c = dsl::code_point - dsl::ascii::control;

        return dsl::quoted(c);
    }();
};
