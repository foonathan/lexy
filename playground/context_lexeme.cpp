// INPUT:Hello,Hallo
struct word
{
    static constexpr auto rule = dsl::identifier(dsl::ascii::alpha);
};

struct production
{
    struct mismatch
    {};

    static constexpr auto rule = [] {
        // Declare a string variable - it is not created yet!
        auto word_var = dsl::context_lexeme<production>;

        // Parse a word and capture it in the variable.
        auto first_word = word_var.capture(dsl::p<word>);
        // Parse another word and compare it agains the variable.
        auto second_word = word_var.require(dsl::p<word>).error<mismatch>;

        // Create the empty variable, then parse the two words.
        return word_var.create() + first_word + dsl::lit_c<','> + second_word;
    }();
};
