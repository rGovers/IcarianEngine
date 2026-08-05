// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>
#include <thread>
#include <type_traits>

namespace IcarianCore
{
    template <typename T, typename = void>
    struct DefaultHashFunctionImpl { };

    struct HashFuncImpl
    {
        constexpr static uint64_t Hash64Shift(uint64_t a_key)
        {
            uint64_t hash = a_key;

            hash = (~hash) + (hash << 21); // key = (key << 21) - key - 1;
            hash = hash ^ (hash >> 24);
            hash = (hash + (hash << 3)) + (hash << 8); // key * 265
            hash = hash ^ (hash >> 14);
            hash = (hash + (hash << 2)) + (hash << 4); // key * 21
            hash = hash ^ (hash >> 28);
            hash = hash + (hash << 31);

            return hash;
        }
    };

    template<typename T>
    struct DefaultHashFunction
    {
        static uint64_t Hash(const T& a_value)
        {
            return DefaultHashFunctionImpl<std::decay_t<T>>::Hash(a_value);
        }
    };

    // Templates still confuse the fuck out of me and give nonsense errors and still just prefer macros
    template <typename, typename = std::void_t<>>
    struct HasHashFunction : std::false_type
    {

    };

    template <typename T>
    struct HasHashFunction<T, std::void_t<std::is_same<decltype(std::declval<const T>().Hash()), uint32_t>>> : std::true_type
    {

    };

    template<typename T>
    struct DefaultHashFunctionImpl<T, std::enable_if_t<HasHashFunction<T>::value>>
    {
        static uint64_t Hash(const T& a_key)
        {
            return a_key.Hash();
        }
    };

    template<>
    struct DefaultHashFunctionImpl<std::thread::id>
    {
        static uint64_t Hash(const std::thread::id& a_id)
        {
            // This is really annoying as the STL gives no assurances about the type
            // It also implements it as a specialization without the hash type argument and uses internal functions
            // Basically to safely hash the thread id I have to either implement my own threads or just use their hash
            // And if size_t does not match uint64_t go fuck myself I guess
            // Just another instance of the STL going here is an interface as to specifics I dunno go figure it out
            // Funny how trying to move away from the STL turns into reinvent the wheel rather then just doing it piece by piece
            static_assert(sizeof(size_t) == sizeof(uint64_t));

            const std::hash<std::thread::id> hasher;

            return (uint64_t)hasher(a_id);
        }
    };

    template<>
    struct DefaultHashFunctionImpl<void*>
    {
        static uint64_t Hash(const void* a_ptr)
        {
            return HashFuncImpl::Hash64Shift((uint64_t)(uintptr_t)a_ptr);
        }
    };

    template<>
    struct DefaultHashFunctionImpl<uint32_t>
    {
        static uint64_t Hash(uint32_t a_value)
        {
            return HashFuncImpl::Hash64Shift((uint64_t)a_value);
        }
    };
    template<>
    struct DefaultHashFunctionImpl<uint64_t>
    {
        static uint64_t Hash(uint64_t a_value)
        {
            return HashFuncImpl::Hash64Shift(a_value);
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
