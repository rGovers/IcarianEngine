#pragma once

#include <filesystem>
#include <stb_truetype.h>

class Font
{
private:
    stbtt_fontinfo m_fontInfo;
    // Forgot that stbb_fontinfo does not own the data or copy it, so we need to keep it around
    unsigned char* m_data;

protected:

public:
    Font(unsigned char* a_data);
    ~Font();

    static Font* LoadFont(const std::filesystem::path& a_path);

    unsigned char* StringToTexture(const std::u32string_view& a_string, float a_size, uint32_t a_width, uint32_t a_height) const;
};