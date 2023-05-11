#pragma once

#include "DataTypes/TArray.h"

#include <string>

class Font;
class RuntimeManager;

class AssetLibrary
{
private:
    TArray<Font*> m_fonts;

    AssetLibrary();
protected:

public:
    ~AssetLibrary();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    static uint32_t GenerateFont(const std::string_view& a_path);
    static Font* GetFont(uint32_t a_addr);
    static void DestroyFont(uint32_t a_addr);
};