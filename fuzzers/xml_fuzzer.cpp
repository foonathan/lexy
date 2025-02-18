#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

// Define this to prevent the original main from being included
#define LEXY_TEST

// Include the original XML implementation
#include "xml.cpp"

#undef LEXY_TEST

// Helper function to generate XML tag names
std::string generateTagName(FuzzedDataProvider& fuzz_data)
{
    static const char first_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_:";
    static const char other_chars[]
        = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_:-.";

    std::string name;
    // First character has more restrictions
    name += first_chars[fuzz_data.ConsumeIntegralInRange<size_t>(0, sizeof(first_chars) - 2)];

    // Rest of the name
    size_t len = fuzz_data.ConsumeIntegralInRange<size_t>(0, 10);
    for (size_t i = 0; i < len; ++i)
    {
        name += other_chars[fuzz_data.ConsumeIntegralInRange<size_t>(0, sizeof(other_chars) - 2)];
    }
    return name;
}

// Helper function to generate entity references
std::string generateEntityRef(FuzzedDataProvider& fuzz_data)
{
    static const char* entities[] = {"&quot;", "&amp;", "&apos;", "&lt;", "&gt;",
                                     // Add some invalid ones for testing
                                     "&invalid;", "&test", "&;", "&&;", "&quot"};
    return entities[fuzz_data.ConsumeIntegralInRange<size_t>(0,
                                                             sizeof(entities) / sizeof(entities[0])
                                                                 - 1)];
}

// Helper function to generate CDATA sections
std::string generateCDATA(FuzzedDataProvider& fuzz_data)
{
    std::string cdata = "<![CDATA[";

    size_t len = fuzz_data.ConsumeIntegralInRange<size_t>(0, 32);
    for (size_t i = 0; i < len; ++i)
    {
        switch (fuzz_data.ConsumeIntegralInRange<int>(0, 4))
        {
        case 0:
            cdata += "]]"; // Almost closing
            break;
        case 1:
            cdata += "]>"; // Partial closing
            break;
        case 2:
            cdata += "<![CDATA["; // Nested CDATA start
            break;
        case 3:
            cdata += fuzz_data.ConsumeRandomLengthString(4); // Random content
            break;
        case 4:
            cdata += "]]>]]>"; // Multiple closing attempts
            break;
        }
    }

    cdata += "]]>";
    return cdata;
}

// Helper function to generate comments
std::string generateComment(FuzzedDataProvider& fuzz_data)
{
    std::string comment = "<!--";

    size_t len = fuzz_data.ConsumeIntegralInRange<size_t>(0, 32);
    for (size_t i = 0; i < len; ++i)
    {
        switch (fuzz_data.ConsumeIntegralInRange<int>(0, 4))
        {
        case 0:
            comment += "--"; // Double dash inside comment
            break;
        case 1:
            comment += "-->"; // Early closing
            break;
        case 2:
            comment += "<!--"; // Nested comment start
            break;
        case 3:
            comment += fuzz_data.ConsumeRandomLengthString(4); // Random content
            break;
        case 4:
            comment += "--->"; // Malformed closing
            break;
        }
    }

    comment += "-->";
    return comment;
}

// Helper function to generate text content
std::string generateText(FuzzedDataProvider& fuzz_data)
{
    std::string text;
    size_t      len = fuzz_data.ConsumeIntegralInRange<size_t>(0, 32);

    for (size_t i = 0; i < len; ++i)
    {
        switch (fuzz_data.ConsumeIntegralInRange<int>(0, 3))
        {
        case 0: // Normal text
            text += fuzz_data.ConsumeRandomLengthString(4);
            break;
        case 1: // Special XML characters
            text += "<>&'\"";
            break;
        case 2: // Entity references
            text += generateEntityRef(fuzz_data);
            break;
        case 3: // Control characters
            text += static_cast<char>(fuzz_data.ConsumeIntegralInRange<int>(0, 31));
            break;
        }
    }
    return text;
}

// Helper function to generate nested XML elements
std::string generateXML(FuzzedDataProvider& fuzz_data, int depth = 0)
{
    if (depth > 5 || fuzz_data.remaining_bytes() < 2)
    {
        return generateText(fuzz_data);
    }

    std::string xml;

    // Add pre-content
    if (fuzz_data.ConsumeBool())
    {
        switch (fuzz_data.ConsumeIntegralInRange<int>(0, 2))
        {
        case 0:
            xml += generateComment(fuzz_data);
            break;
        case 1:
            xml += generateCDATA(fuzz_data);
            break;
        case 2:
            xml += generateText(fuzz_data);
            break;
        }
    }

    // Generate element
    std::string tag = generateTagName(fuzz_data);
    xml += "<" + tag;

    // Maybe make it a self-closing tag
    if (depth > 3 || fuzz_data.ConsumeBool())
    {
        xml += "/>";
        return xml;
    }

    xml += ">";

    // Add content
    size_t children = fuzz_data.ConsumeIntegralInRange<size_t>(0, 5);
    for (size_t i = 0; i < children; ++i)
    {
        switch (fuzz_data.ConsumeIntegralInRange<int>(0, 4))
        {
        case 0:
            xml += generateXML(fuzz_data, depth + 1);
            break;
        case 1:
            xml += generateComment(fuzz_data);
            break;
        case 2:
            xml += generateCDATA(fuzz_data);
            break;
        case 3:
            xml += generateText(fuzz_data);
            break;
        case 4:
            xml += generateEntityRef(fuzz_data);
            break;
        }
    }

    // Add closing tag (sometimes intentionally wrong)
    if (fuzz_data.ConsumeBool())
    {
        xml += "</" + tag + ">";
    }
    else
    {
        // Generate malformed closing tags
        switch (fuzz_data.ConsumeIntegralInRange<int>(0, 4))
        {
        case 0:
            xml += "</" + generateTagName(fuzz_data) + ">"; // Wrong tag
            break;
        case 1:
            xml += "</" + tag; // Missing >
            break;
        case 2:
            xml += "</ " + tag + ">"; // Space after /
            break;
        case 3:
            xml += "<"; // Incomplete closing
            break;
        case 4:
            // No closing tag at all
            break;
        }
    }

    return xml;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    if (Size < 4)
        return 0; // Require minimum input size

    FuzzedDataProvider fuzz_data(Data, Size);

    // Generate XML with probability
    std::string input;
    if (fuzz_data.ConsumeBool())
    {
        // Generate well-formed XML
        input = generateXML(fuzz_data);
    }
    else
    {
        // Generate potentially malformed XML
        while (fuzz_data.remaining_bytes() > 0)
        {
            switch (fuzz_data.ConsumeIntegralInRange<int>(0, 6))
            {
            case 0: // Tags
                input += "<" + generateTagName(fuzz_data) + ">";
                break;
            case 1: // Text
                input += generateText(fuzz_data);
                break;
            case 2: // Comments
                input += generateComment(fuzz_data);
                break;
            case 3: // CDATA
                input += generateCDATA(fuzz_data);
                break;
            case 4: // Entity references
                input += generateEntityRef(fuzz_data);
                break;
            case 5: // Random XML-like content
                input += "<" + fuzz_data.ConsumeRandomLengthString(8) + ">"
                         + fuzz_data.ConsumeRandomLengthString(16) + "</"
                         + fuzz_data.ConsumeRandomLengthString(8) + ">";
                break;
            case 6: // Random bytes
                input += fuzz_data.ConsumeRandomLengthString(16);
                break;
            }
        }
    }

    // Create a buffer from the generated input
    auto buffer = lexy::buffer<lexy::utf8_encoding>(input.data(), input.size());

    // Try to parse the XML
    auto result = lexy::parse<grammar::document>(buffer, lexy_ext::report_error);

    if (result.has_value())
    {
        // For valid XML, also test the printing functionality
        result.value()->print();
    }

    return 0;
}
