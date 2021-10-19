// INPUT:🙂
struct production
{
    static constexpr auto rule = dsl::code_point.lit<0x1F642>() + dsl::eof;
};
