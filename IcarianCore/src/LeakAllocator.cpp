// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Core/DataTypes/Allocators/LeakAllocator.h"

#include "Core/DataTypes/ThreadGuard.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    LeakAllocator::LeakAllocator(Allocator* a_upstreamAllocator) : LeakAllocator(a_upstreamAllocator, a_upstreamAllocator)
    {

    }
    LeakAllocator::LeakAllocator(Allocator* a_upstreamAllocator, Allocator* a_storageAllocator) :
        m_data(a_storageAllocator)
    {
        m_upstreamAllocator = a_upstreamAllocator;
        m_storageAllocator = a_storageAllocator;
    }
    LeakAllocator::~LeakAllocator()
    {
        const Array<Metadata> metadata = m_data.GetValues(m_storageAllocator);

        if (!metadata.Empty())
        {
            for (const Metadata& m : metadata)
            {
                printf(" --------------------------------------- \n\n");
                printf("    Leaked allocation of size %lu bytes \n\n", m.Size);
                printf(" --------------------------------------- \n\n");

                PrintBacktrace(m);
            }

            ICARIAN_ASSERT_MSG(0, "Leak allocator detected leaked memory");

            exit(1);
        }
    }

    void* LeakAllocator::Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size <= 0)
        {
            return nullptr;
        }

        const ThreadGuard g = ThreadGuard(m_lock);

        void* ptr = m_upstreamAllocator->Allocate(a_size, a_alignment);

#ifdef __linux__
        void* bt[BacktraceSize];
        const uint32_t btSize = (uint32_t)backtrace(bt, BacktraceSize);

        const Metadata data =
        {
            .Backtrace = Array<void*>(bt, btSize, m_storageAllocator),
            .Size = a_size,
        };

        m_data.Push(ptr, data);
#endif

        return ptr;
    }
    void* LeakAllocator::Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment)
    {
        if (a_ptr == nullptr)
        {
            return Allocate(a_size, a_alignment);
        }

        {
            const ThreadGuard g = ThreadGuard(m_lock);

            ICARIAN_ASSERT_MSG_R(m_data.Exists(a_ptr), "LeakAllocator pointer does not exist");
        }

        const Metadata data = ILAMBDA(
        {
            const ThreadGuard g = ThreadGuard(m_lock);

            ILRETURN m_data[a_ptr];
        });

        if (data.Size >= a_size)
        {
            return a_ptr;
        }
        IDEFER(Free(a_ptr));

        void* ptr = Allocate(a_size, a_alignment);
        memcpy(ptr, a_ptr, data.Size);

        return ptr;
    }
    void LeakAllocator::Free(void* a_ptr)
    {
        if (a_ptr == nullptr)
        {
            return;
        }

        const ThreadGuard g = ThreadGuard(m_lock);

        ICARIAN_ASSERT_MSG(m_data.Exists(a_ptr), "LeakAllocator pointer does not exist");
        m_data.Erase(a_ptr);

        m_upstreamAllocator->Free(a_ptr);
    }

    void LeakAllocator::PrintBacktrace(const Metadata& a_metadata)
    {
#ifdef __linux__
        const uint32_t backtraceSize = a_metadata.Backtrace.Size();
        if (backtraceSize <= 1)
        {
            printf("Unknown Stacktrace \n");

            return;
        }

        void* const* backtraceData = a_metadata.Backtrace.Data();
        char** symbols = backtrace_symbols(backtraceData, backtraceSize);
        IDEFER(free(symbols));

        for (uint32_t i = 1; i < backtraceSize; ++i)
        {
            const char* symStr = symbols[i];
            if (symStr == NULL || symStr[0] == 0)
            {
                printf("[%d] Unknown symbol \n", i - 1);

                continue;
            }

#ifdef __GNUC__
            const uint32_t startIndex = ILAMBDA(
            {
                const char* slider = symStr;
                while (*slider != 0)
                {
                    if (*slider == '(')
                    {
                        ILRETURN (uint32_t)(slider - symStr);
                    }

                    ++slider;
                }

                ILRETURN uint32_t(-1);
            });
            if (startIndex != uint32_t(-1))
            {
                const uint32_t nextIndex = startIndex + 1;

                const uint32_t endIndex = ILAMBDA(
                {
                    const char* slider = symStr + nextIndex;
                    while (*slider != 0)
                    {
                        if (*slider == '+')
                        {
                            ILRETURN (uint32_t)(slider - symStr);
                        }

                        ++slider;
                    }

                    ILRETURN uint32_t(-1);
                });

                if (endIndex != uint32_t(-1))
                {
                    constexpr uint32_t BufferSize = 2048;

                    const uint32_t size = ILAMBDA(
                    {
                        const uint32_t val = endIndex - nextIndex;
                        if (val > BufferSize - 2)
                        {
                            ILRETURN BufferSize - 2;
                        }

                        ILRETURN val;
                    });

                    if (size > 0)
                    {
                        char buffer[BufferSize];
                        memcpy(buffer, symStr + nextIndex, size);
                        buffer[size] = 0;

                        char* name = abi::__cxa_demangle(buffer, NULL, NULL, NULL);
                        IDEFER(free(name));

                        if (name != NULL && name[0] != 0)
                        {
                            printf("[%d] %s %s \n", i - 1, name, symStr);

                            continue;
                        }

                        printf("[%d] %s %s \n", i - 1, buffer, symStr);

                        continue;
                    }
                }
            }
#endif
            printf("[%d] %s \n", i - 1, symStr);
        }
#endif
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
