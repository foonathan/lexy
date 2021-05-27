// INPUT:nobody
struct production
{
    static constexpr auto rule = [] {
        auto word = dsl::while_one(dsl::ascii::alnum);
        // Without dsl::any, only 'no' is forbidden.
        auto forbidden = LEXY_LIT("no") + dsl::any;
        return dsl::token(word) - dsl::token(forbidden);
    }();
};

