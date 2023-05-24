#pragma once

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
} D_##val(val)

#define ICARIAN_DEFER_T(val, type, func) const struct _##val \
{ \
    type val; \
    _##val(type a_##val) : val(a_##val) { } \
    ~_##val() \
    { \
        func; \
    } \
} D_##val(val)

#define ICARIAN_DEFERF(func) const struct _##func \
{ \
    ~_##func() \
    { \
        func(); \
    } \
} D_##func;

#define ICARIAN_DEFER_del(val) ICARIAN_DEFER(val, delete val)
#define ICARIAN_DEFER_delA(val) ICARIAN_DEFER(val, delete[] val)
#define ICARIAN_DEFER_free(val) ICARIAN_DEFER(val, free(val))
#define ICARIAN_DEFER_closeIFile(val) ICARIAN_DEFER_T(val, std::ifstream&, val.close())
#define ICARIAN_DEFER_closeOFile(val) ICARIAN_DEFER_T(val, std::ofstream&, val.close());
#define ICARIAN_DEFER_monoF(val) ICARIAN_DEFER(val, mono_free(val))