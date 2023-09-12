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

// Old version of defer
// Will be cleaned up later here for reference
#if 0
#define ICARIAN_DEFER_VAL_NAMEI(name, i) defer##name##i
#define ICARIAN_DEFER_VAL_NAMEM(name, i) ICARIAN_DEFER_VAL_NAMEI(name, i)
#define ICARIAN_DEFER_VAL_NAME(name) ICARIAN_DEFER_VAL_NAMEM(name, __COUNTER__)

// Got inspired by zig programming language and wanted it in C++
// Macro is a bit of a mess simplest way I could think of was hijacking destructors
// Problem is destructors cannot take parameters out of scope so had to get parameters in scope
// After a bit of digging people on stack overflow where saying had to pass type not happy with that
// Remembered that C++11 I think added decltype so tried that but then symbol mangaling occured
// So had to type alias the decltype
// Still somehow more readable then templates
// Not perfect as still have to pass a parameter and not just the code but it works
// Yes is still refuse to use smart pointers they are still awful 
// Seems to get optimized away so it gets the pass for no cost abstraction in release
#define ICARIAN_DEFER(val, func) using t_##val = decltype(val); \
const struct _##val \
{ \
    t_##val val; \
    _##val(t_##val a_##val) : val(a_##val) { } \
    ~_##val() \
    { \
        func; \
    } \
} ICARIAN_DEFER_VAL_NAME(Val)(val)

#define ICARIAN_DEFER_T(val, type, func) const struct _##val \
{ \
    type val; \
    _##val(type a_##val) : val(a_##val) { } \
    ~_##val() \
    { \
        func; \
    } \
} ICARIAN_DEFER_VAL_NAME(ValT)(val)

#define ICARIAN_DEFERF(func) const struct _##func \
{ \
    ~_##func() \
    { \
        func(); \
    } \
} ICARIAN_DEFER_VAL_NAME(Func)

#define ICARIAN_DEFER_del(val) ICARIAN_DEFER(val, delete val)
#define ICARIAN_DEFER_delA(val) ICARIAN_DEFER(val, delete[] val)
#define ICARIAN_DEFER_free(val) ICARIAN_DEFER(val, free(val))
#define ICARIAN_DEFER_closeIFile(val) ICARIAN_DEFER_T(val, std::ifstream&, val.close())
#define ICARIAN_DEFER_closeOFile(val) ICARIAN_DEFER_T(val, std::ofstream&, val.close());
#define ICARIAN_DEFER_monoF(val) ICARIAN_DEFER(val, mono_free(val))
#endif