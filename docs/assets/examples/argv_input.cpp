#include <cstdio>

#include <lexy/dsl.hpp>
#include <lexy/input/argv_input.hpp>
#include <lexy/match.hpp>

struct production
{
    static constexpr auto rule = LEXY_LIT("Hi");
};

//{
int main(int argc, char* argv[])
{
    // Create the input.
    lexy::argv_input input(argc, argv);

    // Use the input.
    if (!lexy::match<production>(input))
    {
        std::puts("Error!\n");
        return 1;
    }
}
//}

