#pragma once

#include <filesystem>
#include <stb_truetype.h>

class Font
{
private:
    stbtt_fontinfo m_fontInfo;

protected:

public:
    Font(const unsigned char* a_data);
    ~Font();

    static Font* LoadFont(const std::filesystem::path& a_path);

    unsigned char* StringToTexture(const std::u32string_view& a_string, float a_size, uint32_t a_width, uint32_t a_height) const;
};