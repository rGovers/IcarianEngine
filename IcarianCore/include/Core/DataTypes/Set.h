// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

#include "Core/DataTypes/Allocators/Allocator.h"
#include "Core/DataTypes/LesserFunctions.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"

namespace IcarianCore
{
    template<typename T, typename Lesser = DefaultLesserFunction<T>>
    class Set
    {
    private:
        Allocator* m_allocator;

        T*         m_data;
        uint32_t   m_size;
        uint32_t   m_capacity;

        inline void DestroyData()
        {
            if constexpr (!std::is_trivially_destructible<T>())
            {
                for (uint32_t i = 0; i < m_size; ++i)
                {
                    T* dat = &(m_data[i]);

                    dat->~T();
                }
            }

            memset((void*)m_data, 0, m_capacity * sizeof(T));
        }

    protected:

    public:
        typedef T* iterator;
        typedef const T* const_iterator;

        Set(Allocator* a_allocator)
        {
            m_allocator = a_allocator;

            m_size = 0;
            m_capacity = 1;
            m_data = m_allocator->ZTAllocate<T>(m_capacity);
        }
        Set(const Set& a_other) : Set(a_other, a_other.m_allocator) { }
        Set(const Set& a_other, Allocator* a_allocator)
        {
            m_allocator = a_allocator;

            m_size = a_other.m_size;
            m_capacity = a_other.m_size;
            if (m_capacity < 1)
            {
                m_capacity = 1;
            }

            m_data = m_allocator->ZTAllocate<T>(m_capacity);
            for (uint32_t i = 0; i < m_size; ++i)
            {
                m_data[i] = a_other.m_data[i];
            }
        }
        ~Set()
        {
            if (m_data == nullptr)
            {
                return;
            }

            DestroyData();

            m_allocator->Free(m_data);
        }

        inline Set& operator =(const Set& a_other)
        {
            if (m_data != nullptr && m_allocator != nullptr)
            {
                DestroyData();

                m_allocator->Free(m_data);
            }

            m_allocator = a_other.m_allocator;
            m_size = a_other.m_size;
            m_capacity = a_other.m_size;

            if (m_capacity < 1)
            {
                m_capacity = 1;
            }

            m_data = m_allocator->ZTAllocate<T>(m_capacity);
            for (uint32_t i = 0; i < m_size; ++i)
            {
                m_data[i] = a_other.m_data[i];
            }

            return *this;
        }

        inline iterator begin()
        {
            return m_data;
        }
        inline const_iterator begin() const
        {
            return m_data;
        }

        inline iterator end()
        {
            return m_data + m_size;
        }
        inline const_iterator end() const
        {
            return m_data + m_size;
        }

        inline Allocator* GetAllocator() const
        {
            return m_allocator;
        }

        inline uint32_t Size() const
        {
            return m_size;
        }
        inline bool Empty() const
        {
            return m_size == 0;
        }

        void Clear()
        {
            if (m_data != nullptr)
            {
                DestroyData();
            }

            m_size = 0;
        }

        void Reserve(uint32_t a_size)
        {
            if (a_size > m_capacity)
            {
                IDEFER(m_capacity = a_size);

                T* newData = m_allocator->ZTAllocate<T>(a_size);
                memcpy((void*)newData, m_data, m_capacity * sizeof(T));
                m_allocator->Free(m_data);
                m_data = newData;
            }
        }

        void Push(const T& a_value)
        {
            const uint32_t index = ILAMBDA(
            {
                if (m_size <= 0)
                {
                    ILRETURN uint32_t(0);
                }

                uint32_t lhsIndex = 0;
                uint32_t rhsIndex = m_size - 1;
                while (lhsIndex != rhsIndex)
                {
                    const uint32_t diff = rhsIndex - lhsIndex;
                    const uint32_t halfSize = diff / 2;
                    const uint32_t midPoint = lhsIndex + halfSize;

                    const T& val = m_data[midPoint];
                    if (val == a_value)
                    {
                        ILRETURN uint32_t(-1);
                    }

                    if (diff == 1)
                    {
                        if (m_data[rhsIndex] == a_value)
                        {
                            ILRETURN uint32_t(-1);
                        }

                        ILRETURN rhsIndex;
                    }

                    if (Lesser::Less(val, a_value))
                    {
                        rhsIndex -= halfSize;
                    }
                    else
                    {
                        lhsIndex += halfSize;
                    }
                }

                const T& val = m_data[lhsIndex];
                if (val == a_value)
                {
                    ILRETURN uint32_t(-1);
                }

                if (Lesser::Less(val, a_value))
                {
                    ILRETURN lhsIndex + 1;
                }

                ILRETURN lhsIndex;
            });

            if (index == uint32_t(-1))
            {
                return;
            }

            const uint32_t newSize = m_size + 1;
            IDEFER(m_size = newSize);

            if (newSize > m_capacity)
            {
                Reserve(m_capacity << 1);
            }

            memmove((void*)(m_data + index + 1), (void*)(m_data + index), (m_size - index) * sizeof(T));
            // RAII does weird stuff so use the emplacement new might have to switch the other data types to use the emplacement new
            // To be fair we are kinda doing undefined things memory wise as C++ assumes objects and data are the same sooo.....
            // I guess not that surprising we kinda break RAII
            //
            // Yes there is move semantics but in my experience the existence of a constructor can stop optimizations as
            // the compiler can no longer treat an object as pure data so funny enough a move constructor can stop a move
            // Yes compilers have gotten good at inspecting the constructors to see if you are just assigning fields
            // but do we want to be at the mercy of the compiler when you can just use list intializers so you and the compiler see the same thing
            // C-Style structs are your friend compilers love them as they make their life easy
            // I think compiler developers might need a hug compilers are hard
            new (m_data + index) T(a_value);
        }

        bool Exists(const T& a_value)
        {
            if (m_size <= 0)
            {
                return false;
            }

            uint32_t lhsIndex = 0;
            uint32_t rhsIndex = m_size - 1;
            while (lhsIndex != rhsIndex)
            {
                const uint32_t diff = rhsIndex - lhsIndex;
                const uint32_t halfSize = diff / 2;
                const uint32_t midPoint = lhsIndex + halfSize;

                const T& val = m_data[midPoint];
                if (val == a_value)
                {
                    return true;
                }

                if (diff == 1)
                {
                    if (m_data[rhsIndex] == a_value)
                    {
                        return true;
                    }

                    return false;
                }

                if (Lesser::Less(val, a_value))
                {
                    rhsIndex -= halfSize;
                }
                else
                {
                    lhsIndex += halfSize;
                }
            }

            return m_data[lhsIndex] == a_value;
        }
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
