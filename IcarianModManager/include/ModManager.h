#pragma once

#include <cstdint>
#include <vector>

class ModInfo;

class ModManager
{
private:
    uint32_t              m_selectedMod;
    std::vector<ModInfo*> m_info;
    std::vector<uint32_t> m_loaded;
    std::vector<uint32_t> m_unloaded;

protected:

public:
    ModManager();
    ~ModManager();

    void Update();
};