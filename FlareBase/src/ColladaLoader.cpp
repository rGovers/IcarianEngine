#include "Flare/ColladaLoader.h"

#include <fstream>
#include <string>
#include <tinyxml2.h>
#include <unordered_map>

#include "Flare/IcarianAssert.h"

namespace FlareBase
{
    enum e_ColladaUpAxis
    {
        ColladaUpAxis_XUp,
        ColladaUpAxis_YUp,
        ColladaUpAxis_ZUp
    };

    enum e_ColladaSourceDataType
    {
        ColladaSourceDataType_Null = -1,
        ColladaSourceDataType_Float
    };

    struct ColladaParam
    {
        std::string Name;
        e_ColladaSourceDataType Type;
    };
    struct ColladaInput
    {
        std::string Semantic;
        std::string Source;
        uint32_t Offset;
    };

    struct ColladaAccessor
    {
        std::string Source;
        uint32_t Stride;
        uint32_t Offset;
        uint32_t Count;
        std::vector<ColladaParam> Params;
    };
    struct ColladaData
    {
        std::string ID;
        void* Data;
        e_ColladaSourceDataType Type;
    };

    struct ColladaSource
    {
        std::string ID;
        ColladaData Data;
        ColladaAccessor Accessor;
    };

    struct ColladaTriangles
    {
        std::vector<ColladaInput> Inputs;
        std::vector<uint32_t> P;
    };
    struct ColladaPolylist
    {
        std::vector<ColladaInput> Inputs;
        std::vector<uint32_t> VCount;
        std::vector<uint32_t> P;
    };

    struct ColladaMesh
    {
        std::vector<ColladaSource> Sources;
        std::vector<ColladaInput> Vertices;
        ColladaPolylist Polylist;
        ColladaTriangles Triangles;
    };

    struct ColladaGeometry
    {
        std::string ID;
        ColladaMesh Mesh;
    };

    static ColladaMesh LoadMesh(const tinyxml2::XMLElement* a_meshElement)
    {
        ColladaMesh mesh;

        for (const tinyxml2::XMLElement* sourceElement = a_meshElement->FirstChildElement(); sourceElement != nullptr; sourceElement = sourceElement->NextSiblingElement())
        {
            const char* name = sourceElement->Value();

            if (strcmp(name, "source") == 0)
            {
                ColladaSource s;
                for (const tinyxml2::XMLAttribute* att = sourceElement->FirstAttribute(); att != nullptr; att = att->Next())
                {
                    const char* name = att->Name();
                    if (strcmp(name, "id") == 0)
                    {
                        s.ID = att->Value();
                    }
                }

                for (const tinyxml2::XMLElement* sDataElement = sourceElement->FirstChildElement(); sDataElement != nullptr; sDataElement = sDataElement->NextSiblingElement())
                {
                    const char* name = sDataElement->Value();

                    if (strcmp(name, "float_array") == 0)
                    {
                        ColladaData d;
                        for (const tinyxml2::XMLAttribute* att = sDataElement->FirstAttribute(); att != nullptr; att = att->Next())
                        {
                            const char* name = att->Name();
                            if (strcmp(name, "id") == 0)
                            {
                                d.ID = att->Value();
                            }
                        }
                        d.Type = ColladaSourceDataType_Float;

                        const uint32_t count = (uint32_t)sDataElement->Int64Attribute("count");

                        const char* data = sDataElement->GetText();

                        d.Data = new float[count];
                        float* fDat = (float*)d.Data;

                        const char* sC = data;
                        uint32_t index = 0;

                        while (*sC != 0 && index < count)
                        {
                            while (*sC == ' ' || *sC == '\n')
                            {
                                ++sC;
                            }

                            if (*sC == 0)
                            {
                                break;
                            }

                            const char* next = sC + 1;
                            while (*next != ' ' && *next != 0)
                            {
                                ++next;
                            }

                            fDat[index++] = std::stof(std::string(sC, next - sC));

                            sC = next;
                        }

                        s.Data = d;
                    }
                    else if (strcmp(name, "technique_common") == 0)
                    {
                        const tinyxml2::XMLElement* accessorElement = sDataElement->FirstChildElement("accessor");
                        ICARIAN_ASSERT(accessorElement != nullptr);

                        ColladaAccessor a;
                        for (const tinyxml2::XMLAttribute* att = accessorElement->FirstAttribute(); att != nullptr; att = att->Next())
                        {
                            const char* name = att->Name();
                            if (strcmp(name, "source") == 0)
                            {
                                a.Source = att->Value();
                            }
                            else if (strcmp(name, "count") == 0)
                            {
                                a.Count = (uint32_t)att->Int64Value();
                            }
                            else if (strcmp(name, "stride") == 0)
                            {
                                a.Stride = (uint32_t)att->IntValue();
                            }
                            else if (strcmp(name, "offset") == 0)
                            {
                                a.Offset = (uint32_t)att->IntValue();
                            }
                        }

                        for (const tinyxml2::XMLElement* paramElement = accessorElement->FirstChildElement(); paramElement != nullptr; paramElement = paramElement->NextSiblingElement())
                        {
                            const char* name = paramElement->Value();
                            if (strcmp(name, "param") == 0)
                            {
                                ColladaParam p;
                                p.Type = ColladaSourceDataType_Null;
                                for (const tinyxml2::XMLAttribute* att = paramElement->FirstAttribute(); att != nullptr; att = att->Next())
                                {
                                    const char* name = att->Name();
                                    if (strcmp(name, "name") == 0)
                                    {
                                        p.Name = att->Value();
                                    }
                                    else if (strcmp(name, "type") == 0)
                                    {
                                        const char* type = att->Value();
                                        if (strcmp(type, "float") == 0)
                                        {
                                            p.Type = ColladaSourceDataType_Float;
                                        }
                                    }   
                                }

                                a.Params.emplace_back(p);
                            }
                        }

                        s.Accessor = a;
                    }
                }

                mesh.Sources.emplace_back(s);
            }
            else if (strcmp(name, "vertices") == 0)
            {
                for (const tinyxml2::XMLElement* inputElement = sourceElement->FirstChildElement(); inputElement != nullptr; inputElement = inputElement->NextSiblingElement())
                {
                    ColladaInput i;
                    for (const tinyxml2::XMLAttribute* att = inputElement->FirstAttribute(); att != nullptr; att = att->Next())
                    {
                        const char* name = att->Name();
                        if (strcmp(name, "semantic") == 0)
                        {
                            i.Semantic = att->Value();
                        }
                        else if (strcmp(name, "source") == 0)
                        {
                            i.Source = att->Value();
                        }
                        else if (strcmp(name, "offset") == 0)
                        {
                            i.Offset = (uint32_t)att->IntValue();
                        }
                    }

                    mesh.Vertices.emplace_back(i);
                }
            }
            else if (strcmp(name, "triangles") == 0)
            {
                ColladaTriangles t;

                for (const tinyxml2::XMLElement* element = sourceElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
                {
                    const char* name = element->Value();
                    if (strcmp(name, "input") == 0)
                    {
                        ColladaInput i;
                        for (const tinyxml2::XMLAttribute* att = element->FirstAttribute(); att != nullptr; att = att->Next())
                        {
                            const char* name = att->Name();
                            if (strcmp(name, "semantic") == 0)
                            {
                                i.Semantic = att->Value();
                            }
                            else if (strcmp(name, "source") == 0)
                            {
                                i.Source = att->Value();
                            }
                            else if (strcmp(name, "offset") == 0)
                            {
                                i.Offset = (uint32_t)att->IntValue();
                            }
                        }

                        t.Inputs.emplace_back(i);
                    }
                    else if (strcmp(name, "p") == 0)
                    {
                        const char* data = element->GetText();
                        const char* s = data;
                        while (*s != 0)
                        {
                            while (*s == ' ')
                            {
                                ++s;
                            }

                            if (*s == 0)
                            {
                                break;
                            }

                            const char* next = s + 1;
                            while (*next != ' ' && *next != 0)
                            {
                                ++next;
                            }

                            t.P.emplace_back((uint32_t)std::stoll(std::string(s, next - s)));

                            s = next;
                        }
                    }
                }

                mesh.Triangles = t;
            }
            else if (strcmp(name, "polylist") == 0)
            {
                ColladaPolylist p;

                for (const tinyxml2::XMLElement* element = sourceElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
                {
                    const char* name = element->Value();
                    if (strcmp(name, "input") == 0)
                    {
                        ColladaInput i;
                        for (const tinyxml2::XMLAttribute* att = element->FirstAttribute(); att != nullptr; att = att->Next())
                        {
                            const char* name = att->Name();
                            if (strcmp(name, "semantic") == 0)
                            {
                                i.Semantic = att->Value();
                            }
                            else if (strcmp(name, "source") == 0)
                            {
                                i.Source = att->Value();
                            }
                            else if (strcmp(name, "offset") == 0)
                            {
                                i.Offset = (uint32_t)att->IntValue();
                            }
                        }

                        p.Inputs.emplace_back(i);
                    }
                    else if (strcmp(name, "vcount") == 0)
                    {
                        const char* data = element->GetText();
                        const char* s = data;
                        while (*s != 0)
                        {
                            while (*s == ' ')
                            {
                                ++s;
                            }

                            if (*s == 0)
                            {
                                break;
                            }

                            const char* next = s + 1;
                            while (*next != ' ' && *next != 0)
                            {
                                ++next;
                            }

                            p.VCount.emplace_back((uint32_t)std::stoi(std::string(s, next - s)));

                            s = next;
                        }
                    }
                    else if (strcmp(name, "p") == 0)
                    {
                        const char* data = element->GetText();
                        const char* s = data;
                        while (*s != 0)
                        {
                            while (*s == ' ')
                            {
                                ++s;
                            }

                            if (*s == 0)
                            {
                                break;
                            }

                            const char* next = s + 1;
                            while (*next != ' ' && *next != 0)
                            {
                                ++next;
                            }

                            p.P.emplace_back((uint32_t)std::stoll(std::string(s, next - s)));

                            s = next;
                        }
                    }
                }

                mesh.Polylist = p;
            }
        }

        return mesh;
    }

    static std::vector<ColladaGeometry> LoadGeometry(const tinyxml2::XMLElement* a_libraryElement)
    {
        std::vector<ColladaGeometry> geometryLib;

        for (const tinyxml2::XMLElement* geomElement = a_libraryElement->FirstChildElement(); geomElement != nullptr; geomElement = geomElement->NextSiblingElement())
        {
            if (strcmp(geomElement->Value(), "geometry") == 0)
            {
                ColladaGeometry g;
                for (const tinyxml2::XMLAttribute* att = geomElement->FirstAttribute(); att != nullptr; att = att->Next())
                {
                    const char* name = att->Name();
                    if (strcmp(name, "id") == 0)
                    {
                        g.ID = att->Value();
                    }
                }

                const tinyxml2::XMLElement* meshElement = geomElement->FirstChildElement("mesh");
                ICARIAN_ASSERT(meshElement != nullptr);

                g.Mesh = LoadMesh(meshElement);

                geometryLib.emplace_back(g);
            }
        }

        return geometryLib;
    }

    int* ColladaLoader_GetOffsets(const ColladaSource& a_source, int* a_count)
    {
        *a_count = (int)a_source.Accessor.Params.size();

        int* offset = new int[*a_count];
        for (uint32_t i = 0; i < *a_count; ++i)
        {
            offset[i] = 0;

            const ColladaParam& p = a_source.Accessor.Params[i];
            if (p.Name == "X" || p.Name == "S")
            {
                offset[i] = 0;
            }
            else if (p.Name == "Y" || p.Name == "T")
            {
                offset[i] = 1;
            }
            else if (p.Name == "Z")
            {
                offset[i] = 2;
            }
            else if (p.Name == "W")
            {
                offset[i] = 3;
            }
        }

        return offset;
    }

    bool ColladaLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices)
    {
        tinyxml2::XMLDocument doc;
        if (doc.Parse(a_data, (size_t)a_size) == tinyxml2::XML_SUCCESS)
        {
            const tinyxml2::XMLElement* rootElement = doc.RootElement();

            e_ColladaUpAxis up = ColladaUpAxis_YUp;
            float scale = 1.0f;
            std::vector<ColladaGeometry> geometry;

            for (const tinyxml2::XMLElement* element = rootElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
            {
                const char *elementName = element->Value();

                if (strcmp(elementName, "asset") == 0)
                {
                    for (const tinyxml2::XMLElement* assetElement = element->FirstChildElement(); assetElement != nullptr; assetElement = assetElement->NextSiblingElement())
                    {
                        const char* name = assetElement->Value();
                        if (strcmp(name, "up_axis") == 0)
                        {
                            const char *value = assetElement->GetText();
                            if (strcmp(value, "X_UP") == 0)
                            {
                                up = ColladaUpAxis_XUp;
                            }
                            else if (strcmp(value, "Y_UP") == 0)
                            {
                                up = ColladaUpAxis_YUp;
                            }
                            else if (strcmp(value, "Z_UP") == 0)
                            {
                                up = ColladaUpAxis_ZUp;
                            }
                        }
                        else if (strcmp(name, "unit") == 0)
                        {
                            scale = assetElement->FloatAttribute("meter");
                        }
                    }
                }
                else if (strcmp(elementName, "library_geometries") == 0)
                {
                    geometry = LoadGeometry(element);
                }
            }

            for (const ColladaGeometry &g : geometry)
            {
                if (!g.Mesh.Triangles.P.empty())
                {
                    ColladaInput posInput;
                    ColladaInput normalInput;
                    ColladaInput texcoordInput;

                    for (const ColladaInput &tI : g.Mesh.Triangles.Inputs)
                    {
                        if (tI.Semantic == "POSITION")
                        {
                            posInput = tI;
                        }
                        else if (tI.Semantic == "NORMAL")
                        {
                            normalInput = tI;
                        }
                        else if (tI.Semantic == "TEXCOORD")
                        {
                            texcoordInput = tI;
                        }
                        else if (tI.Semantic == "VERTEX")
                        {
                            for (const ColladaInput &vI : g.Mesh.Vertices)
                            {
                                if (vI.Semantic == "POSITION")
                                {
                                    posInput = vI;
                                    posInput.Offset = tI.Offset;
                                }
                                else if (vI.Semantic == "NORMAL")
                                {
                                    normalInput = vI;
                                    normalInput.Offset = tI.Offset;
                                }
                                else if (vI.Semantic == "TEXCOORD")
                                {
                                    texcoordInput = vI;
                                    texcoordInput.Offset = tI.Offset;
                                }
                            }
                        }
                    }

                    ColladaSource posSource;
                    ColladaSource normalSource;
                    ColladaSource texcoordSource;

                    for (const ColladaSource &d : g.Mesh.Sources)
                    {
                        const std::string idStr = "#" + d.ID;
                        if (idStr == posInput.Source)
                        {
                            posSource = d;
                        }
                        else if (idStr == normalInput.Source)
                        {
                            normalSource = d;
                        }
                        else if (idStr == texcoordInput.Source)
                        {
                            texcoordSource = d;
                        }
                    }

                    int posVCount;
                    int normVCount;
                    int texVCount;

                    int* posOffset = ColladaLoader_GetOffsets(posSource, &posVCount);
                    int* normalOffset = ColladaLoader_GetOffsets(normalSource, &normVCount);
                    int* texcoordOffset = ColladaLoader_GetOffsets(texcoordSource, &texVCount);

                    std::unordered_map<uint64_t, uint32_t> indexMap;

                    const uint32_t cIndexCount = (uint32_t)g.Mesh.Triangles.P.size();
                    const uint32_t cIndexStride = (uint32_t)g.Mesh.Triangles.Inputs.size();
                    const uint32_t cNext = cIndexStride * 3;

                    for (uint32_t i = 0; i < cIndexCount; i += cNext)
                    {
                        for (int j = 0; j < 3; ++j)
                        {
                            const uint32_t index = i + (2 - j) * cIndexStride;

                            const uint32_t posIndex = g.Mesh.Triangles.P[index + posInput.Offset];
                            const uint32_t normIndex = g.Mesh.Triangles.P[index + normalInput.Offset];
                            const uint32_t texIndex = g.Mesh.Triangles.P[index + texcoordInput.Offset];

                            // https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
                            const uint64_t abH = ((uint64_t)posIndex + normIndex) * ((uint64_t)posIndex + normIndex + 1) / 2 + normIndex;
                            const uint64_t h = (abH + texIndex) * (abH + texIndex + 1) / 2 + texIndex;

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

                                Vertex v = Vertex();
                                for (int j = 0; j < posVCount; ++j)
                                {
                                    if (posOffset[j] == 1)
                                    {
                                        v.Position[posOffset[j]] = -((float *)posSource.Data.Data)[(posIndex * posSource.Accessor.Stride) + j] * scale;
                                    }
                                    else
                                    {
                                        v.Position[posOffset[j]] = ((float *)posSource.Data.Data)[(posIndex * posSource.Accessor.Stride) + j] * scale;
                                    }
                                }

                                for (int j = 0; j < normVCount; ++j)
                                {
                                    if (normalOffset[j] == 1)
                                    {
                                        v.Normal[normalOffset[j]] = ((float *)normalSource.Data.Data)[(normIndex * normalSource.Accessor.Stride) + j];
                                    }
                                    else
                                    {
                                        v.Normal[normalOffset[j]] = -((float *)normalSource.Data.Data)[(normIndex * normalSource.Accessor.Stride) + j];
                                    }
                                }

                                for (int j = 0; j < texVCount; ++j)
                                {
                                    v.TexCoords[texcoordOffset[j]] = ((float *)texcoordSource.Data.Data)[(texIndex * texcoordSource.Accessor.Stride) + j];
                                }

                                a_vertices->emplace_back(v);
                            }
                        }
                    }

                    delete[] posOffset;
                    delete[] normalOffset;
                    delete[] texcoordOffset;
                }
                else
                {
                    ColladaInput posInput;
                    ColladaInput normalInput;
                    ColladaInput texcoordInput;

                    for (const ColladaInput &pI : g.Mesh.Polylist.Inputs)
                    {
                        if (pI.Semantic == "POSITION")
                        {
                            posInput = pI;
                        }
                        else if (pI.Semantic == "NORMAL")
                        {
                            normalInput = pI;
                        }
                        else if (pI.Semantic == "TEXCOORD")
                        {
                            texcoordInput = pI;
                        }
                        else if (pI.Semantic == "VERTEX")
                        {
                            for (const ColladaInput &vI : g.Mesh.Vertices)
                            {
                                if (vI.Semantic == "POSITION")
                                {
                                    posInput = vI;
                                    posInput.Offset = pI.Offset;
                                }
                                else if (vI.Semantic == "NORMAL")
                                {
                                    normalInput = vI;
                                    normalInput.Offset = pI.Offset;
                                }
                                else if (vI.Semantic == "TEXCOORD")
                                {
                                    texcoordInput = vI;
                                    texcoordInput.Offset = pI.Offset;
                                }
                            }
                        }
                    }

                    ColladaSource posSource;
                    ColladaSource normalSource;
                    ColladaSource texcoordSource;

                    for (const ColladaSource &d : g.Mesh.Sources)
                    {
                        const std::string idStr = "#" + d.ID;
                        if (idStr == posInput.Source)
                        {
                            posSource = d;
                        }
                        else if (idStr == normalInput.Source)
                        {
                            normalSource = d;
                        }
                        else if (idStr == texcoordInput.Source)
                        {
                            texcoordSource = d;
                        }
                    }

                    int posVCount;
                    int normVCount;
                    int texVCount;

                    int* posOffset = ColladaLoader_GetOffsets(posSource, &posVCount);
                    int* normalOffset = ColladaLoader_GetOffsets(normalSource, &normVCount);
                    int* texcoordOffset = ColladaLoader_GetOffsets(texcoordSource, &texVCount);

                    std::unordered_map<uint64_t, uint32_t> indexMap;

                    uint32_t index = 0;
                    const uint32_t count = (uint32_t)g.Mesh.Polylist.Inputs.size();
                    for (uint32_t vCount : g.Mesh.Polylist.VCount)
                    {
                        uint32_t posIndices[4];
                        uint32_t normalIndices[4];
                        uint32_t texcoordIndices[4];

                        for (int i = 0; i < vCount; ++i)
                        {
                            // Flip faces so back culling work correctly
                            const uint32_t iIndex = index + ((vCount - 1) - i) * count;

                            posIndices[i] = g.Mesh.Polylist.P[iIndex + posInput.Offset];
                            normalIndices[i] = g.Mesh.Polylist.P[iIndex + normalInput.Offset];
                            texcoordIndices[i] = g.Mesh.Polylist.P[iIndex + texcoordInput.Offset];
                        }

                        switch (vCount)
                        {
                        case 3:
                        {
                            for (int i = 0; i < 3; ++i)
                            {
                                // https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
                                const uint64_t abH = ((uint64_t)posIndices[i] + normalIndices[i]) * ((uint64_t)posIndices[i] + normalIndices[i] + 1) / 2 + normalIndices[i];
                                const uint64_t h = (abH + texcoordIndices[i]) * (abH + texcoordIndices[i] + 1) / 2 + texcoordIndices[i];

                                const auto iter = indexMap.find(h);
                                if (iter != indexMap.end())
                                {
                                    a_indices->emplace_back(iter->second);
                                }
                                else
                                {
                                    const uint32_t vIndex = (uint32_t)a_vertices->size();

                                    a_indices->emplace_back(vIndex);

                                    Vertex v = Vertex();
                                    for (int j = 0; j < posVCount; ++j)
                                    {
                                        if (posOffset[j] == 1)
                                        {
                                            v.Position[posOffset[j]] = -((float*)posSource.Data.Data)[(posIndices[i] * posSource.Accessor.Stride) + j] * scale;
                                        }
                                        else
                                        {
                                            v.Position[posOffset[j]] = ((float*)posSource.Data.Data)[(posIndices[i] * posSource.Accessor.Stride) + j] * scale;
                                        }
                                    }

                                    for (int j = 0; j < normVCount; ++j)
                                    {
                                        if (normalOffset[j] == 1)
                                        {
                                            v.Normal[normalOffset[j]] = ((float*)normalSource.Data.Data)[(normalIndices[i] * normalSource.Accessor.Stride) + j];
                                        }
                                        else
                                        {
                                            v.Normal[normalOffset[j]] = -((float*)normalSource.Data.Data)[(normalIndices[i] * normalSource.Accessor.Stride) + j];
                                        }
                                    }

                                    for (int j = 0; j < texVCount; ++j)
                                    {
                                        v.TexCoords[texcoordOffset[j]] = ((float*)texcoordSource.Data.Data)[(texcoordIndices[i] * texcoordSource.Accessor.Stride) + j];
                                    }

                                    a_vertices->emplace_back(v);

                                    indexMap.emplace(h, vIndex);
                                }
                            }

                            break;
                        }
                        case 4:
                        {
                            // TODO: Implement quads

                            break;
                        }
                        default:
                        {
                            // TODO: Implement polys

                            break;
                        }
                        }

                        index += count * vCount;
                    }

                    delete[] posOffset;
                    delete[] normalOffset;
                    delete[] texcoordOffset;
                }
            }

            for (const ColladaGeometry &g : geometry)
            {
                for (const ColladaSource &s : g.Mesh.Sources)
                {
                    switch (s.Data.Type)
                    {
                    case ColladaSourceDataType_Float:
                    {
                        delete[] (float *)s.Data.Data;

                        break;
                    }
                    }
                }
            }

            return true;
        }
    }
    bool ColladaLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path);

            if (file.good() && file.is_open())
            {
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios::beg);

                char* dat = new char[size];
                file.read(dat, size);

                const bool ret = ColladaLoader_LoadData(dat, (uint32_t)size, a_vertices, a_indices);

                delete[] dat;

                return ret;
            }
        }

        return false;
    }
}
