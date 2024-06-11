#pragma once

#include "Core/IcarianAssert.h"
#include "Logger.h"

#define IERRSTRR(v) #v
#define IERRSTR(v) IERRSTRR(v)

#define IWARN(msg) Logger::Warning("IWARN: " + std::string(msg))
#define IERROR(msg) IcarianError("IERROR: " + std::string(msg) + ": " IERRSTR(__FILE__) "," IERRSTR(__LINE__))
#ifdef NDEBUG
#define IVERIFY(val) void(0)
#else
#define IVERIFY(val) do { if (!(val)) { IERROR(#val); } } while (0)
#endif

void IcarianError(const std::string_view& a_msg);
