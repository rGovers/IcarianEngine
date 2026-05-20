// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "Core/CommunicationPipe.h"

#include <enet/enet.h>
#include <mutex>
#include <queue>
#include <string_view>
#include <thread>

#include "Core/PipeMessage.h"

namespace IcarianCore
{   
    class SocketPipe : public CommunicationPipe
    {
    private:
        ENetHost*               m_host;
        ENetPeer*               m_peer;

        std::mutex              m_readLock;
        std::mutex              m_writeLock;

        std::queue<PipeMessage> m_readQueue;
        std::queue<PipeMessage> m_writeQueue;

        std::thread             m_thread;

        volatile bool           m_join;
        volatile bool           m_joined;

        SocketPipe();

        static void Run(SocketPipe* a_pipe);

    protected:

    public:
        virtual ~SocketPipe();

        SocketPipe* Accept() const;

        static SocketPipe* Connect(const std::string_view& a_addr, uint16_t a_port);
        static SocketPipe* Create(uint16_t a_port);

        virtual bool IsAlive() const;

        virtual e_SendError Send(const PipeMessage& a_msg);
        virtual bool Receive(std::queue<PipeMessage>* a_messages);
    };
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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