#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <limits>
#include <string>
#include <vector>

// Define this to prevent the original main from being included
#define LEXY_TEST

// Include the original json implementation
#include "json.cpp"

#undef LEXY_TEST

// Helper function to generate JSON number strings
std::string generateNumber(FuzzedDataProvider& fuzz_data)
{
    std::string num;
    if (fuzz_data.ConsumeBool())
    {
        num += "-";
    }

    // Generate integer part
    if (fuzz_data.ConsumeBool())
    {
        num += std::to_string(fuzz_data.ConsumeIntegral<int64_t>());
    }
    else
    {
        num += std::to_string(fuzz_data.ConsumeIntegralInRange<int>(0, 9));
    }

    // Maybe add fraction
    if (fuzz_data.ConsumeBool())
    {
        num += ".";
        num += std::to_string(fuzz_data.ConsumeIntegralInRange<unsigned>(0, 999999));
    }

    // Maybe add exponent
    if (fuzz_data.ConsumeBool())
    {
        num += fuzz_data.ConsumeBool() ? "e" : "E";
        num += fuzz_data.ConsumeBool() ? "+" : "-";
        num += std::to_string(fuzz_data.ConsumeIntegralInRange<int16_t>(0, 999));
    }

    return num;
}

// Helper function to generate JSON strings with escapes
std::string generateString(FuzzedDataProvider& fuzz_data)
{
    std::string str    = "\"";
    size_t      length = fuzz_data.ConsumeIntegralInRange<size_t>(0, 32);

    for (size_t i = 0; i < length; ++i)
    {
        if (fuzz_data.ConsumeBool())
        {
            // Generate escape sequence
            switch (fuzz_data.ConsumeIntegralInRange<int>(0, 7))
            {
            case 0:
                str += "\\\"";
                break;
            case 1:
                str += "\\\\";
                break;
            case 2:
                str += "\\n";
                break;
            case 3:
                str += "\\r";
                break;
            case 4:
                str += "\\t";
                break;
            case 5:
                str += "\\b";
                break;
            case 6:
                str += "\\f";
                break;
            case 7:
                // Unicode escape
                char unicode[7];
                std::snprintf(unicode, sizeof(unicode), "\\u%04x",
                              fuzz_data.ConsumeIntegralInRange<unsigned>(0, 0xFFFF));
                str += unicode;
                break;
            }
        }
        else
        {
            // Regular printable character
            char c = fuzz_data.ConsumeIntegralInRange<char>(32, 126);
            str += c;
        }
    }

    str += "\"";
    return str;
}

// Helper function to generate nested JSON structures
std::string generateJSON(FuzzedDataProvider& fuzz_data, int depth = 0)
{
    if (depth > 5 || fuzz_data.remaining_bytes() < 2)
    {
        // Generate primitive values at max depth or low remaining bytes
        switch (fuzz_data.ConsumeIntegralInRange<int>(0, 4))
        {
        case 0:
            return "null";
        case 1:
            return fuzz_data.ConsumeBool() ? "true" : "false";
        case 2:
            return generateNumber(fuzz_data);
        case 3:
            return generateString(fuzz_data);
        default:
            return "null";
        }
    }

    // Generate complex structures
    switch (fuzz_data.ConsumeIntegralInRange<int>(0, 6))
    {
    case 0: // Array
    {
        std::string arr      = "[";
        int         elements = fuzz_data.ConsumeIntegralInRange<int>(0, 5);
        for (int i = 0; i < elements; ++i)
        {
            if (i > 0)
                arr += ",";
            arr += generateJSON(fuzz_data, depth + 1);
        }
        arr += "]";
        return arr;
    }

    case 1: // Object
    {
        std::string obj   = "{";
        int         pairs = fuzz_data.ConsumeIntegralInRange<int>(0, 5);
        for (int i = 0; i < pairs; ++i)
        {
            if (i > 0)
                obj += ",";
            obj += generateString(fuzz_data) + ":" + generateJSON(fuzz_data, depth + 1);
        }
        obj += "}";
        return obj;
    }

    default: // Primitives with higher probability
        switch (fuzz_data.ConsumeIntegralInRange<int>(0, 3))
        {
        case 0:
            return "null";
        case 1:
            return fuzz_data.ConsumeBool() ? "true" : "false";
        case 2:
            return generateNumber(fuzz_data);
        case 3:
            return generateString(fuzz_data);
        }
    }

    return "null";
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    if (Size < 4)
        return 0; // Require minimum input size

    FuzzedDataProvider fuzz_data(Data, Size);

    // Generate structured JSON with probability
    std::string input;
    if (fuzz_data.ConsumeBool())
    {
        // Generate well-formed JSON
        input = generateJSON(fuzz_data);
    }
    else
    {
        // Generate potentially malformed JSON for edge cases
        static const char structural[] = "{}[],:\"";
        while (fuzz_data.remaining_bytes() > 0)
        {
            switch (fuzz_data.ConsumeIntegralInRange<int>(0, 4))
            {
            case 0: { // Structural characters
                input += structural[fuzz_data.ConsumeIntegralInRange<size_t>(0, 5)];
                break;
            }

            case 1: { // Numbers
                input += generateNumber(fuzz_data);
                break;
            }

            case 2: { // Strings
                input += generateString(fuzz_data);
                break;
            }

            case 3: { // Keywords
                switch (fuzz_data.ConsumeIntegralInRange<int>(0, 2))
                {
                case 0:
                    input += "null";
                    break;
                case 1:
                    input += "true";
                    break;
                case 2:
                    input += "false";
                    break;
                }
                break;
            }

            case 4: { // Random data
                input += fuzz_data.ConsumeRandomLengthString(16);
                break;
            }
            }
        }
    }

    // Create a buffer from the generated input
    auto buffer = lexy::buffer<lexy::utf8_encoding>(input.data(), input.size());

    // Try to parse the JSON
    auto result = lexy::parse<grammar::json>(buffer, lexy_ext::report_error);

    if (result.has_value())
    {
        // For valid JSON, also test the printing functionality
        result.value().print();
    }

    return 0;
}
