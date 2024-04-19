#pragma once

#include "Core/IcarianAssert.h"
#include "Logger.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#endif

#define IERRSTRR(v) #v
#define IERRSTR(v) IERRSTRR(v)

#define IWARN(msg) Logger::Warning("IWARN: " + std::string(msg))
#define IERROR(msg) IcarianError("IERROR: " + std::string(msg) + ": " IERRSTR(__FILE__) "," IERRSTR(__LINE__))

static void IcarianError(const std::string_view& a_msg)
{
#ifdef WIN32
    MessageBoxA(NULL, msg.data(), NULL, MB_OK);
#endif

    ICARIAN_ASSERT_MSG_R(0, a_msg);
}