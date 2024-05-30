#pragma once

#include <cstdint>

// Got inspired by C# and realised that they are using the hash to allow strings in switch statements
// Not gonna pretend to understand the hash
// Credit: http://www.cse.yorku.ca/~oz/hash.html
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