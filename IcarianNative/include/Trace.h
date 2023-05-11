#pragma once

#include <stdio.h>

#if !defined(NDEBUG) && !defined(FLARENATIVE_ENABLE_TRACE)
#define FLARENATIVE_ENABLE_TRACE
#endif

#ifndef TRACE
#ifdef FLARENATIVE_ENABLE_TRACE
#define TRACE(str) printf("IcarianEngine: %s \n", str)
#else
#define TRACE(str) void(0)
#endif
#endif