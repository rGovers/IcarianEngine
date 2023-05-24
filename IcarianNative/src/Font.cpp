#include "Rendering/UI/Font.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstring>
#include <fstream>
#include <limits>
#include <stb_rect_pack.h>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
#include "Trace.h"

Font::Font(const unsigned char* a_data)
{
     const int offset = stbtt_GetFontOffsetForIndex(a_data, 0);
     ICARIAN_ASSERT_R(stbtt_InitFont(&m_fontInfo, a_data, offset) != 0);
}
Font::~Font()
{
       
}

Font* Font::LoadFont(const std::filesystem::path& a_path)
{
    TRACE("Loading font");

    std::ifstream file = std::ifstream(a_path, std::ios::binary);
    if (file.good() && file.is_open())
    {
        ICARIAN_DEFER_closeIFile(file);

        file.ignore(std::numeric_limits<std::streamsize>::max());
        const std::streamsize size = file.gcount();
        file.clear();
        file.seekg(0, std::ios::beg);

        ICARIAN_ASSERT_R(size != 0);

        unsigned char* dat = new unsigned char[size];
        ICARIAN_DEFER_delA(dat);
        file.read((char*)dat, size);

        return new Font(dat);
    }
    else
    {
        ICARIAN_ASSERT_MSG_R(0, "Unable to open font file");
    }

    return nullptr;
}

unsigned char* Font::StringToTexture(const std::u32string_view& a_string, float a_size, uint32_t a_width, uint32_t a_height) const
{
    const uint32_t len = a_string.size();

    const uint32_t size = a_width * a_height;

    unsigned char* tex = new unsigned char[size];
    memset(tex, 0, size);
    const float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, a_size);

    int ascent;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, NULL, NULL);

    unsigned int xPos = 0;
    unsigned int yPos = (unsigned int)(ascent * scale);

    // TODO: Improve down the line wasting time with allocation
    for (uint32_t i = 0; i < len; ++i)
    {
        const int codePoint = (int)a_string[i];
        
        if (codePoint == '\n')
        {
            xPos = 0;
            yPos += (unsigned int)(ascent * scale);

            continue;
        }

        int advance;
        int lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, codePoint, &advance, &lsb);

        int x0;
        int y0;
        int x1;
        int y1;
        stbtt_GetCodepointBitmapBox(&m_fontInfo, codePoint, scale, scale, &x0, &y0, &x1, &y1);

        const unsigned int gWidth = x1 - x0;
        const unsigned int gHeight = y1 - y0;
        const unsigned int gSize = gWidth * gHeight;
        unsigned char* data = new unsigned char[gSize];
        ICARIAN_DEFER_delA(data);
        memset(data, 0, gSize);
        stbtt_MakeCodepointBitmap(&m_fontInfo, data, gWidth, gHeight, gWidth, scale, scale, codePoint);

        for (unsigned int y = 0; y < gHeight; ++y)
        {
            const unsigned int cYPos = (unsigned int)(y + yPos + y0);
            if (cYPos >= a_height)
            {
                break;
            }

            for (unsigned int x = 0; x < gWidth; ++x)
            {
                const unsigned int cXPos = x + xPos;
                if (cXPos >= a_width)
                {
                    break;
                }

                const unsigned int index = x + (y * gWidth);
                const unsigned int cIndex = cXPos + (cYPos * a_width);

                tex[cIndex] = (unsigned char)glm::min(255U, data[index] + (unsigned int)tex[cIndex]);
            }
        }

        xPos += (unsigned int)(advance * scale);
    }

    return tex;
}