#include "Core/OBJLoader.h"

#include <glm/gtx/norm.hpp>

#include <fstream>
#include <string>
#include <string.h>
#include <unordered_map>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"

namespace IcarianCore
{
    enum e_OBJFaceMode
    {
        OBJFaceMode_Position,
        OBJFaceMode_PositionNormal,
        OBJFaceMode_PositionTexCoords,
        OBJFaceMode_PositionNormalTexcoords
    };

    struct OBJIndexMap
    {
        uint32_t PositionIndex;
        uint32_t NormalIndex;
        uint32_t TexCoordsIndex;
    };

    constexpr std::string_view VertexPositionStr = "v";
    constexpr std::string_view VertexNormalStr = "vn";
    constexpr std::string_view VertexTexCoordsStr = "vt";
    constexpr std::string_view IndicesFaceStr = "f";

    bool OBJLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        std::vector<glm::vec4> positions;
        positions.reserve(1024);
        std::vector<glm::vec3> normals;
        normals.reserve(1024);
        std::vector<glm::vec2> texcoords;
        texcoords.reserve(1024);

        std::vector<OBJIndexMap> indices;
        indices.reserve(1024);

        *a_radius = 0;

        float radSqr = 0;

        const char* s = a_data;
        while (s - a_data < a_size)
        {
            while (*s == '\n' || *s == ' ')
            {
                ++s;

                if ((s + 1) - a_data >= a_size)
                {
                    goto Exit;
                }
            }

            const char* nl = s + 1;
            while (*nl != '\n')
            {
                ++nl;

                if (nl - a_data >= a_size)
                {
                    goto Exit;
                }
            }

            const char* spc = s + 1;
            while (*spc != ' ')
            {
                ++spc;
            }

            const uint32_t len = (uint32_t)(spc - s);

            if (len == VertexPositionStr.size() && strncmp(s, VertexPositionStr.data(), VertexPositionStr.size()) == 0)
            {
                const char* xStr = spc + 1;
                while (*xStr == ' ')
                {
                    ++xStr;
                }

                const char* yStr = xStr + 1;
                while (*yStr != ' ')
                {
                    ++yStr;
                }
                while (*yStr == ' ')
                {
                    ++yStr;
                }

                const char* zStr = yStr + 1;
                while (*zStr != ' ')
                {
                    ++zStr;
                }
                while (*zStr == ' ')
                {
                    ++zStr;
                }

                const char* next = zStr + 1;
                while (*next != '\n' && *next != ' ')
                {
                    ++next;
                }

                const char* wStr = nullptr;
                if (*next != '\n')
                {
                    wStr = next;
                    while (*wStr == ' ')
                    {
                        ++wStr;
                    }

                    if (*wStr == '\n')
                    {
                        wStr = nullptr;
                    }
                }

                glm::vec4 pos;

                pos.x = std::stof(std::string(xStr, yStr - xStr));
                pos.y = std::stof(std::string(yStr, zStr - yStr));

                // W component is optional in file spec
                if (wStr != nullptr)
                {
                    pos.z = std::stof(std::string(zStr, wStr - zStr));
                    pos.w = std::stof(std::string(wStr, nl - wStr));
                }
                else
                {
                    pos.z = std::stof(std::string(zStr, nl - zStr));

                    pos.w = 1.0f;
                }

                const float radius = glm::length2(pos.xyz());
                if (radius > radSqr)
                {
                    radSqr = radius;
                }

                positions.emplace_back(pos);
            }
            else if (len == VertexNormalStr.size() && strncmp(s, VertexNormalStr.data(), VertexNormalStr.size()) == 0)
            {
                const char* xStr = spc + 1;
                while (*xStr == ' ')
                {
                    ++xStr;
                }

                const char* yStr = xStr + 1;
                while (*yStr != ' ')
                {
                    ++yStr;
                }
                while (*yStr == ' ')
                {
                    ++yStr;
                }

                const char* zStr = yStr + 1;
                while (*zStr != ' ')
                {
                    ++zStr;
                }
                while (*zStr == ' ')
                {
                    ++zStr;
                }

                glm::vec3 norm;

                norm.x = std::stof(std::string(xStr, yStr - xStr));
                norm.y = std::stof(std::string(yStr, zStr - yStr));
                norm.z = std::stof(std::string(zStr, nl - zStr));

                // File spec does not guarantee normalization
                normals.emplace_back(glm::normalize(norm));
            }
            else if (len == VertexTexCoordsStr.size() && strncmp(s, VertexTexCoordsStr.data(), VertexTexCoordsStr.size()) == 0)
            {
                const char* uStr = spc + 1;
                while (*uStr == ' ')
                {
                    ++uStr;
                }

                const char *next = uStr + 1;
                while (*next != '\n' && *next != ' ')
                {
                    ++next;
                }

                // V component is optional
                const char* vStr = nullptr;
                if (*next != '\n')
                {
                    vStr = next;
                    while (*vStr == ' ')
                    {
                        ++vStr;
                    }

                    if (*vStr == '\n')
                    {
                        vStr = nullptr;
                    }
                }

                glm::vec2 uv;
                if (vStr != nullptr)
                {
                    uv.x = std::stof(std::string(uStr, vStr - uStr));

                    // Can be a W component but dont care about it
                    next = vStr + 1;
                    while (*next != '\n' && *next != ' ')
                    {
                        ++next;
                    }

                    uv.y = std::stof(std::string(vStr, next - vStr));
                }
                else
                {
                    uv.x = std::stof(std::string(uStr, nl - uStr));
                    uv.y = 0.0f;
                }

                texcoords.emplace_back(uv);
            }
            else if (len == IndicesFaceStr.size() && strncmp(s, IndicesFaceStr.data(), IndicesFaceStr.size()) == 0)
            {
                const char* iA = spc + 1;
                while (*iA == ' ')
                {
                    ++iA;
                }

                std::vector<const char*> iSpc;
                iSpc.emplace_back(iA);

                const char* sl[2];
                sl[0] = nullptr;
                sl[1] = nullptr;

                const char* s = iA + 1;
                while (*s != ' ')
                {
                    if (*s == '/')
                    {
                        if (sl[0] == nullptr)
                        {
                            sl[0] = s;
                        }
                        else
                        {
                            sl[1] = s;

                            break;
                        }
                    }

                    ++s;
                }

                e_OBJFaceMode faceMode;
                if (sl[0] != nullptr && sl[1] != nullptr)
                {
                    if (sl[0] + 1 == sl[1])
                    {
                        faceMode = OBJFaceMode_PositionNormal;
                    }
                    else
                    {
                        faceMode = OBJFaceMode_PositionNormalTexcoords;
                    }
                }
                else if (sl[0] != nullptr)
                {
                    faceMode = OBJFaceMode_PositionTexCoords;
                }
                else
                {
                    faceMode = OBJFaceMode_Position;
                }

                while (*s != '\n')
                {
                    const char* next = s + 1;

                    while (*next != ' ' && *next != '\n')
                    {
                        ++next;
                    }
                    while (*next == ' ')
                    {
                        ++next;
                    }

                    iSpc.emplace_back(next);

                    if (*next == '\n')
                    {
                        break;
                    }

                    s = next + 1;
                }


                const uint32_t indexCount = iSpc.size() - 1;
                OBJIndexMap* tIndices = new OBJIndexMap[indexCount];
                IDEFER(delete[] tIndices);

                switch (faceMode)
                {
                case OBJFaceMode_Position:
                {
                    for (int i = 0; i < indexCount; ++i)
                    {
                        const char* cur = iSpc[i];
                        const char* next = iSpc[i + 1];

                        tIndices[i].PositionIndex = (uint32_t)std::stoul(std::string(cur, next - cur));
                        tIndices[i].NormalIndex = -1;
                        tIndices[i].TexCoordsIndex = -1;
                    }

                    break;
                }
                case OBJFaceMode_PositionTexCoords:
                {
                    for (int i = 0; i < indexCount; ++i)
                    {
                        const char* cur = iSpc[i];
                        const char* next = iSpc[i + 1];

                        const char* s = cur + 1;
                        while (*s != '/')
                        {
                            ++s;
                        }

                        tIndices[i].PositionIndex = (uint32_t)std::stoul(std::string(cur, s - cur));
                        tIndices[i].NormalIndex = -1;
                        tIndices[i].TexCoordsIndex = (uint32_t)std::stoul(std::string(s + 1, next - s - 1));
                    }

                    break;
                }
                case OBJFaceMode_PositionNormalTexcoords:
                {
                    for (int i = 0; i < indexCount; ++i)
                    {
                        const char* cur = iSpc[i];
                        const char* next = iSpc[i + 1];

                        const char* sA = cur + 1;
                        while (*sA != '/')
                        {
                            ++sA;
                        }

                        const char* sB = sA + 1;
                        while (*sB != '/')
                        {
                            ++sB;
                        }

                        tIndices[i].PositionIndex = (uint32_t)std::stoul(std::string(cur, sA - cur));
                        tIndices[i].NormalIndex = (uint32_t)std::stoul(std::string(sB + 1, (next - sB) - 1));
                        tIndices[i].TexCoordsIndex = (uint32_t)std::stoul(std::string(sA + 1, (sB - sA) - 1));
                    }

                    break;
                }
                case OBJFaceMode_PositionNormal:
                {
                    for (int i = 0; i < indexCount; ++i)
                    {
                        const char* cur = iSpc[i];
                        const char* next = iSpc[i + 1];

                        const char* s = cur + 1;
                        while (*s != '/')
                        {
                            ++s;
                        }

                        tIndices[i].PositionIndex = (uint32_t)std::stoul(std::string(cur, s - cur));
                        tIndices[i].NormalIndex = (uint32_t)std::stoul(std::string(s + 2, next - s - 2));
                        tIndices[i].TexCoordsIndex = -1;
                    }

                    break;
                }
                }

                ICARIAN_ASSERT(indexCount >= 3);

                for (uint32_t i = 2; i < indexCount; ++i)
                {
                    indices.emplace_back(tIndices[0]);
                    indices.emplace_back(tIndices[i - 1]);
                    indices.emplace_back(tIndices[i]);
                }
            }

            s = nl + 1;
        }

    Exit:;

        *a_radius = glm::sqrt(radSqr);

        std::unordered_map<uint64_t, uint32_t> indexMap;

        a_indices->reserve(indices.size());
        a_vertices->reserve(indices.size());
        for (const OBJIndexMap& iMap : indices)
        {
            // https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
            const uint64_t abH = ((uint64_t)iMap.PositionIndex + iMap.NormalIndex) * ((uint64_t)iMap.PositionIndex + iMap.NormalIndex + 1) / 2 + iMap.NormalIndex;
            const uint64_t h = ((abH + iMap.TexCoordsIndex) * (abH + iMap.TexCoordsIndex + 1) / 2 + iMap.TexCoordsIndex);

            const auto iter = indexMap.find(h);
            if (iter != indexMap.end())
            {
                a_indices->emplace_back(iter->second);
            }
            else
            {
                const uint32_t index = (uint32_t)a_vertices->size();
                a_indices->emplace_back(index);

                indexMap.emplace(h, index);

                ICARIAN_ASSERT(iMap.PositionIndex - 1 < positions.size());
                Vertex v;
                v.Position = positions[iMap.PositionIndex - 1];

                if (iMap.NormalIndex != -1)
                {
                    ICARIAN_ASSERT(iMap.NormalIndex - 1 < normals.size());
                    v.Normal = normals[iMap.NormalIndex - 1];
                }

                if (iMap.TexCoordsIndex != -1)
                {
                    ICARIAN_ASSERT(iMap.TexCoordsIndex - 1 < texcoords.size());
                    v.TexCoords = texcoords[iMap.TexCoordsIndex - 1];
                }

                a_vertices->emplace_back(v);
            }
        }

        return true;
    }
    bool OBJLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path, std::ios_base::binary);

            if (file.good() && file.is_open())
            {
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios::beg);

                char* dat = new char[size];
                IDEFER(delete[] dat);
                file.read(dat, size);

                return OBJLoader_LoadData(dat, (uint32_t)size, a_vertices, a_indices, a_radius);          
            }
        }

        return false;
    }
}
