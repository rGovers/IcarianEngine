// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Logger.h"

#include <iostream>

#include "Core/IcarianDefer.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

Logger::Callback* Logger::CallbackFunc = nullptr;

RUNTIME_FUNCTION(void, Logger, PushMessage, 
{
    char* str = mono_string_to_utf8(a_string);
    IDEFER(mono_free(str));

    Logger::Message(str);
}, MonoString* a_string)
RUNTIME_FUNCTION(void, Logger, PushWarning, 
{
    char* str = mono_string_to_utf8(a_string);
    IDEFER(mono_free(str));

    Logger::Warning(str);
}, MonoString* a_string)
RUNTIME_FUNCTION(void, Logger, PushError, 
{
    char* str = mono_string_to_utf8(a_string);
    IDEFER(mono_free(str));

    Logger::Error(str);
}, MonoString* a_string)

void Logger::Message(const std::string_view& a_msg)
{
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(a_msg, LoggerMessageType_Message);
    }
    
    std::cout<< "FEL: " << a_msg << "\n";
}
void Logger::Warning(const std::string_view& a_msg)
{
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(a_msg, LoggerMessageType_Warning);
    }
    
    std::cout << "FEL: " << a_msg << "\n";
}
void Logger::Error(const std::string_view& a_msg)
{
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(a_msg, LoggerMessageType_Error);
    }
    
    std::cout << "FEL: " << a_msg << "\n";
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
// Copyright (c) 2024 River Govers
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