struct name
{
    static constexpr auto rule = dsl::while_(dsl::ascii::alnum / dsl::lit_c<'_'> / dsl::lit_c<'-'>);
};

struct production
{
    static constexpr auto rule = [] {
        auto greeting = LEXY_LIT("Hello");
        return greeting + dsl::ascii::space + dsl::p<name> + dsl::lit_c<'!'> + dsl::eof;
    }();
};
