// INPUT:abc
struct production
{
    static constexpr auto rule = LEXY_LIT("a") / LEXY_LIT("abc") / LEXY_LIT("bc");
};
