// INPUT:// This is a comment.
struct production
{
    // Note that `dsl::eol` matches EOF as well, so `.or_eof()` is implied.
    static constexpr auto rule = LEXY_LIT("//") + dsl::until(dsl::eol);
};
