// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Core/DataTypes/Allocators/RingAllocator.h"

#include "Core/IcarianDefer.h"
#include "Core/IcarianMemory.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    RingAllocator::RingAllocator(uint64_t a_size, Allocator* a_upstreamAllocator)
    {
        m_upstreamAllocator = a_upstreamAllocator;

        m_memory = m_upstreamAllocator->Allocate(a_size, BaseAlignment);
        m_slider = m_memory;
        m_end = (void*)((char*)m_memory + a_size);
    }
    RingAllocator::~RingAllocator()
    {
        m_upstreamAllocator->Free(m_memory);
    }

    void* RingAllocator::Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size <= 0)
        {
            return nullptr;
        }

        const void* next = AlignTo((uint8_t*)m_slider + a_size, (uintptr_t)a_alignment);
        if (next >= (uint8_t*)m_end)
        {
            m_slider = m_memory;
        }

        void* basePtr = AlignTo((uint8_t*)m_slider, (uintptr_t)a_alignment);
        IDEFER(m_slider = (uint8_t*)basePtr + a_size);

        return basePtr;
    }
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
