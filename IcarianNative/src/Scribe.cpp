// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Scribe.h"

#include <codecvt>
#include <locale>

#include "Core/IcarianDefer.h"
#include "Runtime/RuntimeManager.h"

Scribe* Scribe::Instance = nullptr;

static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;

RUNTIME_FUNCTION(void, Scribe, SetInternalLanguage, 
{
    char* language = mono_string_to_utf8(a_language);
    IDEFER(mono_free(language));

    Scribe::SetCurrentLanguage(language);
}, MonoString* a_language)
RUNTIME_FUNCTION(MonoString*, Scribe, GetInternalLanguage,
{
    return mono_string_new_wrapper(Scribe::GetCurrentLanguage().c_str());
})

RUNTIME_FUNCTION(uint32_t, Scribe, GetFontAddr, 
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));

    return Scribe::GetFont(key);
}, MonoString* a_key);
RUNTIME_FUNCTION(MonoString*, Scribe, GetString,
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));

    const std::u32string str = Scribe::GetString(key);
    if (!str.empty())
    {
        return mono_string_from_utf32((mono_unichar4*)str.c_str());
    }

    return NULL;
}, MonoString* a_key)
RUNTIME_FUNCTION(MonoString*, Scribe, GetStringFormated, 
{
    const uintptr_t size = mono_array_length(a_args);

    char32_t** args = nullptr;
    if (size > 0)
    {
        args = new char32_t*[size];
        for (uintptr_t i = 0; i < size; ++i)
        {
            args[i] = (char32_t*)mono_string_to_utf32(mono_array_get(a_args, MonoString*, i));
        }
    }

    IDEFER(
    if (args != nullptr)
    {
        for (uint32_t i = 0; i < size; ++i)
        {
            mono_free(args[i]);
        }

        delete[] args;
    });

    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));

    const std::u32string str = Scribe::GetStringFormated(key, args, (uint32_t)size);

    if (!str.empty())
    {
        return mono_string_from_utf32((mono_unichar4*)str.c_str());
    }

    return NULL;
}, MonoString* a_key, MonoArray* a_args)

RUNTIME_FUNCTION(void, Scribe, SetFont,
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));

    Scribe::SetFont(key, a_value);
}, MonoString* a_key, uint32_t a_value)
RUNTIME_FUNCTION(void, Scribe, SetString,
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));
    char32_t* value = (char32_t*)mono_string_to_utf32(a_value);
    IDEFER(mono_free(value));

    Scribe::SetString(key, value);
}, MonoString* a_key, MonoString* a_value)

RUNTIME_FUNCTION(uint32_t, Scribe, Exists,
{
    char* key = mono_string_to_utf8(a_key);
    IDEFER(mono_free(key));

    return (uint32_t)Scribe::KeyExists(key);
}, MonoString* a_key)

Scribe::Scribe()
{
    
}
Scribe::~Scribe()
{

}

void Scribe::Init()
{
    if (Instance == nullptr)
    {
        Instance = new Scribe();

        BIND_FUNCTION(IcarianEngine, Scribe, SetInternalLanguage);
        BIND_FUNCTION(IcarianEngine, Scribe, GetInternalLanguage);

        BIND_FUNCTION(IcarianEngine, Scribe, GetFontAddr);
        BIND_FUNCTION(IcarianEngine, Scribe, GetString);
        BIND_FUNCTION(IcarianEngine, Scribe, GetStringFormated);

        BIND_FUNCTION(IcarianEngine, Scribe, SetFont);
        BIND_FUNCTION(IcarianEngine, Scribe, SetString);

        BIND_FUNCTION(IcarianEngine, Scribe, Exists);
    }
}
void Scribe::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

std::string Scribe::GetCurrentLanguage()
{
    IVERIFY(Instance != nullptr);

    const SharedThreadGuard g = SharedThreadGuard(Instance->m_lock);

    return Instance->m_curLang;
}
void Scribe::SetCurrentLanguage(const std::string_view& a_language)
{
    IVERIFY(Instance != nullptr);

    const ThreadGuard g = ThreadGuard(Instance->m_lock);

    Instance->m_curLang = std::string(a_language);
}

uint32_t Scribe::GetFont(const std::string_view& a_key)
{
    IVERIFY(Instance != nullptr);

    const std::string key = std::string(a_key);
    if (Instance->m_fonts.Exists(key))
    {
        return Instance->m_fonts[key];
    }

    return -1;
}

std::u32string Scribe::GetString(const std::string_view& a_key)
{
    IVERIFY(Instance != nullptr);
    
    const std::string key = std::string(a_key);
    if (Instance->m_strings.Exists(key))
    {
        return Instance->m_strings[key];
    }

    return converter.from_bytes(key);
}
std::u32string Scribe::GetStringFormated(const std::string_view& a_key, const char32_t* const* a_args, uint32_t a_count)
{
    std::u32string str = GetString(a_key);

    if (str.empty())
    {
        return str;
    }

    for (uint32_t i = 0; i < a_count; ++i)
    {
        const std::u32string iStr = converter.from_bytes("{" + std::to_string(i) + "}");
        const size_t size = iStr.size();

        while (true)
        {
            const std::size_t pos = str.find(iStr);
            if (pos == std::u32string::npos)
            {
                break;
            }

            str.replace(pos, size, a_args[i]);
        }   
    }

    return str;
}
std::u32string Scribe::GetStringFormated(const std::string_view& a_key, const std::u32string* a_args, uint32_t a_count)
{
    std::u32string str = GetString(a_key);

    if (str.empty())
    {
        return str;
    }

    for (uint32_t i = 0; i < a_count; ++i)
    {
        const std::u32string iStr = converter.from_bytes("{" + std::to_string(i) + "}");
        const size_t size = iStr.size();

        while (true)
        {
            const std::size_t pos = str.find(iStr);
            if (pos == std::u32string::npos)
            {
                break;
            }

            str.replace(pos, size, a_args[i]);
        }   
    }

    return str;
}

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