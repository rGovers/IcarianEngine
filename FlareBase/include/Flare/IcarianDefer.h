#pragma once

// The magic 3 macros that make the names format correctly
#define ICARIAN_DEFER_NAMEI(a, b) a##b
#define ICARIAN_DEFER_NAMEM(a, b) ICARIAN_DEFER_NAMEI(a, b)
#define ICARIAN_DEFER_NAME(a) ICARIAN_DEFER_NAMEM(a, __LINE__)

// Improved version of defer
// I have discovered the trinity of auto decltype and using
// Figured out how to get rid of the need to pass a parameter
// A mess that exploits the fact that C++ compilers are aggressive with optimizations and inlining with const values and single use variables
// and to the people that say just use templates, fuck templates
#define IDEFER(code) \
    const auto ICARIAN_DEFER_NAME(_defer) = [&] { code; }; \
    using ICARIAN_DEFER_NAME(_t) = decltype(ICARIAN_DEFER_NAME(_defer)); \
    const struct ICARIAN_DEFER_NAME(_defer_struct) \
    { \
        ICARIAN_DEFER_NAME(_t) m_val; \
        ICARIAN_DEFER_NAME(_defer_struct)(ICARIAN_DEFER_NAME(_t) a_val) : m_val(a_val) { } \
        ~ICARIAN_DEFER_NAME(_defer_struct)() \
        { \
            m_val(); \
        } \
    } ICARIAN_DEFER_NAME(_defer_var)(ICARIAN_DEFER_NAME(_defer))
