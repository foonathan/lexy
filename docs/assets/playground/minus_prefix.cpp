// INPUT:nobody
struct production
{
    static constexpr auto rule = [] {
        auto word = dsl::while_one(dsl::ascii::alnum);
        // Without dsl::prefix, only 'no' is forbidden.
        auto forbidden = dsl::prefix(LEXY_LIT("no"));
        return dsl::token(word) - forbidden;
    }();
};

