#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

enum e_LoggerMessageType : uint32_t
{
    LoggerMessageType_Message,
    LoggerMessageType_Warning,
    LoggerMessageType_Error
};

class Logger
{
public:
    typedef std::function<void(const std::string_view&, e_LoggerMessageType)> Callback;
private:

protected:

public:
    static Callback* CallbackFunc;

    static void Message(const std::string_view& a_msg);
    static void Warning(const std::string_view& a_msg);
    static void Error(const std::string_view& a_msg);

    static void Init();
};