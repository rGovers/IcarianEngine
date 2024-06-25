#pragma once

#include <cstdint>
#include <filesystem>
#include <stb_truetype.h>

#include "DataTypes/Array.h"

#include "EngineModelInteropStructures.h"

class Font
{
private:
    stbtt_fontinfo m_fontInfo;
    // Forgot that stbb_fontinfo does not own the data or copy it, so we need to keep it around
    uint8_t*       m_data;

protected:

public:
    Font(uint8_t* a_data);
    ~Font();

    static Font* LoadFont(const std::filesystem::path& a_path);

    uint8_t* StringToTexture(const std::u32string_view& a_string, float a_fontSize, uint32_t a_width, uint32_t a_height) const;
    void StringToModel(const std::u32string_view& a_string, float a_fontSize, float a_scale, float a_depth, Array<Vertex>* a_vertices, Array<uint32_t>* a_indices, float* a_radius) const;
};