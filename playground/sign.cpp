// INPUT:+123
struct production
{
    static constexpr auto rule
        // Sign followed by a decimal integer.
        = dsl::sign + dsl::integer<int>(dsl::digits<>);
};
