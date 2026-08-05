// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Core/DataTypes/Allocators/StackAllocator.h"

#include "Core/IcarianDefer.h"
#include "Core/IcarianMemory.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    StackAllocator::StackAllocator(uint64_t a_size, Allocator* a_upstreamAllocator)
    {
        m_upstreamAllocator = a_upstreamAllocator;

        m_memory = m_upstreamAllocator->Allocate(a_size, BaseAlignment);
        m_end = (char*)m_memory + a_size;
        m_stackPointer = m_memory;
        m_stackSlider = m_memory;
    }
    StackAllocator::~StackAllocator()
    {
        m_upstreamAllocator->Free(m_memory);
    }

    void StackAllocator::PushStackPointer()
    {
        [[maybe_unused]] const uintptr_t oldStackSlider = (uintptr_t)m_stackSlider;

        uintptr_t* oldPtr = TAllocate<uintptr_t>();
        *oldPtr = (uintptr_t)m_stackPointer;

        m_stackPointer = m_stackSlider;

        ICARIAN_ASSERT((uintptr_t)m_stackSlider - oldStackSlider >= sizeof(uintptr_t));
    }
    void StackAllocator::PopStackPointer()
    {
        if (m_stackPointer <= m_memory)
        {
            ICARIAN_ASSERT_MSG(0, "Pop at bottom of the Stack");

            exit(1);
        }

        m_stackSlider = m_stackPointer;
        m_stackPointer = (void*)*((uintptr_t*)m_stackPointer - 1);
    }

    void StackAllocator::Reset()
    {
        m_stackPointer = m_memory;
        m_stackSlider = m_memory;
    }

    void* StackAllocator::Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size <= 0)
        {
            return nullptr;
        }

        void* ptr = AlignTo((uint8_t*)m_stackSlider, a_alignment);
        void* next = (uint8_t*)ptr + a_size;
        if (next > m_end)
        {
            ICARIAN_ASSERT_MSG(0, "Stack Allocator out of memory");

            exit(1);
        }

        IDEFER(m_stackSlider = next);

        return ptr;
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
