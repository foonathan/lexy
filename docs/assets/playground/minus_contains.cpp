// INPUT:annotate
struct production
{
    static constexpr auto rule = [] {
        auto word = dsl::while_one(dsl::ascii::alnum);
        // Without dsl::contains, only 'no' is forbidden.
        auto forbidden = dsl::contains(LEXY_LIT("no"));
        return dsl::token(word) - forbidden;
    }();
};

