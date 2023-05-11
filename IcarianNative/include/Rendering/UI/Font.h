#pragma once

#include <stb_truetype.h>
#include <string>

class Font
{
private:
    stbtt_fontinfo m_fontInfo;

protected:

public:
    Font(const std::string_view& a_path);
    ~Font();

    static void Init();
};