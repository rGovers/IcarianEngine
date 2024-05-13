#pragma once

#include "Core/PipeMessage.h"
#include "Core/WindowsHeaders.h"

#include <queue>
#include <string_view>

namespace IcarianCore
{   
    class IPCPipe
    {
    private:
#if WIN32
        SOCKET m_pipeSock;
#else
        int    m_pipeSock;
#endif

        IPCPipe();

    protected:

    public:
        ~IPCPipe();

        IPCPipe* Accept() const;

        static IPCPipe* Connect(const std::string_view& a_pipeName);
        static IPCPipe* Create(const std::string_view& a_pipeName);

        bool Send(const PipeMessage& a_msg) const;
        bool Receive(std::queue<PipeMessage>* a_messages) const;
    };
}
