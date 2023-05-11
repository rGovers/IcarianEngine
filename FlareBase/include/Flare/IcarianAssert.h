#pragma once

#ifndef NDEBUG
#define ICARIAN_ENABLE_ASSERT
#endif

#include <cassert>
#include <string>

#include "Logger.h"

#ifdef ICARIAN_ENABLE_ASSERT
#define ICARIAN_ASSERT(val) if (!(val)) { Logger::Error("IcarianAssert: " #val); assert(0); }
#define ICARIAN_ASSERT_R(val) if (!(val)) { Logger::Error("IcarianAssert: " #val); assert(0); }
#define ICARIAN_ASSERT_MSG(val, msg) if (!(val)) { Logger::Error(std::string("IcarianAssert: ") + (msg) + ": " #val); assert(0); }
#define ICARIAN_ASSERT_MSG_R(val, msg) if (!(val)) { Logger::Error(std::string("IcarianAssert: ") + (msg) + ": " #val); assert(0); }
#else
#define ICARIAN_ASSERT void(0);
#define ICARIAN_ASSERT_R(val) if (!(val)) { Logger::Error("IcarianAssert: " #val); }
#define ICARIAN_ASSERT_MSG(val, msg) void(0);
#define ICARIAN_ASSERT_MSG_R(val, msg) if (!(val)) { Logger::Error(std::string("IcarianAssert: ") + (msg) + ": " #val); }
#endif