#pragma once

#include "Flare/PipeMessage.h"
#include "Flare/WindowsHeaders.h"

#include <queue>
#include <string_view>

namespace FlareBase
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
