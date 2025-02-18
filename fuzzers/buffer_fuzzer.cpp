#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#define LEXY_TEST
#include <lexy/_detail/buffer_builder.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    if (Size < 4)
        return 0;

    FuzzedDataProvider fuzz_data(Data, Size);

    // Choose buffer type
    auto type = fuzz_data.ConsumeIntegralInRange<int>(0, 2);

    try
    {
        switch (type)
        {
        case 0: {
            // char buffer - test basic operations
            lexy::_detail::buffer_builder<char> buffer;

            while (fuzz_data.remaining_bytes() > 0)
            {
                switch (fuzz_data.ConsumeIntegralInRange<int>(0, 3))
                {
                case 0: { // Write and commit
                    const auto write_size = buffer.write_size();
                    if (write_size > 0 && fuzz_data.remaining_bytes() > 0)
                    {
                        // Ensure max is at least 1
                        const auto max_size
                            = std::max(std::min(write_size, fuzz_data.remaining_bytes()),
                                       size_t(1));
                        const auto n = fuzz_data.ConsumeIntegralInRange<size_t>(1, max_size);

                        auto bytes = fuzz_data.ConsumeBytes<uint8_t>(n);
                        if (bytes.size() == n)
                        {
                            std::memcpy(buffer.write_data(), bytes.data(), n);
                            buffer.commit(n);
                        }
                    }
                    break;
                }
                case 1: // Clear
                    buffer.clear();
                    break;
                case 2: // Grow
                    buffer.grow();
                    break;
                case 3: { // Read using iterator
                    if (buffer.read_size() > 0)
                    {
                        using iterator = typename decltype(buffer)::stable_iterator;
                        iterator it(buffer, 0);
                        iterator end(buffer, buffer.read_size());
                        while (it != end)
                        {
                            volatile auto ch = *it;
                            (void)ch;
                            ++it;
                        }
                    }
                    break;
                }
                }
            }
            break;
        }
        case 1: {
            // int buffer - test element alignment
            lexy::_detail::buffer_builder<int> buffer;
            while (fuzz_data.remaining_bytes() >= sizeof(int))
            {
                if (buffer.write_size() > 0)
                {
                    int value = fuzz_data.ConsumeIntegral<int>();
                    std::memcpy(buffer.write_data(), &value, sizeof(int));
                    buffer.commit(1);
                }
                else
                {
                    buffer.grow();
                }
            }
            break;
        }
        case 2: {
            // double buffer - test larger alignment
            lexy::_detail::buffer_builder<double> buffer;
            while (fuzz_data.remaining_bytes() >= sizeof(double))
            {
                if (buffer.write_size() > 0)
                {
                    std::vector<uint8_t> bytes = fuzz_data.ConsumeBytes<uint8_t>(sizeof(double));
                    if (bytes.size() == sizeof(double))
                    {
                        std::memcpy(buffer.write_data(), bytes.data(), sizeof(double));
                        buffer.commit(1);
                    }
                }
                else
                {
                    buffer.grow();
                }
            }
            break;
        }
        }
    }
    catch (...)
    {}

    return 0;
}
