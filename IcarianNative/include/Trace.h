#pragma once

#include <cstdio>

#if !defined(NDEBUG) && !defined(ICARIANNATIVE_ENABLE_TRACE)
#define ICARIANNATIVE_ENABLE_TRACE
#endif

#ifndef TRACE
#ifdef ICARIANNATIVE_ENABLE_TRACE
#define TRACE(str) std::printf("IcarianEngine: %s \n", str)
#else
#define TRACE(str) void(0)
#endif
#endif