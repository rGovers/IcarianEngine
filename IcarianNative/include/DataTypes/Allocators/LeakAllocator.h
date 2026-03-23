// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/Allocators/ComplexAllocator.h"

#ifdef __linux__
#ifdef __GNUC__
#include <cxxabi.h>
#endif
#endif

#include <cstdio>

#include "DataTypes/COWString.h"
#include "DataTypes/Dictionary.h"
#include "DataTypes/SpinLock.h"
#include "DataTypes/ThreadGuard.h"

class LeakAllocator : public ComplexAllocator
{
private:
    constexpr static uint32_t BacktraceSize = 128;

    struct Metadata
    {
        Array<COWU8String> Stacktrace;
        uint64_t Size;
    };

    Allocator*                  m_upstreamAllocator;
    Allocator*                  m_storageAllocator;

    Dictionary<void*, Metadata> m_data;
    // Yes locking on every allocation is bad however this allocator is only used in Debug
    // I have a lot more to worry about in Debug performance wise
    // We should probably handle this better down the line however
    SpinLock                    m_lock;

protected:

public:
    LeakAllocator(Allocator* a_upstreamAllocator) : LeakAllocator(a_upstreamAllocator, a_upstreamAllocator)
    {

    }
    LeakAllocator(Allocator* a_upstreamAllocator, Allocator* a_storageAllocator) :
        m_upstreamAllocator(a_upstreamAllocator),
        m_storageAllocator(a_storageAllocator),
        m_data(m_storageAllocator)
    {

    }
    virtual ~LeakAllocator()
    {
        const Array<Metadata> metadata = m_data.GetValues(m_storageAllocator);

        if (!metadata.Empty())
        {
            for (const Metadata& m : metadata)
            {
                printf(" --------------------------------------- \n\n");
                printf("    Leaked allocation of size %lu bytes \n\n", m.Size);
                printf(" --------------------------------------- \n\n");

                const uint32_t size = m.Stacktrace.Size();
                if (size <= 0)
                {
                    printf("Unknown Stacktrace \n");

                    continue;
                }

                for (uint32_t i = 0; i < size; ++i)
                {
                    printf("[%d] %s \n", i, m.Stacktrace[i].CStr());
                }
            }

            IERROR("Leak allocator detected leaked memory");
        }
    }

    inline Allocator* GetUpstreamAllocator() const
    {
        return m_upstreamAllocator;
    }
    inline Allocator* GetStorageAllocator() const
    {
        return m_storageAllocator;
    }

    [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size <= 0)
        {
            return nullptr;
        }

        const ThreadGuard g = ThreadGuard(m_lock);

        void* ptr = m_upstreamAllocator->Allocate(a_size, a_alignment);

        Array<COWU8String> bkArray = Array<COWU8String>(m_upstreamAllocator);
#ifdef __linux__
        void* array[BacktraceSize];
        const uint32_t size = (uint32_t)backtrace(array, BacktraceSize);

        char** symbols = backtrace_symbols(array, (int)size);
        IDEFER(free(symbols));

        for (uint32_t i = 0; i < size; ++i)
        {
            const char* str = symbols[i];
            if (str == nullptr)
            {
                const COWU8String symbolStr = COWU8String("Unknown Symbol", m_storageAllocator);
                bkArray.Push(symbolStr);

                continue;
            }

#ifdef __GNUC__
            const char* slider = str;
            while (*slider != 0)
            {
                ++slider;
            }

            char* buffer = m_storageAllocator->ZTAllocate<char>((slider - str) + 1);
            IDEFER(m_storageAllocator->Free(buffer));

            if (buffer != NULL)
            {
                bool write = false;
                uint32_t writeIndex = 0;

                slider = str;
                while (true)
                {
                    const char chr = *slider;
                    if (chr == 0)
                    {
                        break;
                    }

                    if (chr == ')' || chr == '+')
                    {
                        buffer[writeIndex] = 0;

                        break;
                    }

                    if (write)
                    {
                        buffer[writeIndex++] = chr;
                    }

                    if (chr == '(')
                    {
                        write = true;
                    }

                    ++slider;
                }

                char* name = abi::__cxa_demangle(buffer, NULL, NULL, NULL);
                if (name == NULL)
                {
                    const COWU8String symbolStr = COWU8String(buffer, m_storageAllocator);
                    bkArray.Push(symbolStr);

                    continue;
                }
                IDEFER(free(name));

                const COWU8String symbolStr = COWU8String(name, m_storageAllocator);
                bkArray.Push(symbolStr);
            }
            else
            {
                const COWU8String symbolStr = COWU8String("Unknown Symbol", m_storageAllocator);
                bkArray.Push(symbolStr);
            }
#else
            const COWU8String symbolStr = COWU8String(str, m_storageAllocator);
            bkArray.Push(symbolStr);
#endif
        }
#endif

        const Metadata data =
        {
            .Stacktrace = bkArray,
            .Size = a_size,
        };

        m_data.Push(ptr, data);

        return ptr;
    }

    [[nodiscard]] virtual void* Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment)
    {
        if (a_ptr == nullptr)
        {
            return Allocate(a_size, a_alignment);
        }

        {
            const ThreadGuard g = ThreadGuard(m_lock);

            if (!m_data.Exists(a_ptr))
            {
                IERROR("Likely multi free or does not exist");
            }
        }
        IDEFER(Free(a_ptr));

        const Metadata data = ILAMBDA(
        {
            const ThreadGuard g = ThreadGuard(m_lock);

            ILRETURN m_data[a_ptr];
        });
        void* ptr = Allocate(a_size, a_alignment);
        memcpy(ptr, a_ptr, data.Size);

        return ptr;
    }

    virtual void Free(void* a_ptr)
    {
        if (a_ptr == nullptr)
        {
            return;
        }

        const ThreadGuard g = ThreadGuard(m_lock);
        if (!m_data.Exists(a_ptr))
        {
            IERROR("Likely multi free or does not exist");
        }
        m_data.Erase(a_ptr);

        m_upstreamAllocator->Free(a_ptr);
    }
};

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