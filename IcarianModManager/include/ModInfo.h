#pragma once

#include <filesystem>
#include <string>

class ModInfo
{
private:
    std::string m_name;
    std::string m_id;
    std::string m_description;

protected:

public:
    ModInfo(); 
    ~ModInfo();

    inline std::string_view GetName() const
    {
        return m_name;
    }
    inline std::string_view GetID() const
    {
        return m_id;
    }
    inline std::string_view GetDescription() const
    {
        return m_description;
    }

    static ModInfo* LoadMod(const std::filesystem::path& a_path);
};