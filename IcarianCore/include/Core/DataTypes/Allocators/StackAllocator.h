// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/DataTypes/Allocators/Allocator.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    class StackAllocator : public Allocator
    {
    private:
        Allocator* m_upstreamAllocator;

        void*      m_memory;
        void*      m_end;
        void*      m_stackPointer;
        void*      m_stackSlider;

    protected:

    public:
        StackAllocator(uint64_t a_size, Allocator* a_upstreamAllocator);
        virtual ~StackAllocator();

        inline Allocator* GetUpstreamAllocator() const
        {
            return m_upstreamAllocator;
        }

        inline uint64_t GetSize() const
        {
            return (uint64_t)((char*)m_end - (char*)m_memory);
        }
        inline uint64_t GetUsedSize() const
        {
            return (uint64_t)((char*)m_stackSlider - (char*)m_memory);
        }
        inline uint64_t GetRemainingSize() const
        {
            return (uint64_t)((char*)m_end - (char*)m_stackSlider);
        }

        void PushStackPointer();
        void PopStackPointer();

        void Reset();

        [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment);
    };
}

ICARIAN_POP_FASTALLOCTOR

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
