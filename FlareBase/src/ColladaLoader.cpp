#include "Flare/ColladaLoader.h"

#include <fstream>
#include <string>
#include <tinyxml2.h>
#include <unordered_map>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"

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
        ColladaSourceDataType_Float,
        ColladaSourceDataType_Name
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

    struct ColladaVertexWeights
    {
        uint32_t Count;
        std::vector<ColladaInput> Inputs;
        std::vector<uint32_t> VCount;
        std::vector<uint32_t> V;
    };

    struct ColladaSkin
    {
        std::string Source;
        float BindShapeMatrix[16];
        std::vector<ColladaSource> Sources;
        ColladaVertexWeights VertexWeights;
    };

    struct ColladaSceneNode
    {
        std::string ID;
        std::string Name;
        std::string Type;
        float Transform[16];
        std::vector<ColladaSceneNode> Children;
    };

    struct ColladaVisualScene
    {
        std::string ID;
        std::vector<ColladaSceneNode> Nodes;
    };

    struct ColladaController
    {
        std::string ID;
        ColladaSkin Skin;
    };

    static ColladaSource LoadSource(const tinyxml2::XMLElement* a_sourceElement)
    {
        ColladaSource s;
        for (const tinyxml2::XMLAttribute* att = a_sourceElement->FirstAttribute(); att != nullptr; att = att->Next()) 
        {
            const char* name = att->Name();
            if (strcmp(name, "id") == 0) 
            {
              s.ID = att->Value();
            }
        }

        for (const tinyxml2::XMLElement* sDataElement = a_sourceElement->FirstChildElement(); sDataElement != nullptr; sDataElement = sDataElement->NextSiblingElement()) 
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

                    const char *next = sC + 1;
                    while (*next != ' ' && *next != 0) 
                    {
                      ++next;
                    }

                    fDat[index++] = std::stof(std::string(sC, next - sC));

                    sC = next;
                }

                s.Data = d;
            }
            else if (strcmp(name, "Name_array") == 0)
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
                d.Type = ColladaSourceDataType_Name;

                const uint32_t count = (uint32_t)sDataElement->Int64Attribute("count");

                const char* data = sDataElement->GetText();

                d.Data = new std::string[count];
                std::string* sDat = (std::string*)d.Data;

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

                    sDat[index++] = std::string(sC, next - sC);

                    sC = next;
                }
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

        return s;
    }

    static ColladaMesh LoadMesh(const tinyxml2::XMLElement* a_meshElement)
    {
        ColladaMesh mesh;

        for (const tinyxml2::XMLElement* sourceElement = a_meshElement->FirstChildElement(); sourceElement != nullptr; sourceElement = sourceElement->NextSiblingElement())
        {
            const char* name = sourceElement->Value();

            if (strcmp(name, "source") == 0)
            {
                const ColladaSource s = LoadSource(sourceElement);                

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
    static ColladaSkin LoadSkin(const tinyxml2::XMLElement* a_skinElement)
    {
        ColladaSkin skin;

        for (const tinyxml2::XMLElement* element = a_skinElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
        {
            const char* name = element->Value();

            if (strcmp(name, "bind_shape_matrix") == 0)
            {
                const char* data = element->GetText();
                const char* s = data;
                uint32_t index = 0;
                while (*s != 0 && index < 16)
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

                    skin.BindShapeMatrix[index++] = std::stof(std::string(s, next - s));

                    s = next;
                }
            }
            else if (strcmp(name, "source") == 0) 
            {
                const ColladaSource s = LoadSource(element);

                skin.Sources.emplace_back(s);
            }
            else if (strcmp(name, "vertex_weights") == 0)
            {
                ColladaVertexWeights w;
                for (const tinyxml2::XMLAttribute* att = element->FirstAttribute(); att != nullptr; att = att->Next())
                {
                    const char* name = att->Name();
                    if (strcmp(name, "count") == 0)
                    {
                        skin.VertexWeights.Count = (uint32_t)att->IntValue();
                    }
                }

                for (const tinyxml2::XMLElement* inputElement = element->FirstChildElement(); inputElement != nullptr; inputElement = inputElement->NextSiblingElement())
                {
                    const char* name = inputElement->Value();

                    if (strcmp(name, "input") == 0)
                    {
                        ColladaInput input;

                        for (const tinyxml2::XMLAttribute* att = inputElement->FirstAttribute(); att != nullptr; att = att->Next())
                        {
                            const char* name = att->Name();
                            if (strcmp(name, "semantic") == 0)
                            {
                                input.Semantic = att->Value();
                            }
                            else if (strcmp(name, "source") == 0)
                            {
                                input.Source = att->Value();
                            }
                            else if (strcmp(name, "offset") == 0)
                            {
                                input.Offset = (uint32_t)att->IntValue();
                            }
                        }

                        w.Inputs.emplace_back(input);
                    }
                    else if (strcmp(name, "vcount") == 0)
                    {
                        const char* data = inputElement->GetText();
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

                            w.VCount.emplace_back((uint32_t)std::stoll(std::string(s, next - s)));

                            s = next;
                        }
                    }
                    else if (strcmp(name, "v") == 0)
                    {
                        const char* data = inputElement->GetText();
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

                            w.V.emplace_back((uint32_t)std::stoll(std::string(s, next - s)));

                            s = next;
                        }
                    }
                }
            }
        }

        return skin;
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

    static std::vector<ColladaController> LoadController(const tinyxml2::XMLElement* a_libraryElement)
    {
        std::vector<ColladaController> controllerLib;

        for (const tinyxml2::XMLElement* controllerElement = a_libraryElement->FirstChildElement(); controllerElement != nullptr; controllerElement = controllerElement->NextSiblingElement())
        {
            if (strcmp(controllerElement->Value(), "controller") == 0)
            {
                ColladaController c;
                for (const tinyxml2::XMLAttribute* att = controllerElement->FirstAttribute(); att != nullptr; att = att->Next())
                {
                    const char* name = att->Name();
                    if (strcmp(name, "id") == 0)
                    {
                        c.ID = att->Value();
                    }
                }

                const tinyxml2::XMLElement* skinElement = controllerElement->FirstChildElement("skin");
                ICARIAN_ASSERT(skinElement != nullptr);

                c.Skin = LoadSkin(skinElement);

                controllerLib.emplace_back(c);
            }
        }

        return controllerLib;
    }

    static ColladaSceneNode LoadSceneNode(const tinyxml2::XMLElement* a_element)
    {
        ColladaSceneNode node;
        for (const tinyxml2::XMLAttribute* att = a_element->FirstAttribute(); att != nullptr; att = att->Next())
        {
            const char* name = att->Name();
            if (strcmp(name, "id") == 0)
            {
                node.ID = att->Value();
            }
            else if (strcmp(name, "name") == 0)
            {
                node.Name = att->Value();
            }
            else if (strcmp(name, "type") == 0)
            {
                node.Type = att->Value();
            }
        }

        for (const tinyxml2::XMLElement* element = a_element->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
        {
            const char* name = element->Value();

            if (strcmp(name, "matrix") == 0)
            {
                const char* data = element->GetText();
                const char* s = data;
                uint32_t index = 0;
                while (*s != 0 && index < 16)
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

                    node.Transform[index++] = std::stof(std::string(s, next - s));

                    s = next;
                }
            }
            else if (strcmp(name, "node") == 0)
            {
                const ColladaSceneNode n = LoadSceneNode(element);

                node.Children.emplace_back(n);
            }
        }

        return node;
    }

    static std::vector<ColladaVisualScene> LoadScene(const tinyxml2::XMLElement* a_libraryElement)
    {
        std::vector<ColladaVisualScene> sceneLib;

        for (const tinyxml2::XMLElement* sceneElement = a_libraryElement->FirstChildElement(); sceneElement != nullptr; sceneElement = sceneElement->NextSiblingElement())
        {
            if (strcmp(sceneElement->Value(), "visual_scene") == 0)
            {
                ColladaVisualScene s;
                for (const tinyxml2::XMLAttribute* att = sceneElement->FirstAttribute(); att != nullptr; att = att->Next())
                {
                    const char* name = att->Name();
                    if (strcmp(name, "id") == 0)
                    {
                        s.ID = att->Value();
                    }
                }

                for (const tinyxml2::XMLElement* nodeElement = sceneElement->FirstChildElement(); nodeElement != nullptr; nodeElement = nodeElement->NextSiblingElement())
                {
                    if (strcmp(nodeElement->Value(), "node") == 0)
                    {
                        const ColladaSceneNode n = LoadSceneNode(nodeElement);

                        s.Nodes.emplace_back(n);
                    }
                }

                sceneLib.emplace_back(s);
            }
        }

        return sceneLib;
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

    bool ColladaLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        tinyxml2::XMLDocument doc;
        if (doc.Parse(a_data, (size_t)a_size) == tinyxml2::XML_SUCCESS)
        {
            const tinyxml2::XMLElement* rootElement = doc.RootElement();

            e_ColladaUpAxis up = ColladaUpAxis_YUp;
            float scale = 1.0f;
            std::vector<ColladaGeometry> geometry;
            IDEFER( 
            {
                for (const ColladaGeometry& g : geometry)
                {
                    for (const ColladaSource& s : g.Mesh.Sources)
                    {
                        switch (s.Data.Type)
                        {
                        case ColladaSourceDataType_Float:
                        {
                            delete[] (float*)s.Data.Data;

                            break;
                        }
                        case ColladaSourceDataType_Name:
                        {
                            delete[] (std::string*)s.Data.Data;

                            break;
                        }
                        default:
                        {
                            break;
                        }
                        }
                    }
                }
            });

            for (const tinyxml2::XMLElement* element = rootElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
            {
                const char* elementName = element->Value();

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

            for (const ColladaGeometry& g : geometry)
            {
                if (!g.Mesh.Triangles.P.empty())
                {
                    ColladaInput posInput;
                    ColladaInput normalInput;
                    ColladaInput texcoordInput;

                    for (const ColladaInput& tI : g.Mesh.Triangles.Inputs)
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
                            for (const ColladaInput& vI : g.Mesh.Vertices)
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

                    for (const ColladaSource& d : g.Mesh.Sources)
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

                    const int* posOffset = ColladaLoader_GetOffsets(posSource, &posVCount);
                    IDEFER(delete[] posOffset);
                    const int* normalOffset = ColladaLoader_GetOffsets(normalSource, &normVCount);
                    IDEFER(delete[] normalOffset);
                    const int* texcoordOffset = ColladaLoader_GetOffsets(texcoordSource, &texVCount);
                    IDEFER(delete[] texcoordOffset);

                    std::unordered_map<uint64_t, uint32_t> indexMap;

                    const uint32_t cIndexCount = (uint32_t)g.Mesh.Triangles.P.size();
                    const uint32_t cIndexStride = (uint32_t)g.Mesh.Triangles.Inputs.size();
                    const uint32_t cNext = cIndexStride * 3;

                    for (uint32_t i = 0; i < cIndexCount; i += cNext)
                    {
                        for (uint32_t j = 0; j < 3; ++j)
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

                                // TODO: Account for up axis
                                Vertex v = Vertex();
                                for (int j = 0; j < posVCount; ++j)
                                {
                                    if (posOffset[j] == 1)
                                    {
                                        v.Position[posOffset[j]] = -((float*)posSource.Data.Data)[(posIndex * posSource.Accessor.Stride) + j] * scale;
                                    }
                                    else
                                    {
                                        v.Position[posOffset[j]] = ((float*)posSource.Data.Data)[(posIndex * posSource.Accessor.Stride) + j] * scale;
                                    }
                                }

                                for (int j = 0; j < normVCount; ++j)
                                {
                                    if (normalOffset[j] == 1)
                                    {
                                        v.Normal[normalOffset[j]] = ((float*)normalSource.Data.Data)[(normIndex * normalSource.Accessor.Stride) + j];
                                    }
                                    else
                                    {
                                        v.Normal[normalOffset[j]] = -((float*)normalSource.Data.Data)[(normIndex * normalSource.Accessor.Stride) + j];
                                    }
                                }

                                for (int j = 0; j < texVCount; ++j)
                                {
                                    v.TexCoords[texcoordOffset[j]] = ((float*)texcoordSource.Data.Data)[(texIndex * texcoordSource.Accessor.Stride) + j];
                                }

                                a_vertices->emplace_back(v);

                                const float radius = glm::length(v.Position.xyz());
                                if (radius > *a_radius)
                                {
                                    *a_radius = radius;
                                }
                            }
                        }
                    }
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
                            for (const ColladaInput& vI : g.Mesh.Vertices)
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

                    for (const ColladaSource& d : g.Mesh.Sources)
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
                    IDEFER(delete[] posOffset);
                    int* normalOffset = ColladaLoader_GetOffsets(normalSource, &normVCount);
                    IDEFER(delete[] normalOffset);
                    int* texcoordOffset = ColladaLoader_GetOffsets(texcoordSource, &texVCount);
                    IDEFER(delete[] texcoordOffset);

                    std::unordered_map<uint64_t, uint32_t> indexMap;

                    uint32_t index = 0;
                    const uint32_t count = (uint32_t)g.Mesh.Polylist.Inputs.size();
                    for (uint32_t vCount : g.Mesh.Polylist.VCount)
                    {
                        uint32_t posIndices[4];
                        uint32_t normalIndices[4];
                        uint32_t texcoordIndices[4];

                        for (uint32_t i = 0; i < vCount; ++i)
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
                            for (uint32_t i = 0; i < 3; ++i)
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

                                    // TODO: Account for up axis
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

                                    const float radius = glm::length(v.Position.xyz());
                                    if (radius > *a_radius)
                                    {
                                        *a_radius = radius;
                                    }
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
                }
            }

            return true;
        }

        return false;
    }
    bool ColladaLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path);

            if (file.good() && file.is_open())
            {
                IDEFER(file.close());

                // Read comment in ColladaLoader_LoadBoneFile
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios::beg);

                char* dat = new char[size];
                IDEFER(delete[] dat);
                file.read(dat, size);

                return ColladaLoader_LoadData(dat, (uint32_t)size, a_vertices, a_indices, a_radius);
            }
        }

        return false;
    }

    void ColladaLoader_BuildJointMap(const ColladaSceneNode& a_node, uint32_t* a_index, std::unordered_map<std::string, uint32_t>* a_map)
    {
        if (a_node.Type == "JOINT")
        {
            a_map->emplace(a_node.Name, (*a_index)++);
        }

        for (const ColladaSceneNode& n : a_node.Children)
        {
            ColladaLoader_BuildJointMap(n, a_index, a_map);
        }
    }
    void ColladaLoader_BuildJointData(const ColladaSceneNode& a_node, uint32_t* a_index, uint32_t a_parent, std::vector<BoneData>* a_dat)
    {
        uint32_t pIndex = a_parent;
        if (a_node.Type == "JOINT")
        {
            BoneData d;
            d.Parent = a_parent;
            d.Name = a_node.Name;
            for (int i = 0; i < 16; ++i)
            {
                d.Transform[i / 4][i % 4] = a_node.Transform[i];
            }

            a_dat->emplace_back(d);

            pIndex = (*a_index)++;
        }

        for (const ColladaSceneNode& n : a_node.Children)
        {
            ColladaLoader_BuildJointData(n, a_index, pIndex, a_dat);
        }
    }

    bool ColladaLoader_LoadSkinnedData(const char* a_data, uint32_t a_size, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        tinyxml2::XMLDocument doc;
        if (doc.Parse(a_data, (size_t)a_size) == tinyxml2::XML_SUCCESS)
        {
            const tinyxml2::XMLElement* rootElement = doc.RootElement();

            e_ColladaUpAxis up = ColladaUpAxis_YUp;
            float scale = 1.0f;
            std::vector<ColladaGeometry> geometry;
            IDEFER(
            {
                for (const ColladaGeometry& g : geometry)
                {
                    for (const ColladaSource& s : g.Mesh.Sources)
                    {
                        switch (s.Data.Type)
                        {
                        case ColladaSourceDataType_Float:
                        {
                            delete[] (float*)s.Data.Data;

                            break;
                        }
                        case ColladaSourceDataType_Name:
                        {
                            delete[] (std::string*)s.Data.Data;

                            break;
                        }
                        default:
                        {
                            break;
                        }
                        }
                    }
                }
            });
            std::vector<ColladaController> controllers;
            IDEFER(
            {
                for (const ColladaController& c : controllers)
                {
                    for (const ColladaSource& s : c.Skin.Sources)
                    {
                        switch (s.Data.Type)
                        {
                        case ColladaSourceDataType_Float:
                        {
                            delete[] (float*)s.Data.Data;

                            break;
                        }
                        case ColladaSourceDataType_Name:
                        {
                            delete[] (std::string*)s.Data.Data;

                            break;
                        }
                        default:
                        {
                            break;
                        }
                        }
                    }
                }
            });
            std::vector<ColladaVisualScene> scenes;

            for (const tinyxml2::XMLElement* element = rootElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
            {
                const char* elementName = element->Value();

                if (strcmp(elementName, "asset") == 0)
                {
                    for (const tinyxml2::XMLElement* assetElement = element->FirstChildElement(); assetElement != nullptr; assetElement = assetElement->NextSiblingElement())
                    {
                        const char* name = assetElement->Value();
                        if (strcmp(name, "up_axis") == 0)
                        {
                            const char* value = assetElement->GetText();
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
                else if (strcmp(elementName, "library_controllers") == 0)
                {
                    controllers = LoadController(element);
                }
                else if (strcmp(elementName, "library_visual_scenes") == 0)
                {
                    scenes = LoadScene(element);
                }
            }

            std::unordered_map<std::string, uint32_t> jointMap;
            uint32_t jointIndex = 0;
            for (const ColladaVisualScene& s : scenes)
            {
                for (const ColladaSceneNode& n : s.Nodes)
                {
                    ColladaLoader_BuildJointMap(n, &jointIndex, &jointMap);
                }
            }

            for (const ColladaController& controller : controllers)
            {
                for (const ColladaGeometry& geom : geometry)
                {
                    const std::string geomID = "#" + geom.ID;
                    if (controller.ID == geomID)
                    {
                        if (geom.Mesh.Triangles.P.empty())
                        {
                            ColladaInput vertexInput;
                            ColladaInput posInput;
                            ColladaInput normInput;
                            ColladaInput texcoordInput;
                            ColladaInput weightInput;
                            ColladaInput jointInput;

                            for (const ColladaInput& input : geom.Mesh.Polylist.Inputs)
                            {
                                if (input.Semantic == "POSITION")
                                {
                                    posInput = input;
                                }
                                else if (input.Semantic == "NORMAL")
                                {
                                    normInput = input;
                                }
                                else if (input.Semantic == "TEXCOORD")
                                {
                                    texcoordInput = input;
                                }
                                else if (input.Semantic == "VERTEX")
                                {
                                    vertexInput = input;

                                    for (const ColladaInput& vInput : geom.Mesh.Vertices)
                                    {
                                        if (vInput.Semantic == "POSITION")
                                        {
                                            posInput = vInput;
                                            posInput.Offset = input.Offset;
                                        }
                                        else if (vInput.Semantic == "NORMAL")
                                        {
                                            normInput = vInput;
                                            normInput.Offset = input.Offset;
                                        }
                                        else if (vInput.Semantic == "TEXCOORD")
                                        {
                                            texcoordInput = vInput;
                                            texcoordInput.Offset = input.Offset;
                                        }
                                    }
                                }
                            }

                            for (const ColladaInput& input : controller.Skin.VertexWeights.Inputs)
                            {
                                if (input.Semantic == "WEIGHT")
                                {
                                    weightInput = input;
                                }
                                else if (input.Semantic == "JOINT")
                                {
                                    jointInput = input;
                                }
                            }

                            ColladaSource posSource;
                            ColladaSource normalSource;
                            ColladaSource texcoordSource;
                            ColladaSource weightSource;
                            ColladaSource jointSource;

                            for (const ColladaSource& d : geom.Mesh.Sources)
                            {
                                const std::string idStr = "#" + d.ID;
                                if (idStr == posInput.Source)
                                {
                                    posSource = d;
                                }
                                else if (idStr == normInput.Source)
                                {
                                    normalSource = d;
                                }
                                else if (idStr == texcoordInput.Source)
                                {
                                    texcoordSource = d;
                                }
                            }

                            for (const ColladaSource& d : controller.Skin.Sources)
                            {
                                const std::string idStr = "#" + d.ID;
                                if (idStr == weightInput.Source)
                                {
                                    weightSource = d;
                                }
                                else if (idStr == jointInput.Source)
                                {
                                    jointSource = d;
                                }   
                            }

                            glm::vec4* weights = new glm::vec4[controller.Skin.VertexWeights.Count];
                            IDEFER(delete[] weights);
                            glm::ivec4* joints = new glm::ivec4[controller.Skin.VertexWeights.Count];
                            IDEFER(delete[] joints);

                            memset(weights, 0, sizeof(glm::vec4) * controller.Skin.VertexWeights.Count);
                            memset(joints, 0, sizeof(glm::ivec4) * controller.Skin.VertexWeights.Count);

                            uint32_t index = 0;
                            const uint32_t count = (uint32_t)controller.Skin.VertexWeights.Inputs.size();
                            for (uint32_t i = 0; i < controller.Skin.VertexWeights.Count; ++i)
                            {
                                const uint32_t vCount = controller.Skin.VertexWeights.VCount[i];
                                ICARIAN_ASSERT(vCount <= 4);

                                for (uint32_t j = 0; j < vCount; ++j)
                                {
                                    const uint32_t iIndex = index + j * count;

                                    const uint32_t weightIndex = controller.Skin.VertexWeights.V[iIndex + weightInput.Offset];
                                    const uint32_t jointIndex = controller.Skin.VertexWeights.V[iIndex + jointInput.Offset];

                                    weights[i][j] = ((float*)weightSource.Data.Data)[(weightIndex * weightSource.Accessor.Stride) + weightInput.Offset];
                                    joints[i][j] = jointMap[((std::string*)jointSource.Data.Data)[(jointIndex * jointSource.Accessor.Stride) + jointInput.Offset]];
                                }

                                index += count * vCount;
                            }

                            int posVCount;
                            int normVCount;
                            int texVCount;

                            const int* posOffset = ColladaLoader_GetOffsets(posSource, &posVCount);
                            IDEFER(delete[] posOffset);
                            const int* normalOffset = ColladaLoader_GetOffsets(normalSource, &normVCount);
                            IDEFER(delete[] normalOffset);
                            const int* texcoordOffset = ColladaLoader_GetOffsets(texcoordSource, &texVCount);
                            IDEFER(delete[] texcoordOffset);

                            std::unordered_map<uint64_t, uint32_t> indexMap;

                            index = 0;
                            const uint32_t cIndexCount = (uint32_t)geom.Mesh.Polylist.Inputs.size();
                            for (const uint32_t vCount : geom.Mesh.Polylist.VCount)
                            {
                                uint32_t posIndices[4];
                                uint32_t normalIndices[4];
                                uint32_t texcoordIndices[4];
                                uint32_t vertexIndices[4];

                                // TODO: Need to account for sizes greater than 4
                                for (uint32_t i = 0; i < vCount; ++i)
                                {
                                    // Flip faces so back culling work correctly
                                    const uint32_t iIndex = index + ((vCount - 1) - i) * count;

                                    posIndices[i] = geom.Mesh.Polylist.P[iIndex + posInput.Offset];
                                    normalIndices[i] = geom.Mesh.Polylist.P[iIndex + normInput.Offset];
                                    texcoordIndices[i] = geom.Mesh.Polylist.P[iIndex + texcoordInput.Offset];
                                    vertexIndices[i] = geom.Mesh.Polylist.P[iIndex + vertexInput.Offset];
                                }

                                switch (vCount)
                                {
                                case 3:
                                {
                                    for (uint32_t i = 0; i < 3; ++i)
                                    {
                                        // https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
                                        const uint64_t abH = ((uint64_t)posIndices[i] + normalIndices[i]) * ((uint64_t)posIndices[i] + normalIndices[i] + 1) / 2 + normalIndices[i];
                                        const uint64_t bcH = ((uint64_t)abH + texcoordIndices[i]) * ((uint64_t)abH + texcoordIndices[i] + 1) / 2 + texcoordIndices[i];
                                        const uint64_t h = (bcH + vertexIndices[i]) * (bcH + vertexIndices[i] + 1) / 2 + vertexIndices[i];

                                        const auto iter = indexMap.find(h);
                                        if (iter != indexMap.end())
                                        {
                                            a_indices->emplace_back(iter->second);
                                        }
                                        else 
                                        {
                                            const uint32_t vIndex = (uint32_t)a_vertices->size();

                                            SkinnedVertex v = SkinnedVertex();

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

                                            const uint32_t iVIndex = vertexIndices[i];

                                            v.BoneWeights = weights[iVIndex];
                                            v.BoneIndices = joints[iVIndex];

                                            a_vertices->emplace_back(v);
                                            a_indices->emplace_back(vIndex);
                                            indexMap.emplace(h, vIndex);                                            
                                        }
                                    }

                                    break;
                                }
                                case 4: 
                                {
                                    // TODO: Implement me

                                    break;
                                }
                                default:
                                {
                                    // TODO: Implement me

                                    break;
                                }
                                }
                            }
                        }
                        else
                        {
                            ColladaInput vertexInput;
                            ColladaInput posInput;
                            ColladaInput normInput;
                            ColladaInput texcoordInput;
                            ColladaInput weightInput;
                            ColladaInput jointInput;

                            for (const ColladaInput& input : geom.Mesh.Triangles.Inputs)
                            {
                                if (input.Semantic == "POSITION")
                                {
                                    posInput = input;
                                }
                                else if (input.Semantic == "NORMAL")
                                {
                                    normInput = input;
                                }
                                else if (input.Semantic == "TEXCOORD")
                                {
                                    texcoordInput = input;
                                }
                                else if (input.Semantic == "VERTEX")
                                {
                                    vertexInput = input;

                                    for (const ColladaInput& vInput : geom.Mesh.Vertices)
                                    {
                                        if (vInput.Semantic == "POSITION")
                                        {
                                            posInput = vInput;
                                            posInput.Offset = input.Offset;
                                        }
                                        else if (vInput.Semantic == "NORMAL")
                                        {
                                            normInput = vInput;
                                            normInput.Offset = input.Offset;
                                        }
                                        else if (vInput.Semantic == "TEXCOORD")
                                        {
                                            texcoordInput = vInput;
                                            texcoordInput.Offset = input.Offset;
                                        }
                                    }
                                }
                            }

                            for (const ColladaInput& input : controller.Skin.VertexWeights.Inputs)
                            {
                                if (input.Semantic == "WEIGHT")
                                {
                                    weightInput = input;
                                }
                                else if (input.Semantic == "JOINT")
                                {
                                    jointInput = input;
                                }
                            }

                            ColladaSource posSource;
                            ColladaSource normalSource;
                            ColladaSource texcoordSource;
                            ColladaSource weightSource;
                            ColladaSource jointSource;

                            for (const ColladaSource& d : geom.Mesh.Sources)
                            {
                                const std::string idStr = "#" + d.ID;
                                if (idStr == posInput.Source)
                                {
                                    posSource = d;
                                }
                                else if (idStr == normInput.Source)
                                {
                                    normalSource = d;
                                }
                                else if (idStr == texcoordInput.Source)
                                {
                                    texcoordSource = d;
                                }
                            }

                            for (const ColladaSource& d : controller.Skin.Sources)
                            {
                                const std::string idStr = "#" + d.ID;
                                if (idStr == weightInput.Source)
                                {
                                    weightSource = d;
                                }
                                else if (idStr == jointInput.Source)
                                {
                                    jointSource = d;
                                }
                            }

                            glm::vec4* weights = new glm::vec4[controller.Skin.VertexWeights.Count];
                            IDEFER(delete[] weights);
                            glm::ivec4* joints = new glm::ivec4[controller.Skin.VertexWeights.Count];
                            IDEFER(delete[] joints);

                            memset(weights, 0, sizeof(glm::vec4) * controller.Skin.VertexWeights.Count);
                            memset(joints, 0, sizeof(glm::ivec4) * controller.Skin.VertexWeights.Count);

                            uint32_t index = 0;
                            const uint32_t count = (uint32_t)controller.Skin.VertexWeights.Inputs.size();
                            for (uint32_t i = 0; i < controller.Skin.VertexWeights.Count; ++i)
                            {
                                const uint32_t vCount = controller.Skin.VertexWeights.VCount[i];
                                ICARIAN_ASSERT(vCount <= 4);
                                for (uint32_t j = 0; j < vCount; ++j)
                                {
                                    const uint32_t iIndex = index + j * count;

                                    const uint32_t weightIndex = controller.Skin.VertexWeights.V[iIndex + weightInput.Offset];
                                    const uint32_t jointIndex = controller.Skin.VertexWeights.V[iIndex + jointInput.Offset];

                                    const std::string& jointName = ((std::string*)jointSource.Data.Data)[jointIndex];

                                    weights[i][j] = ((float*)weightSource.Data.Data)[weightIndex];
                                    joints[i][j] = (int)jointMap[jointName];
                                }

                                index += vCount * count;
                            }

                            int posVCount;
                            int normVCount;
                            int texVCount;

                            const int* posOffset = ColladaLoader_GetOffsets(posSource, &posVCount);
                            IDEFER(delete[] posOffset);
                            const int* normalOffset = ColladaLoader_GetOffsets(normalSource, &normVCount);
                            IDEFER(delete[] normalOffset);
                            const int* texcoordOffset = ColladaLoader_GetOffsets(texcoordSource, &texVCount);
                            IDEFER(delete[] texcoordOffset);

                            std::unordered_map<uint64_t, uint32_t> indexMap;

                            const uint32_t cIndexCount = (uint32_t)geom.Mesh.Triangles.P.size();
                            const uint32_t cIndexStride = (uint32_t)geom.Mesh.Triangles.Inputs.size();
                            const uint32_t cNext = cIndexStride * 3;
                            for (uint32_t i = 0; i < cIndexCount; i += cNext)
                            {
                                for (uint32_t j = 0; j < 3; ++j)
                                {
                                    const uint32_t index = i + (2 - j) * cIndexStride;

                                    const uint32_t posIndex = geom.Mesh.Triangles.P[index + posInput.Offset];
                                    const uint32_t normIndex = geom.Mesh.Triangles.P[index + normInput.Offset];
                                    const uint32_t texIndex = geom.Mesh.Triangles.P[index + texcoordInput.Offset];
                                    const uint32_t vertexIndex = geom.Mesh.Triangles.P[index + vertexInput.Offset];

                                    // https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
                                    const uint64_t abH = ((uint64_t)posIndex + normIndex) * ((uint64_t)posIndex + normIndex + 1) / 2 + normIndex;
                                    const uint64_t bcH = ((uint64_t)abH + texIndex) * ((uint64_t)abH + texIndex + 1) / 2 + texIndex;
                                    const uint64_t h = (bcH + vertexIndex) * (bcH + vertexIndex + 1) / 2 + vertexIndex;

                                    const auto iter = indexMap.find(h);
                                    if (iter != indexMap.end())
                                    {
                                        a_indices->emplace_back(iter->second);
                                    }
                                    else 
                                    {
                                        const uint32_t vIndex = (uint32_t)a_vertices->size();

                                        SkinnedVertex v = SkinnedVertex();

                                        for (int j = 0; j < posVCount; ++j)
                                        {
                                            if (posOffset[j] == 1)
                                            {
                                                v.Position[posOffset[j]] = -((float*)posSource.Data.Data)[(posIndex * posSource.Accessor.Stride) + j] * scale;
                                            }
                                            else
                                            {
                                                v.Position[posOffset[j]] = ((float*)posSource.Data.Data)[(posIndex * posSource.Accessor.Stride) + j] * scale;
                                            }
                                        }

                                        for (int j = 0; j < normVCount; ++j)
                                        {
                                            if (normalOffset[j] == 1)
                                            {
                                                v.Normal[normalOffset[j]] = ((float*)normalSource.Data.Data)[(normIndex * normalSource.Accessor.Stride) + j];
                                            }
                                            else
                                            {
                                                v.Normal[normalOffset[j]] = -((float*)normalSource.Data.Data)[(normIndex * normalSource.Accessor.Stride) + j];
                                            }
                                        }

                                        for (int j = 0; j < texVCount; ++j)
                                        {
                                            v.TexCoords[texcoordOffset[j]] = ((float*)texcoordSource.Data.Data)[(texIndex * texcoordSource.Accessor.Stride) + j];
                                        }

                                        v.BoneWeights = weights[vertexIndex];
                                        v.BoneIndices = joints[vertexIndex];

                                        a_vertices->emplace_back(v);
                                        a_indices->emplace_back(vIndex);
                                        indexMap.emplace(h, vIndex);
                                    }
                                }
                            }
                        }

                        break;
                    }
                }
            }

            return true;
        }

        return false;
    }
    bool ColladaLoader_LoadSkinnedFile(const std::filesystem::path& a_path, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path);

            if (file.good() && file.is_open())
            {
                // Read comment in ColladaLoader_LoadBoneFile
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios::beg);

                char* dat = new char[size];
                IDEFER(delete[] dat);
                file.read(dat, size);

                return ColladaLoader_LoadSkinnedData(dat, (uint32_t)size, a_vertices, a_indices, a_radius);
            }
        }

        return false;
    }

    bool ColladaLoader_LoadBoneData(const char* a_data, uint32_t a_size, std::vector<BoneData>* a_bones)
    {
        tinyxml2::XMLDocument doc;
        if (doc.Parse(a_data, (size_t)a_size) == tinyxml2::XML_SUCCESS)
        {
            const tinyxml2::XMLElement* rootElement = doc.RootElement();

            std::vector<ColladaVisualScene> scenes;

            for (const tinyxml2::XMLElement* element = rootElement->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
            {
                const char* elementName = element->Value();

                if (strcmp(elementName, "library_visual_scenes") == 0)
                {
                    scenes = LoadScene(element);
                }
            }

            uint32_t jointIndex = 0;
            for (const ColladaVisualScene& s : scenes)
            {
                for (const ColladaSceneNode& n : s.Nodes)
                {
                    ColladaLoader_BuildJointData(n, &jointIndex, -1, a_bones);
                }
            }

            return true;
        }

        return false;
    }
    bool ColladaLoader_LoadBoneFile(const std::filesystem::path& a_path, std::vector<BoneData>* a_bones)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path);

            if (file.good() && file.is_open())
            {
                IDEFER(file.close());
                // Well that is time out of my life I am never getting back
                // Apparently std::filesystem::file_size does not report the correct size of the file on WIN32 systems
                // Only discovered this after getting back weird sizes and reading an article trying to figure out why
                // Fuck Windows and fuck the STL
                // const uint32_t size = (uint32_t)std::filesystem::file_size(a_path);
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios::beg);

                char* dat = new char[size];
                IDEFER(delete[] dat);
                file.read(dat, size);

                return ColladaLoader_LoadBoneData(dat, (uint32_t)size, a_bones);
            }
        }

        return false;
    }
}
