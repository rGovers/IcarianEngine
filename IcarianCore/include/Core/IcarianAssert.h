#pragma once

#ifndef NDEBUG 
#ifndef ICARIAN_ENABLE_ASSERT
#define ICARIAN_ENABLE_ASSERT
#endif
#endif

#include <cassert>
#include <string>

typedef void (*AssertCallback)(std::string);

static void DefaultAssertCallback(const std::string& a_string)
{
    printf("%s \n", a_string.c_str());
}

static AssertCallback AssertCallbackFunc = (AssertCallback)DefaultAssertCallback;

#ifdef ICARIAN_ENABLE_ASSERT
#define ICARIAN_ASSERT(val) if (!(val)) { AssertCallbackFunc("IcarianAssert: " #val); assert(0); }
#define ICARIAN_ASSERT_R(val) if (!(val)) { AssertCallbackFunc("IcarianAssert: " #val); assert(0); }
#define ICARIAN_ASSERT_MSG(val, msg) if (!(val)) { AssertCallbackFunc(std::string("IcarianAssert: ") + (msg) + ": " #val); assert(0); }
#define ICARIAN_ASSERT_MSG_R(val, msg) if (!(val)) { AssertCallbackFunc(std::string("IcarianAssert: ") + (msg) + ": " #val); assert(0); }
#else
#define ICARIAN_ASSERT(val) void(0);
#define ICARIAN_ASSERT_R(val) if (!(val)) { AssertCallbackFunc("IcarianAssert: " #val); }
#define ICARIAN_ASSERT_MSG(val, msg) void(0);
#define ICARIAN_ASSERT_MSG_R(val, msg) if (!(val)) { AssertCallbackFunc(std::string("IcarianAssert: ") + (msg) + ": " #val); }
#endif