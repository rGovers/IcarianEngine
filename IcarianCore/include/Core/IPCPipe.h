// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#endif

#include "Core/CommunicationPipe.h"
#include "Core/PipeMessage.h"

#include <mutex>
#include <queue>
#include <thread>

namespace IcarianCore
{
    class IPCPipe : public CommunicationPipe
    {
    private:
        constexpr static uint32_t PartialReadBit = 0;
        constexpr static uint32_t PartialWriteBit = 1;

        // TODO: Probably need to update the Windows implementation of this class
        // I have just left it for the time being
#ifdef WIN32
        SOCKET                  m_pipeSock;
#else
        int                     m_pipeSock;
#endif

        std::mutex              m_sendLock;
        std::mutex              m_receiveLock;

        std::queue<PipeMessage> m_sendQueue;
        std::queue<PipeMessage> m_receiveQueue;

        std::thread             m_thread;

        uint32_t                m_sendOffset;
        uint32_t                m_receiveOffset;

        PipeMessage             m_partialSend;
        PipeMessage             m_partialReceive;

        uint8_t                 m_flags;

        volatile bool           m_join;
        volatile bool           m_joined;

#ifdef WIN32
        IPCPipe(SOCKET a_socket);
#else
        IPCPipe(int a_socket);
#endif

        static void Run(IPCPipe* a_pipe);

    protected:

    public:
        virtual ~IPCPipe();

        IPCPipe* Accept(float a_timeoutSec) const;

        static IPCPipe* Connect(const char* a_pipeName);
        static IPCPipe* Create(const char* a_pipeName);

        virtual bool IsAlive() const;

        virtual e_SendError Send(const PipeMessage& a_msg);
        virtual bool Receive(std::queue<PipeMessage>* a_messages);
    };
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
