#pragma once

#if WIN32
#ifdef __MINGW32__
#include <winsock2.h>
#include <windows.h>
#else
#include <WinSock2.h>
#include <Windows.h>
#endif

#include <afunix.h>
#endif