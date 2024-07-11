#pragma once

#include <cstdint>
#include <string_view>

// Got inspired by C# and realised that they are using the hash to allow strings in switch statements
// Not gonna pretend to understand the hash
// Credit: http://www.cse.yorku.ca/~oz/hash.html
// Using C str specifically as if the compiler determines it will take too long it will do it at runtime
// Minimises the amount of work the compiler has to do making it more likely to be executed at compile time
template<typename T = uintmax_t>
constexpr static T StringHash(const char* a_str)
{
    T hash = 5381;

    const char* s = a_str;
    while (*s != 0)
    {
        hash = ((hash << 5) + hash) + *s;

        ++s;
    }

    return hash;
}
template<typename T = uintmax_t>
constexpr static T StringHash(const char* a_start, const char* a_end)
{
    T hash = 5381;

    for (const char* s = a_start; s < a_end && *s != 0; ++s)
    {
        hash = ((hash << 5) + hash) + *s;
    }

    return hash;
}

constexpr static bool IsAlphaNumeric(const std::string_view& a_str)
{
    for (const char c : a_str)
    {
        const bool isNum = c >= '0' && c <= '9';
        const bool isLowerCase = c >= 'a' && c <= 'z';
        const bool isUpperCase = c >= 'A' && c <= 'Z';
        if (!isNum && !isLowerCase && !isUpperCase)
        {
            return false;
        }
    }

    return true;
}