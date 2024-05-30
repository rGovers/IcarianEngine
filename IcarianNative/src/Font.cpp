#include "Rendering/UI/Font.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstring>
#include <set>
#include <stb_rect_pack.h>
#include <unordered_map>

#include "Core/IcarianDefer.h"
#include "FileCache.h"
#include "IcarianError.h"
#include "Trace.h"

Font::Font(uint8_t* a_data)
{
    m_data = a_data;

    const int offset = stbtt_GetFontOffsetForIndex(a_data, 0);
    if (stbtt_InitFont(&m_fontInfo, m_data, offset) == 0)
    {
        IERROR("Failed to init font");
    }
}
Font::~Font()
{
    delete[] m_data;
}

Font* Font::LoadFont(const std::filesystem::path& a_path)
{
    TRACE("Loading font");
    FileHandle* fileHandle = FileCache::LoadFile(a_path);
    if (fileHandle != nullptr)
    {
        const uint32_t size = (uint32_t)fileHandle->GetSize();

        uint8_t* dat = new uint8_t[size];
        if (fileHandle->Read(dat, size) != size)
        {
            IERROR("Error reading file");
        }

        return new Font(dat);
    }
    else
    {
        IERROR("Unable to open font file");
    }

    return nullptr;
}

struct CodePointTexture
{
    uint32_t Width;
    uint32_t Height;
    int Advance;
    int yOffset;
    uint8_t* Data;
};

uint8_t* Font::StringToTexture(const std::u32string_view& a_string, float a_fontSize, uint32_t a_width, uint32_t a_height) const
{
    // Changes have negative effects with small strings but alot faster with large strings with alot of repeating characters
    // Worth it as small string are fast enough anyway
    // Anyway the performance rat in me wants to use a chunk allocator but will hold off for now
    const uint32_t len = (uint32_t)a_string.size();

    const float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, a_fontSize);

    int ascent;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, NULL, NULL);
    
    std::set<int> codePoints;
    for (uint32_t i = 0; i < len; ++i)
    {
        codePoints.insert((int)a_string[i]);
    }

    std::unordered_map<int, CodePointTexture> codePointTextures;
    IDEFER(
    for (const auto iter : codePointTextures)
    {
        delete[] iter.second.Data;
    });
    for (int codepoint : codePoints)
    {
        switch (codepoint)
        {
        case '\n':
        {
            continue;
        }
        default:
        {
            CodePointTexture cpt = { };

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
            if (codepoint != ' ')
            {
                const uint32_t size = cpt.Width * cpt.Height;
                cpt.Data = new uint8_t[size] { 0 };
                stbtt_MakeCodepointBitmap(&m_fontInfo, (unsigned char*)cpt.Data, (int)cpt.Width, (int)cpt.Height, (int)cpt.Width, scale, scale, codepoint);
            }
            
            codePointTextures.emplace(codepoint, cpt);

            break;
        }
        }   
    }

    const uint32_t size = a_width * a_height;

    uint8_t* tex = new uint8_t[size] { 0 };
    
    uint32_t xPos = 0;
    uint32_t yPos = (uint32_t)(ascent * scale);

    for (uint32_t i = 0; i < len; ++i)
    {
        const int codePoint = (int)a_string[i];
        
        switch (codePoint) 
        {
        case '\n':
        {
            xPos = 0;
            yPos += (uint32_t)(ascent * scale);

            break;
        }
        case ' ':
        {
            const CodePointTexture& cpt = codePointTextures[codePoint];

            xPos += (uint32_t)(cpt.Advance * scale);

            break;
        }
        default:
        {
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

                    tex[cIndex] = (uint8_t)glm::min(255U, cpt.Data[index] + (uint32_t)tex[cIndex]);
                }
            }

            xPos += (uint32_t)(cpt.Advance * scale);

            break;
        }   
        }        
    }

    return tex;
}