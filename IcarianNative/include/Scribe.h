#pragma once

#include <cstdint>
#include <string>

#include "DataTypes/TUMap.h"
#include "Core/IcarianAssert.h"

class Scribe
{
private:
    static Scribe* Instance;

    std::shared_mutex                  m_mutex;
    std::string                        m_curLang;
    TUMap<std::string, std::u32string> m_strings;
    TUMap<std::string, uint32_t>       m_fonts;

    Scribe();

protected:

public:
    ~Scribe();

    static void Init();
    static void Destroy();

    inline static std::string GetCurrentLanguage()
    {
        ICARIAN_ASSERT(Instance != nullptr);

        const std::shared_lock g = std::shared_lock(Instance->m_mutex);

        return Instance->m_curLang;
    }
    inline static void SetCurrentLanguage(const std::string_view& a_language)
    {
        ICARIAN_ASSERT(Instance != nullptr);

        const std::unique_lock g = std::unique_lock(Instance->m_mutex);

        Instance->m_curLang = std::string(a_language);
    }

    inline static bool KeyExists(const std::string_view& a_key)
    {
        return Instance->m_strings.Exists(std::string(a_key));
    }

    inline static void SetFont(const std::string_view& a_key, uint32_t a_addr)
    {
        ICARIAN_ASSERT(Instance != nullptr);

        Instance->m_fonts.Push(std::string(a_key), a_addr);
    }
    inline static void SetString(const std::string_view& a_key, const std::u32string_view& a_string)
    {
        ICARIAN_ASSERT(Instance != nullptr);

        Instance->m_strings.Push(std::string(a_key), std::u32string(a_string));
    }

    static uint32_t GetFont(const std::string_view& a_key);

    static std::u32string GetString(const std::string_view& a_key);
    static std::u32string GetStringFormated(const std::string_view& a_key, char32_t* const* a_args, uint32_t a_count);
    static std::u32string GetStringFormated(const std::string_view& a_key, const std::u32string* a_args, uint32_t a_count);
};