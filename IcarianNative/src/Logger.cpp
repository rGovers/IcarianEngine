// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Logger.h"

#include <iostream>

#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "DataTypes/Allocators/MallocAllocator.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

Logger::Callback* Logger::CallbackFunc = nullptr;

RUNTIME_FUNCTION(void, Logger, PushMessage,
{
    char* str = mono_string_to_utf8(a_string);
    IDEFER(mono_free(str));

    const uint32_t stackTraceCount = ILAMBDA(
    {
        if (a_stackTrace != NULL)
        {
            ILRETURN (uint32_t)mono_array_length(a_stackTrace);
        }

        ILRETURN uint32_t(0);
    });

    char** stackTrace = ILAMBDA(
    {
        if (stackTraceCount <= 0)
        {
            ILRETURN (char**)nullptr;
        }

        char** vals = MallocAllocator::Instance->TAllocate<char*>(stackTraceCount);

        for (uint32_t i = 0; i < stackTraceCount; ++i)
        {
            MonoString* string = mono_array_get(a_stackTrace, MonoString*, i);

            vals[i] = mono_string_to_utf8(string);
        }

        ILRETURN vals;
    });
    IDEFER(
    {
        if (stackTrace != nullptr)
        {
            for (uint32_t i = 0; i < stackTraceCount; ++i)
            {
                mono_free(stackTrace[i]);
            }

            MallocAllocator::Instance->Free(stackTrace);
        }
    });

    Logger::Message(str, stackTraceCount, stackTrace);
}, MonoString* a_string, MonoArray* a_stackTrace)
RUNTIME_FUNCTION(void, Logger, PushWarning,
{
    char* str = mono_string_to_utf8(a_string);
    IDEFER(mono_free(str));

    const uint32_t stackTraceCount = ILAMBDA(
    {
        if (a_stackTrace != NULL)
        {
            ILRETURN (uint32_t)mono_array_length(a_stackTrace);
        }

        ILRETURN uint32_t(0);
    });

    char** stackTrace = ILAMBDA(
    {
        if (stackTraceCount <= 0)
        {
            ILRETURN (char**)nullptr;
        }

        char** vals = MallocAllocator::Instance->TAllocate<char*>(stackTraceCount);

        for (uint32_t i = 0; i < stackTraceCount; ++i)
        {
            MonoString* string = mono_array_get(a_stackTrace, MonoString*, i);

            vals[i] = mono_string_to_utf8(string);
        }

        ILRETURN vals;
    });
    IDEFER(
    {
        if (stackTrace != nullptr)
        {
            for (uint32_t i = 0; i < stackTraceCount; ++i)
            {
                mono_free(stackTrace[i]);
            }

            MallocAllocator::Instance->Free(stackTrace);
        }
    });

    Logger::Warning(str, stackTraceCount, stackTrace);
}, MonoString* a_string, MonoArray* a_stackTrace)
RUNTIME_FUNCTION(void, Logger, PushError,
{
    char* str = mono_string_to_utf8(a_string);
    IDEFER(mono_free(str));

    const uint32_t stackTraceCount = ILAMBDA(
    {
        if (a_stackTrace != NULL)
        {
            ILRETURN (uint32_t)mono_array_length(a_stackTrace);
        }

        ILRETURN uint32_t(0);
    });

    char** stackTrace = ILAMBDA(
    {
        if (stackTraceCount <= 0)
        {
            ILRETURN (char**)nullptr;
        }

        char** vals = MallocAllocator::Instance->TAllocate<char*>(stackTraceCount);

        for (uint32_t i = 0; i < stackTraceCount; ++i)
        {
            MonoString* string = mono_array_get(a_stackTrace, MonoString*, i);

            vals[i] = mono_string_to_utf8(string);
        }

        ILRETURN vals;
    });
    IDEFER(
    {
        if (stackTrace != nullptr)
        {
            for (uint32_t i = 0; i < stackTraceCount; ++i)
            {
                mono_free(stackTrace[i]);
            }

            MallocAllocator::Instance->Free(stackTrace);
        }
    });

    Logger::Error(str, stackTraceCount, stackTrace);
}, MonoString* a_string, MonoArray* a_stackTrace)

void Logger::Message(const char* a_msg, uint32_t a_stackTraceCount, const char* const* a_stackTrace)
{
    const COWU8String msg = COWU8String(a_msg, MallocAllocator::Instance);

    Message(msg, a_stackTraceCount, a_stackTrace);
}
void Logger::Message(const COWU8String& a_msg, uint32_t a_stackTraceCount, const char* const* a_stackTrace)
{
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(a_msg, IcarianCore::LoggerMessageType_Message, a_stackTraceCount, a_stackTrace);
    }

    std::cout << "IEM: " << a_msg.CStr() << "\n";
}
void Logger::Warning(const char* a_msg, uint32_t a_stackTraceCount, const char* const* a_stackTrace)
{
    const COWU8String msg = COWU8String(a_msg, MallocAllocator::Instance);

    Warning(msg, a_stackTraceCount, a_stackTrace);
}
void Logger::Warning(const COWU8String& a_msg, uint32_t a_stackTraceCount, const char* const* a_stackTrace)
{
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(a_msg, IcarianCore::LoggerMessageType_Warning, a_stackTraceCount, a_stackTrace);
    }

    std::cout << "IEW: " << a_msg.CStr() << "\n";
}
void Logger::Error(const char* a_msg, uint32_t a_stackTraceCount, const char* const* a_stackTrace)
{
    const COWU8String msg = COWU8String(a_msg, MallocAllocator::Instance);

    Error(msg, a_stackTraceCount, a_stackTrace);
}
void Logger::Error(const COWU8String& a_msg, uint32_t a_stackTraceCount, const char* const* a_stackTrace)
{
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(a_msg, IcarianCore::LoggerMessageType_Error, a_stackTraceCount, a_stackTrace);
    }

    std::cout << "IEE: " << a_msg.CStr() << "\n";
}
void Logger::Init()
{
    TRACE("Initializing C# Logger");

    BIND_FUNCTION(IcarianEngine, Logger, PushMessage);
    BIND_FUNCTION(IcarianEngine, Logger, PushWarning);
    BIND_FUNCTION(IcarianEngine, Logger, PushError);
}

// MIT License
// 
// Copyright (c) 2026 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.