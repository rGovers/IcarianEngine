#include "Rendering/UI/Font.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstring>
#include <set>
#include <stb_rect_pack.h>
#include <unordered_map>

#include "Core/Bitfield.h"
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

std::unordered_map<char32_t, CodePointTexture> GetTextures(const stbtt_fontinfo* a_fontInfo, float a_fontSize, const std::u32string_view& a_str)
{
    const float scale = stbtt_ScaleForPixelHeight(a_fontInfo, a_fontSize);

    std::unordered_map<char32_t, CodePointTexture> codePointTextures;

    std::set<char32_t> codePoints;
    for (char32_t codepoint : a_str)
    {
        codePoints.insert(codepoint);
    }

    for (char32_t codepoint : codePoints)
    {
        switch (codepoint)
        {
        case '\n':
        {
            continue;
        }
        default:
        {
            CodePointTexture cpt = { 0 };

            int lsb;
            stbtt_GetCodepointHMetrics(a_fontInfo, (int)codepoint, &cpt.Advance, &lsb);

            int x0;
            int y0;
            int x1;
            int y1;
            stbtt_GetCodepointBitmapBox(a_fontInfo, (int)codepoint, scale, scale, &x0, &y0, &x1, &y1);
            cpt.yOffset = y0;

            cpt.Width = x1 - x0;
            cpt.Height = y1 - y0;
            if (codepoint != ' ')
            {
                const uint32_t size = cpt.Width * cpt.Height;
                cpt.Data = new uint8_t[size];
                memset(cpt.Data, 0, size * sizeof(uint8_t));

                stbtt_MakeCodepointBitmap(a_fontInfo, (unsigned char*)cpt.Data, (int)cpt.Width, (int)cpt.Height, (int)cpt.Width, scale, scale, (int)codepoint);
            }
            
            codePointTextures.emplace(codepoint, cpt);

            break;
        }
        }   
    }

    return codePointTextures;
}

uint8_t* Font::StringToTexture(const std::u32string_view& a_string, float a_fontSize, uint32_t a_width, uint32_t a_height) const
{
    // Changes have negative effects with small strings but alot faster with large strings with alot of repeating characters
    // Worth it as small string are fast enough anyway
    // Anyway the performance rat in me wants to use a chunk allocator but will hold off for now
    const std::unordered_map<char32_t, CodePointTexture> codePointTextures = GetTextures(&m_fontInfo, a_fontSize, a_string);
    IDEFER(
    for (const auto iter : codePointTextures)
    {
        delete[] iter.second.Data;
    });
    
    const float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, a_fontSize);

    int ascent;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, NULL, NULL);

    const uint32_t size = a_width * a_height;
    uint8_t* tex = new uint8_t[size] { 0 };
    
    uint32_t xPos = 0;
    uint32_t yPos = (uint32_t)(ascent * scale);
    for (const char32_t codePoint : a_string)
    {
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
            const CodePointTexture& cpt = codePointTextures.at(codePoint);

            xPos += (uint32_t)(cpt.Advance * scale);

            break;
        }
        default:
        {
            const CodePointTexture& cpt = codePointTextures.at(codePoint);

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

                    tex[cIndex] = (uint8_t)glm::min((uint32_t)UINT8_MAX, cpt.Data[index] + (uint32_t)tex[cIndex]);
                }
            }

            xPos += (uint32_t)(cpt.Advance * scale);

            break;
        }   
        }        
    }

    return tex;
}

constexpr float ISOValue = 10.0f;

// Cannot be evaluated at compile time with the value but can be if using constexpr values therefore constexpr
// Just trying to get in the habit
constexpr static glm::vec2 BlendVertices(const glm::vec2& a_vertA, const glm::vec2& a_vertB, float a_valA, float a_valB)
{
    if (glm::abs(a_valA - a_valB) < glm::epsilon<float>())
    {
        return a_vertA;
    }

    const float lerp = (ISOValue - a_valA) / (a_valB - a_valA);

    return glm::mix(a_vertA, a_vertB, lerp);
}

static uint32_t AddVertex(const glm::vec2& a_vert, Array<glm::vec2>* a_vertices)
{
    const float Distance = 0.0001f;
    const float DistanceSqr = Distance * Distance;

    const uint32_t size = a_vertices->Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (glm::distance2(a_vert, a_vertices->Get(i)) < DistanceSqr)
        {
            return i;
        }
    }

    a_vertices->Push(a_vert);

    return size;
}

static void AddEdge(uint32_t a_lhs, uint32_t a_rhs, std::unordered_map<uint64_t, bool>* a_edgeTable)
{
    const uint64_t edge = (uint64_t)a_lhs << 31 | a_rhs;
    const uint64_t invEdge = (uint64_t)a_rhs << 31 | a_lhs;

    auto iter = a_edgeTable->find(edge);
    if (iter != a_edgeTable->end())
    {
        iter->second = false;
        (*a_edgeTable)[invEdge] = false;

        return;
    }

    a_edgeTable->emplace(edge, true);
    a_edgeTable->emplace(invEdge, true);
}

static uint32_t AddSideVertex(uint32_t a_index, uint32_t a_offset, Array<Vertex>* a_vertices, uint32_t* a_indexMap)
{
    if (a_indexMap[a_index] != -1)
    {
        return a_indexMap[a_index];
    }

    const uint32_t size = a_vertices->Size() - a_offset;
    const uint32_t index = size / 2;

    Vertex vert = a_vertices->Get(a_index);

    vert.Normal = glm::vec3(0.0f);
    a_vertices->Push(vert);

    vert.Position.z = -vert.Position.z;
    a_vertices->Push(vert);

    a_indexMap[a_index] = index;

    return index;
}
static void AddSideEdge(uint32_t a_indexA, uint32_t a_indexB, uint32_t a_sideVertCount, uint32_t* a_indexMap, Array<Vertex>* a_vertices, Array<uint32_t>* a_indices)
{
    const uint32_t qIndexA = AddSideVertex(a_indexA, a_sideVertCount, a_vertices, a_indexMap) * 2 + a_sideVertCount;
    const uint32_t qIndexB = AddSideVertex(a_indexB, a_sideVertCount, a_vertices, a_indexMap) * 2 + a_sideVertCount;
    const uint32_t qIndexC = qIndexB + 1;
    const uint32_t qIndexD = qIndexA + 1;

    Vertex& vertA = a_vertices->Ref(qIndexA);
    Vertex& vertB = a_vertices->Ref(qIndexB);
    Vertex& vertC = a_vertices->Ref(qIndexC);
    Vertex& vertD = a_vertices->Ref(qIndexD);

    const glm::vec3 posA = vertA.Position.xyz();
    const glm::vec3 posB = vertB.Position.xyz();
    const glm::vec3 posC = vertD.Position.xyz();

    const glm::vec3 dirA = glm::normalize(posB - posA);
    const glm::vec3 dirB = glm::normalize(posC - posA);

    const glm::vec3 norm = glm::cross(dirB, dirA);

    vertA.Normal += norm;
    vertB.Normal += norm;
    vertC.Normal += norm;
    vertD.Normal += norm;

    a_indices->Push(qIndexB);
    a_indices->Push(qIndexA);
    a_indices->Push(qIndexD);

    a_indices->Push(qIndexB);
    a_indices->Push(qIndexD);
    a_indices->Push(qIndexC); 
}

void Font::StringToModel(const std::u32string_view& a_string, float a_fontSize, float a_scale, float a_depth, Array<Vertex>* a_vertices, Array<uint32_t>* a_indices, float* a_radius) const
{
    constexpr uint32_t EdgeTable[] =
    {
        // 0
        0b00000000,
        // 1
        0b10001001,
        // 2
        0b01001100,
        // 3
        0b11000101,
        // 4
        0b00100110,
        // 5
        0b10101111,
        // 6
        0b01101010,
        // 7
        0b11100011,
        // 8
        0b00010011,
        // 9
        0b10011010,
        // 10
        0b01011111,
        // 11
        0b11010110,
        // 12
        0b00110101,
        // 13
        0b10111100,
        // 14
        0b01111001,
        // 15
        0b11110000
    };

    constexpr uint32_t TriTable[] =
    {
        // A                                B                                   C                                   D
        // 0
        UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, 
        // 1
        0,          4,          7,          UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 2
        1,          5,          4,          UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 3
        0,          5,          7,          1,          5,          0,          UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 4
        2,          6,          5,          UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 5
        0,          4,          7,          4,          5,          7,          7,          6,          5,          5,          2,          6,
        // 6
        1,          6,          4,          1,          2,          6,          UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 7
        0,          1,          7,          1,          6,          7,          1,          2,          6,          UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 8
        3,          7,          6,          UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 9
        0,          4,          6,          0,          6,          3,          UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 10
        1,          5,          4,          4,          5,          7,          7,          4,          6,          7,          6,          3,
        // 11
        0,          1,          5,          0,          5,          6,          0,          6,          3,          UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 12
        2,          7,          5,          2,          3,          7,          UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 13
        0,          4,          3,          3,          4,          5,          3,          5,          2,          UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 14
        1,          2,          4,          2,          7,          4,          2,          3,          7,          UINT32_MAX, UINT32_MAX, UINT32_MAX,
        // 15
        0,          1,          3,          1,          2,          3,          UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX
    };

    struct CodepointModel
    {
        float yOffset;
        float Advance;
        Array<glm::vec2> Vertices;
    };

    std::unordered_map<char32_t, CodepointModel> codePointModels;

    {
        const std::unordered_map<char32_t, CodePointTexture> codePointTextures = GetTextures(&m_fontInfo, a_fontSize, a_string);

        // Not the most effectient probably should not be rasterizing it but already had code around for rasterized fonts
        for (const auto iter : codePointTextures)
        {
            const CodePointTexture& tex = iter.second;
            IDEFER(delete[] tex.Data);

            CodepointModel model;
            model.Advance = tex.Advance;
            model.yOffset = tex.yOffset;

            switch (iter.first)
            {
            case ' ':
            case '\n':
            {
                break;
            }
            default:
            {
                const uint32_t sX = tex.Width - 1;
                const uint32_t sY = tex.Height - 1;

                for (uint32_t y = 0; y < sY; ++y)
                {
                    const uint32_t y0 = y + 0;
                    const uint32_t y1 = y + 1;

                    const uint32_t yValA = y0 * tex.Width;
                    const uint32_t yValB = y1 * tex.Width;

                    for (uint32_t x = 0; x < sX; ++x)
                    {
                        const uint32_t x0 = x + 0;
                        const uint32_t x1 = x + 1;

                        const float llD = (float)tex.Data[yValA + x0];
                        const float lrD = (float)tex.Data[yValA + x1];
                        const float ulD = (float)tex.Data[yValB + x0];
                        const float urD = (float)tex.Data[yValB + x1];

                        uint8_t bitSet = 0;

                        if (llD > ISOValue)
                        {
                            ISETBIT(bitSet, 0);
                        }
                        if (lrD > ISOValue)
                        {
                            ISETBIT(bitSet, 1);
                        }
                        if (urD > ISOValue)
                        {
                            ISETBIT(bitSet, 2);
                        }
                        if (ulD > ISOValue)
                        {
                            ISETBIT(bitSet, 3);
                        }

                        glm::vec2 vertTable[8];
                        const uint32_t edgeSet = EdgeTable[bitSet];

                        if (IISBITSET(edgeSet, 7))
                        {
                            vertTable[0] = glm::vec2(x0, y0);
                        }
                        if (IISBITSET(edgeSet, 6))
                        {
                            vertTable[1] = glm::vec2(x1, y0);
                        }
                        if (IISBITSET(edgeSet, 5))
                        {
                            vertTable[2] = glm::vec2(x1, y1);
                        }
                        if (IISBITSET(edgeSet, 4))
                        {
                            vertTable[3] = glm::vec2(x0, y1);
                        }

                        if (IISBITSET(edgeSet, 3))
                        {
                            vertTable[4] = BlendVertices(glm::vec2(x0, y0), glm::vec2(x1, y0), llD, lrD);
                        }
                        if (IISBITSET(edgeSet, 2))
                        {
                            vertTable[5] = BlendVertices(glm::vec2(x1, y0), glm::vec2(x1, y1), lrD, urD);
                        }
                        if (IISBITSET(edgeSet, 1))
                        {
                            vertTable[6] = BlendVertices(glm::vec2(x1, y1), glm::vec2(x0, y1), urD, ulD);
                        }
                        if (IISBITSET(edgeSet, 0))
                        {
                            vertTable[7] = BlendVertices(glm::vec2(x0, y1), glm::vec2(x0, y0), ulD, llD);
                        }

                        const uint32_t triTableOffset = bitSet * 12;
                        for (uint32_t i = 0; i < 12 && TriTable[triTableOffset + i] != -1; i += 3)
                        {
                            model.Vertices.Push(vertTable[TriTable[triTableOffset + i + 0]]);
                            model.Vertices.Push(vertTable[TriTable[triTableOffset + i + 1]]);
                            model.Vertices.Push(vertTable[TriTable[triTableOffset + i + 2]]);
                        }
                    }
                }

                break;
            }
            }

            codePointModels.emplace(iter.first, model);
        }
    }

    const float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, a_fontSize);

    int ascent;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, NULL, NULL);

    Array<glm::vec2> stringVertices;
    Array<uint32_t> stringIndices;
    std::unordered_map<uint64_t, bool> uniqueEdge;

    stringIndices.Reserve(1024);
    stringVertices.Reserve(1024);

    const float newLine = ascent * scale;

    glm::vec2 pos = glm::vec2(0, newLine);
    for (const char32_t codePoint : a_string)
    {
        switch (codePoint) 
        {
        case '\n':
        {
            pos.x = 0;
            pos.y += newLine;

            break;
        }
        case ' ':
        {
            const CodepointModel& model = codePointModels.at(codePoint);

            pos.x += model.Advance * scale;

            break;
        }
        default:
        {
            const CodepointModel& model = codePointModels.at(codePoint);

            const glm::vec2 offset = glm::vec2(0.0f, model.yOffset);

            const uint32_t triCount = model.Vertices.Size() / 3;
            for (uint32_t i = 0; i < triCount; ++i)
            {
                const glm::vec2 vertA = (model.Vertices[i * 3 + 0] + offset + pos) * a_scale;
                const glm::vec2 vertB = (model.Vertices[i * 3 + 1] + offset + pos) * a_scale;
                const glm::vec2 vertC = (model.Vertices[i * 3 + 2] + offset + pos) * a_scale;

                const uint32_t indexA = AddVertex(vertA, &stringVertices);
                const uint32_t indexB = AddVertex(vertB, &stringVertices);
                const uint32_t indexC = AddVertex(vertC, &stringVertices);

                AddEdge(indexA, indexB, &uniqueEdge);
                AddEdge(indexB, indexC, &uniqueEdge);
                AddEdge(indexC, indexA, &uniqueEdge);

                stringIndices.Push(indexA);
                stringIndices.Push(indexB);
                stringIndices.Push(indexC);
            }

            pos.x += model.Advance * scale;

            break;
        }
        }
    }

    *a_radius = (pos.length() + newLine) * a_scale;

    const uint32_t stringVertCount = stringVertices.Size();
    const uint32_t sideVertCount = stringVertCount * 2;
    a_vertices->Resize(sideVertCount);

    const float halfDepth = a_depth * 0.5f;
    for (uint32_t i = 0; i < stringVertCount; ++i)
    {
        const glm::vec2& p = stringVertices[i];

        const Vertex v = Vertex
        (
            glm::vec4(p.x, p.y, halfDepth, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        const Vertex invV = Vertex
        (
            glm::vec4(p.x, p.y, -halfDepth, 1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f)
        );

        a_vertices->Set(i, v);
        a_vertices->Set(stringVertCount + i, invV);
    }

    const uint32_t indexCount = stringIndices.Size();
    const uint32_t sideIndexCount = indexCount * 2;
    
    uint32_t* indexMap = new uint32_t[indexCount];
    IDEFER(delete[] indexMap);
    memset(indexMap, -1, indexCount * sizeof(uint32_t));

    a_indices->Reserve(sideIndexCount);

    const uint32_t stringTriCount = indexCount / 3;
    for (uint32_t i = 0; i < stringTriCount; ++i)
    {
        const uint32_t indexA = stringIndices[i * 3 + 0];
        const uint32_t indexB = stringIndices[i * 3 + 1];
        const uint32_t indexC = stringIndices[i * 3 + 2];

        a_indices->Push(indexA);
        a_indices->Push(indexB);
        a_indices->Push(indexC);

        const uint32_t invIndexA = indexA + stringVertCount;
        const uint32_t invIndexB = indexB + stringVertCount;
        const uint32_t invIndexC = indexC + stringVertCount;

        a_indices->Push(invIndexC);
        a_indices->Push(invIndexB);
        a_indices->Push(invIndexA);

        const uint64_t edgeA = (uint64_t)indexA << 31 | indexB;
        if (uniqueEdge.at(edgeA))
        {
            AddSideEdge(indexA, indexB, sideVertCount, indexMap, a_vertices, a_indices);
        }

        const uint64_t edgeB = (uint64_t)indexB << 31 | indexC;
        if (uniqueEdge.at(edgeB))
        {
            AddSideEdge(indexB, indexC, sideVertCount, indexMap, a_vertices, a_indices);
        }

        const uint64_t edgeC = (uint64_t)indexC << 31 | indexA;
        if (uniqueEdge.at(edgeC))
        {
            AddSideEdge(indexC, indexA, sideVertCount, indexMap, a_vertices, a_indices);
        }
    }

    const uint32_t vertCount = a_vertices->Size();
    for (uint32_t i = sideVertCount; i < vertCount; ++i)
    {
        Vertex& vert = a_vertices->Ref(i);

        vert.Normal = glm::normalize(vert.Normal);
    }
}