// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once 

#include <queue>

#include "Core/PipeMessage.h"

#define I_INTER_SENDERRORTABLE(F) \
    F(Success) \
    F(Timeout) \
    F(Fail) \

#define I_INTER_SENDERROR_ENUM(val) SendError_##val,
#define I_INTER_SENDERROR_STR(val) case SendError_##val: { return #val; }

namespace IcarianCore
{
    class CommunicationPipe
    {
    public:
        enum e_SendError
        {
            I_INTER_SENDERRORTABLE(I_INTER_SENDERROR_ENUM)
        };

    private:

    protected:

    public:
        static constexpr const char* SendErrorString(e_SendError a_error)
        {
            switch (a_error)
            {
            I_INTER_SENDERRORTABLE(I_INTER_SENDERROR_STR)
            default:
            {
                break;
            }
            }

            return "Unknown";
        }

        virtual ~CommunicationPipe() { };

        virtual bool IsAlive() const = 0;

        virtual e_SendError Send(const PipeMessage& a_msg) = 0;
        virtual bool Receive(std::queue<PipeMessage>* a_messages) = 0;
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