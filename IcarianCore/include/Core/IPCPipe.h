// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#endif

#include "Core/CommunicationPipe.h"
#include "Core/PipeMessage.h"

#include <queue>
#include <string_view>

namespace IcarianCore
{   
    class IPCPipe : public CommunicationPipe
    {
    private:
#ifdef WIN32
        SOCKET m_pipeSock;
#else
        int    m_pipeSock;
#endif

        bool   m_closed;

        IPCPipe();

    protected:

    public:
        virtual ~IPCPipe();

        IPCPipe* Accept() const;

        static IPCPipe* Connect(const std::string_view& a_pipeName);
        static IPCPipe* Create(const std::string_view& a_pipeName);

        virtual bool IsAlive() const;

        virtual bool Send(const PipeMessage& a_msg);
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
