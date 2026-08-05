// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/DataTypes/Allocators/ComplexAllocator.h"

#ifdef __linux__
#ifdef __GNUC__
#include <cxxabi.h>
#endif
#endif

#include "Core/DataTypes/Dictionary.h"
#include "Core/DataTypes/SpinLock.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    class LeakAllocator : public ComplexAllocator
    {
    private:
        constexpr static uint32_t BacktraceSize = 128;

        struct Metadata
        {
    #ifdef __linux__
            Array<void*> Backtrace;
    #endif
            uint64_t Size;
        };

        Allocator*                  m_upstreamAllocator;
        Allocator*                  m_storageAllocator;

        Dictionary<void*, Metadata> m_data;
        // Yes locking on every allocation is bad however this allocator is only used in Debug
        // I have a lot more to worry about in Debug performance wise
        // We should probably handle this better down the line however
        SpinLock                    m_lock;

        static void PrintBacktrace(const Metadata& a_metadata);

    protected:

    public:
        LeakAllocator(Allocator* a_upstreamAllocator);
        LeakAllocator(Allocator* a_upstreamAllocator, Allocator* a_storageAllocator);
        virtual ~LeakAllocator();

        inline Allocator* GetUpstreamAllocator() const
        {
            return m_upstreamAllocator;
        }
        inline Allocator* GetStorageAllocator() const
        {
            return m_storageAllocator;
        }

        [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment);
        [[nodiscard]] virtual void* Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment);
        virtual void Free(void* a_ptr);
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
