// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <string>

#include "DataTypes/SpinLock.h"
#include "DataTypes/TUMap.h"
#include "IcarianError.h"

class Scribe
{
private:
    static Scribe* Instance;

    SharedSpinLock                     m_lock;
    std::string                        m_curLang;
    TUMap<std::string, std::u32string> m_strings;
    TUMap<std::string, uint32_t>       m_fonts;

    Scribe();

protected:

public:
    ~Scribe();

    static void Init();
    static void Destroy();

    static std::string GetCurrentLanguage();
    static void SetCurrentLanguage(const std::string_view& a_language);

    inline static bool KeyExists(const std::string_view& a_key)
    {
        IVERIFY(Instance != nullptr);

        return Instance->m_strings.Exists(std::string(a_key));
    }

    inline static void SetFont(const std::string_view& a_key, uint32_t a_addr)
    {
        IVERIFY(Instance != nullptr);

        Instance->m_fonts.Push(std::string(a_key), a_addr);
    }
    inline static void SetString(const std::string_view& a_key, const std::u32string_view& a_string)
    {
        IVERIFY(Instance != nullptr);

        Instance->m_strings.Push(std::string(a_key), std::u32string(a_string));
    }

    static uint32_t GetFont(const std::string_view& a_key);

    static std::u32string GetString(const std::string_view& a_key);
    static std::u32string GetStringFormated(const std::string_view& a_key, const char32_t* const* a_args, uint32_t a_count);
    static std::u32string GetStringFormated(const std::string_view& a_key, const std::u32string* a_args, uint32_t a_count);
};

// MIT License
// 
// Copyright (c) 2024 River Govers
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