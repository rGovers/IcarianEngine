#include "Logger.h"

#include <iostream>

#include "Flare/IcarianDefer.h"
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