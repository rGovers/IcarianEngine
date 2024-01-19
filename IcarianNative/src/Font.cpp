#include "Rendering/UI/Font.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstring>
#include <fstream>
#include <limits>
#include <set>
#include <stb_rect_pack.h>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
#include "Trace.h"

Font::Font(unsigned char* a_data)
{
    m_data = a_data;

    const int offset = stbtt_GetFontOffsetForIndex(a_data, 0);
    ICARIAN_ASSERT_R(stbtt_InitFont(&m_fontInfo, m_data, offset) != 0);
}
Font::~Font()
{
    delete[] m_data;
}

Font* Font::LoadFont(const std::filesystem::path& a_path)
{
    TRACE("Loading font");

    std::ifstream file = std::ifstream(a_path, std::ios::binary);
    if (file.good() && file.is_open())
    {
        // Fuck Windows
        // const uint32_t size = (uint32_t)std::filesystem::file_size(a_path);
        file.ignore(std::numeric_limits<std::streamsize>::max());
        const std::streamsize size = file.gcount();
        file.clear();
        file.seekg(0, std::ios::beg);

        ICARIAN_ASSERT_R(size != 0);

        unsigned char* dat = new unsigned char[size];
        file.read((char*)dat, (std::streamsize)size);

        return new Font(dat);
    }
    else
    {
        ICARIAN_ASSERT_MSG_R(0, "Unable to open font file");
    }

    return nullptr;
}

struct CodePointTexture
{
    uint32_t Width;
    uint32_t Height;
    int Advance;
    int yOffset;
    unsigned char* Data;
};

unsigned char* Font::StringToTexture(const std::u32string_view& a_string, float a_size, uint32_t a_width, uint32_t a_height) const
{
    // Changes have negative effects with small strings but alot faster with large strings with alot of repeating characters
    // Worth it as small string are fast enough anyway
    // Anyway the performance rat in me wants to use a chunk allocator but will hold off for now
    const uint32_t len = a_string.size();

    const float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, a_size);

    int ascent;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, NULL, NULL);
    
    std::set<int> codePoints;
    for (uint32_t i = 0; i < len; ++i)
    {
        codePoints.insert((int)a_string[i]);
    }

    std::unordered_map<int, CodePointTexture> codePointTextures;
    for (int codepoint : codePoints)
    {
        if (codepoint == '\n')
        {
            continue;
        }

        CodePointTexture cpt;

        int lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, codepoint, &cpt.Advance, &lsb);

        int x0;
        int y0;
        int x1;
        int y1;
        stbtt_GetCodepointBitmapBox(&m_fontInfo, codepoint, scale, scale, &x0, &y0, &x1, &y1);
        cpt.yOffset = y0;

        cpt.Width = x1 - x0;
        cpt.Height = y1 - y0;
        const uint32_t size = cpt.Width * cpt.Height;
        cpt.Data = new unsigned char[size];
        memset(cpt.Data, 0, size);
        stbtt_MakeCodepointBitmap(&m_fontInfo, cpt.Data, cpt.Width, cpt.Height, cpt.Width, scale, scale, codepoint);

        codePointTextures.emplace(codepoint, cpt);
    }

    const uint32_t size = a_width * a_height;

    unsigned char* tex = new unsigned char[size];
    memset(tex, 0, size);
    
    uint32_t xPos = 0;
    uint32_t yPos = (uint32_t)(ascent * scale);

    for (uint32_t i = 0; i < len; ++i)
    {
        const int codePoint = (int)a_string[i];
        
        if (codePoint == '\n')
        {
            xPos = 0;
            yPos += (unsigned int)(ascent * scale);

            continue;
        }

        const CodePointTexture& cpt = codePointTextures[codePoint];

        for (uint32_t y = 0; y < cpt.Height; ++y)
        {
            const uint32_t cYPos = (uint32_t)(y + yPos + cpt.yOffset);
            if (cYPos >= a_height)
            {
                break;
            }

            for (uint32_t x = 0; x < cpt.Width; ++x)
            {
                const uint32_t cXPos = x + xPos;
                if (cXPos >= a_width)
                {
                    break;
                }

                const uint32_t index = x + (y * cpt.Width);
                const uint32_t cIndex = cXPos + (cYPos * a_width);

                tex[cIndex] = (unsigned char)glm::min(255U, cpt.Data[index] + (unsigned int)tex[cIndex]);
            }
        }

        xPos += (uint32_t)(cpt.Advance * scale);
    }

    for (auto iter : codePointTextures)
    {
        delete[] iter.second.Data;
    }

    return tex;
}