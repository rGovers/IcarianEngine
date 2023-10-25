#include "Flare/FBXLoader.h"

#include <fstream>
#include <ofbx.h>
#include <unordered_map>

#include "Flare/IcarianDefer.h"

namespace FlareBase
{
    bool FBXLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        *a_radius = 0.0f;

        constexpr ofbx::LoadFlags Flags = 
            ofbx::LoadFlags::IGNORE_BLEND_SHAPES | 
            ofbx::LoadFlags::IGNORE_CAMERAS | 
            ofbx::LoadFlags::IGNORE_LIGHTS |
            ofbx::LoadFlags::IGNORE_TEXTURES | 
            ofbx::LoadFlags::IGNORE_SKIN | 
            ofbx::LoadFlags::IGNORE_BONES |
            ofbx::LoadFlags::IGNORE_PIVOTS |
            ofbx::LoadFlags::IGNORE_ANIMATIONS |
            ofbx::LoadFlags::IGNORE_MATERIALS |
            ofbx::LoadFlags::IGNORE_POSES |
            ofbx::LoadFlags::IGNORE_VIDEOS |
            ofbx::LoadFlags::IGNORE_LIMBS;

        ofbx::IScene* scene = ofbx::load((ofbx::u8*)a_data, a_size, (ofbx::u16)Flags);
        IDEFER(scene->destroy());

        int indexBuffer[1024];

        if (scene != nullptr)
        {
            const ofbx::GlobalSettings* settings = scene->getGlobalSettings();

            // If memory serves me correctly, the unit scale factor is based off of centimeters.
            const float trueScale = settings->UnitScaleFactor * 0.01f;
            const int meshCount = scene->getMeshCount();

            for (int i = 0; i < meshCount; ++i)
            {
                const ofbx::Mesh* mesh = scene->getMesh(i);
                const ofbx::GeometryData& geom = mesh->getGeometryData();
                const ofbx::Vec3Attributes positions = geom.getPositions();
                const ofbx::Vec3Attributes normals = geom.getNormals();
                const ofbx::Vec2Attributes uvs = geom.getUVs();
                const ofbx::Vec4Attributes colors = geom.getColors();

                const int partitionCount = geom.getPartitionCount();

                for (int j = 0; j < partitionCount; ++j)
                {
                    std::unordered_map<int, uint32_t> indexMap;

                    const ofbx::GeometryPartition& partition = geom.getPartition(j);
                    
                    for (int k = 0; k < partition.polygon_count; ++k)
                    {
                        const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[k];

                        const ofbx::u32 indexCount = ofbx::triangulate(geom, polygon, indexBuffer);

                        for (ofbx::u32 l = 0; l < indexCount; ++l)
                        {
                            const int index = indexBuffer[l];

                            const auto iter = indexMap.find(index);
                            if (iter != indexMap.end())
                            {
                                a_indices->push_back(iter->second);
                            }
                            else 
                            {
                                glm::vec3 pos;
                                glm::vec3 norm = glm::vec3(0.0f);
                                glm::vec2 uv = glm::vec2(0.0f);
                                glm::vec4 color = glm::vec4(1.0f);

                                const ofbx::Vec3 position = positions.get(index);

                                pos = glm::vec3(position.x, -position.y, position.z) * trueScale;
                                if (normals.values != nullptr)
                                {
                                    const ofbx::Vec3 normal = normals.get(index);
                                    norm = glm::vec3(normal.x, -normal.y, normal.z);
                                }

                                if (uvs.values != nullptr)
                                {
                                    const ofbx::Vec2 uvCoord = uvs.get(index);
                                    uv = glm::vec2(uvCoord.x, uvCoord.y);
                                }

                                if (colors.values != nullptr)
                                {
                                    const ofbx::Vec4 colorVec = colors.get(index);
                                    color = glm::vec4(colorVec.x, colorVec.y, colorVec.z, colorVec.w);
                                }

                                const float mag = glm::length(pos);
                                if (mag > *a_radius)
                                {
                                    *a_radius = mag;
                                }

                                Vertex v;
                                v.Position = glm::vec4(pos, 1.0f);
                                v.Normal = norm;
                                v.TexCoords = uv;
                                v.Color = color;

                                const uint32_t vIndex = (uint32_t)a_vertices->size();

                                a_vertices->push_back(v);
                                a_indices->push_back(vIndex);

                                indexMap.emplace(index, vIndex);
                            }
                        }
                    }
                }
            }

            return true;
        }

        return false;
    }

    bool FBXLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path);

            if (file.good() && file.is_open())
            {
                file.ignore(std::numeric_limits<std::streamsize>::max());
                const std::streamsize size = file.gcount();
                file.clear();
                file.seekg(0, std::ios_base::beg);

                if (size > UINT32_MAX)
                {
                    return false;
                }

                char* data = new char[size];
                IDEFER(delete[] data);
                file.read(data, size);

                return FBXLoader_LoadData(data, (uint32_t)size, a_vertices, a_indices, a_radius);
            }
        }

        return false;
    }
}