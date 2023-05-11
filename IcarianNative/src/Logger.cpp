#include "Logger.h"

#include <iostream>

#include "Runtime/RuntimeManager.h"
#include "Trace.h"

Logger::Callback* Logger::CallbackFunc = nullptr;

FLARE_MONO_EXPORT(void, Logger_PushMessage, MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    Logger::Message(str);

    mono_free(str);
}
FLARE_MONO_EXPORT(void, Logger_PushWarning, MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    Logger::Warning(str);

    mono_free(str);
}
FLARE_MONO_EXPORT(void, Logger_PushError, MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    Logger::Error(str);

    mono_free(str);
}

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
void Logger::InitRuntime(RuntimeManager* a_runtime)
{
    TRACE("Initializing C# Logger");

    a_runtime->BindFunction("IcarianEngine.Logger::PushMessage", (void*)Logger_PushMessage);
    a_runtime->BindFunction("IcarianEngine.Logger::PushWarning", (void*)Logger_PushWarning);
    a_runtime->BindFunction("IcarianEngine.Logger::PushError", (void*)Logger_PushError);
}