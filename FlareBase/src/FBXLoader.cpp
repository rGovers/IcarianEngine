#include "Flare/FBXLoader.h"

#include <cstring>
#include <fstream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
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
        if (scene != nullptr)
        {
            IDEFER(scene->destroy());

            int indexBuffer[1024];

            const ofbx::GlobalSettings* settings = scene->getGlobalSettings();

            // If memory serves me correctly, the unit scale factor is based off of centimeters.
            const float trueScale = settings->UnitScaleFactor * 0.01f;
            const int meshCount = scene->getMeshCount();

            for (int i = 0; i < meshCount; ++i)
            {
                // TODO: Another map that should be an array.
                std::unordered_map<int, uint32_t> indexMap;

                const ofbx::Mesh* mesh = scene->getMesh(i);
                const ofbx::GeometryData& geom = mesh->getGeometryData();
                const ofbx::Vec3Attributes positions = geom.getPositions();
                const ofbx::Vec3Attributes normals = geom.getNormals();
                const ofbx::Vec2Attributes uvs = geom.getUVs();
                const ofbx::Vec4Attributes colors = geom.getColors();

                const int partitionCount = geom.getPartitionCount();

                for (int j = 0; j < partitionCount; ++j)
                {
                    const ofbx::GeometryPartition& partition = geom.getPartition(j);
                    
                    for (int k = 0; k < partition.polygon_count; ++k)
                    {
                        const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[k];

                        const ofbx::u32 indexCount = ofbx::triangulate(geom, polygon, indexBuffer);

                        for (ofbx::u32 l = 0; l < indexCount; ++l)
                        {
                            const int index = indexBuffer[indexCount - l - 1];

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
            std::ifstream file = std::ifstream(a_path, std::ios_base::binary);

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

    bool FBXLoader_LoadSkinnedData(const char* a_data, uint32_t a_size, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        *a_radius = 0.0f;

        ofbx::IScene* scene = ofbx::load((ofbx::u8*)a_data, a_size, 0);
        if (scene != nullptr)
        {
            IDEFER(scene->destroy());

            const ofbx::GlobalSettings* settings = scene->getGlobalSettings();

            // If memory serves me correctly, the unit scale factor is based off of centimeters.
            const float trueScale = settings->UnitScaleFactor * 0.01f;
            const int meshCount = scene->getMeshCount();

            std::unordered_map<std::string, uint32_t> boneMap;
            uint32_t boneCount = 0;

            for (int i = 0; i < meshCount; ++i)
            {
                const ofbx::Mesh* mesh = scene->getMesh(i);
                const ofbx::Geometry* geometry = mesh->getGeometry();
                const ofbx::Skin* skin = geometry->getSkin();
                if (skin == nullptr)
                {
                    continue;
                }

                const ofbx::GeometryData& geom = geometry->getGeometryData();

                const ofbx::Vec3Attributes positions = geom.getPositions();
                
                glm::vec4* geomWeights = new glm::vec4[positions.values_count];
                IDEFER(delete[] geomWeights);
                memset(geomWeights, 0, sizeof(glm::vec4) * positions.values_count);
                glm::ivec4* geomIndices = new glm::ivec4[positions.values_count];
                IDEFER(delete[] geomIndices);
                memset(geomIndices, 0, sizeof(glm::ivec4) * positions.values_count);

                const int clusterCount = skin->getClusterCount();
                for (int j = 0; j < clusterCount; ++j)
                {
                    const ofbx::Cluster* cluster = skin->getCluster(j);
                    const int indexCount = cluster->getIndicesCount();
                    if (indexCount <= 0)
                    {
                        continue;
                    }

                    const ofbx::Object* link = cluster->getLink();

                    const char* name = link->name;
                    uint32_t boneIndex = boneCount;
                    
                    const auto iter = boneMap.find(name);
                    if (iter == boneMap.end())
                    {
                        boneMap.emplace(name, boneCount++);
                    }
                    else
                    {
                        boneIndex = iter->second;
                    }
                    
                    const int* indices = cluster->getIndices();
                    const double* weights = cluster->getWeights();
                    for (int k = 0; k < indexCount; ++k)
                    {
                        const int index = indices[k];
                        const double weight = weights[k];

                        glm::vec4& vWeight = geomWeights[index];
                        glm::ivec4& vIndex = geomIndices[index];

                        for (int l = 0; l < 4; ++l)
                        {
                            if (weight > vWeight[l])
                            {
                                for (int m = 3; m > l; --m)
                                {
                                    vIndex[m] = vIndex[m - 1];
                                    vWeight[m] = vWeight[m - 1];
                                }

                                vIndex[l] = (int)boneIndex;
                                vWeight[l] = (float)weight;
                                
                                break;
                            }
                        }
                    }
                }

                std::vector<uint32_t> indexMap;

                const ofbx::Vec3Attributes normals = geom.getNormals();
                const ofbx::Vec2Attributes uvs = geom.getUVs();
                const ofbx::Vec4Attributes colors = geom.getColors();

                const int partitionCount = geom.getPartitionCount();
                for (int j = 0; j < partitionCount; ++j)
                {
                    const ofbx::GeometryPartition& partition = geom.getPartition(j);
                    indexMap.resize(indexMap.size() + partition.triangles_count * 3 + 3, -1);
                    
                    int* indexBuffer = new int[partition.max_polygon_triangles * 3 + 3];
                    IDEFER(delete[] indexBuffer);

                    for (int k = 0; k < partition.polygon_count; ++k)
                    {
                        const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[k];

                        const ofbx::u32 indexCount = ofbx::triangulate(geom, polygon, indexBuffer);

                        for (ofbx::u32 l = 0; l < indexCount; ++l)
                        {
                            const int index = indexBuffer[indexCount - l - 1];
                            
                            const uint32_t tIndex = indexMap[index];
                            if (tIndex != -1)
                            {
                                a_indices->push_back(tIndex);

                                continue;
                            }

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

                            SkinnedVertex v;
                            v.Position = glm::vec4(pos, 1.0f);
                            v.Normal = norm;
                            v.TexCoords = uv;
                            v.Color = color;

                            if (positions.indices != nullptr)
                            {
                                const int tIndex = positions.indices[index];
                                v.BoneIndices = geomIndices[tIndex];
                                v.BoneWeights = geomWeights[tIndex];
                            }
                            else
                            {
                                v.BoneIndices = geomIndices[index];
                                v.BoneWeights = geomWeights[index];
                            }

                            const uint32_t vIndex = (uint32_t)a_vertices->size();

                            a_vertices->push_back(v);
                            a_indices->push_back(vIndex);

                            indexMap[index] = vIndex;
                        }
                    }
                }
            }

            return true;
        }

        return false;
    }

    bool FBXLoader_LoadSkinnedFile(const std::filesystem::path& a_path, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path, std::ios_base::binary);

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

                return FBXLoader_LoadSkinnedData(data, (uint32_t)size, a_vertices, a_indices, a_radius);
            }
        }

        return false;
    }

    bool FBXLoader_LoadBoneData(const char* a_data, uint32_t a_size, std::vector<BoneData>* a_bones)
    {
        ofbx::IScene* scene = ofbx::load((ofbx::u8*)a_data, a_size, 0);

        if (scene == nullptr)
        {
            return false;
        }
        IDEFER(scene->destroy());

        const ofbx::GlobalSettings* settings = scene->getGlobalSettings();

        // If memory serves me correctly, the unit scale factor is based off of centimeters.
        const float trueScale = settings->UnitScaleFactor * 0.01f;

        std::unordered_map<std::string, uint32_t> boneMap;
        std::vector<const ofbx::Object*> boneList;

        const int meshCount = scene->getMeshCount();
        for (uint32_t i = 0; i < meshCount; ++i)
        {
            const ofbx::Mesh* mesh = scene->getMesh(i);
            const ofbx::Geometry* geometry = mesh->getGeometry();
            const ofbx::Skin* skin = geometry->getSkin();
            if (skin == nullptr)
            {
                continue;
            }

            const int clusterCount = skin->getClusterCount();
            for (int j = 0; j < clusterCount; ++j)
            {
                const ofbx::Cluster* cluster = skin->getCluster(j);
                if (cluster->getIndicesCount() <= 0)
                {
                    continue;
                }

                const ofbx::Object* link = cluster->getLink();
                const char* name = link->name;
                if (boneMap.find(name) != boneMap.end())
                {
                    continue;
                }

                const ofbx::DMatrix linkMat = cluster->getTransformLinkMatrix();

                glm::mat4 gLinkMat;
                float* t = (float*)&gLinkMat;
                for (uint32_t k = 0; k < 16; ++k) 
                {
                    t[k] = (float)linkMat.m[k];
                }

                // So apparently the order is completely random, so we have to do multiple passes.
                boneMap.emplace(name, (uint32_t)boneList.size());
                boneList.push_back(link);
            }
        }

        a_bones->reserve(boneList.size());
        for (const ofbx::Object* link : boneList)
        {
            const ofbx::DMatrix linkMat = link->getGlobalTransform();

            glm::mat4 gLinkMat;
            float* t = (float*)&gLinkMat;
            for (uint32_t i = 0; i < 16; ++i) 
            {
                t[i] = (float)linkMat.m[i];
            }

            glm::vec3 translation;
            glm::quat rotation;
            glm::vec3 scale;
            glm::vec3 skew;
            glm::vec4 perspective;

            glm::decompose(gLinkMat, scale, rotation, translation, skew, perspective);

            const glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, 1.0f);
            const glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);

            const glm::vec3 tForward = glm::vec3(forward.x, -forward.y, forward.z);
            const glm::vec3 tRight = glm::vec3(right.x, -right.y, right.z);
            const glm::vec3 tUp = glm::cross(tForward, tRight);

            constexpr glm::mat4 Iden = glm::identity<glm::mat4>();

            const glm::mat4 translationMat = glm::translate(Iden, glm::vec3(translation.x, -translation.y, translation.z) * trueScale);
            const glm::mat4 rotationMat = glm::mat4(glm::vec4(tRight, 0.0f), glm::vec4(tUp, 0.0f), glm::vec4(tForward, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            const glm::mat4 scaleMat = glm::scale(Iden, scale);
            
            BoneData bone;
            bone.Name = link->name;
            bone.Parent = -1;
            bone.Transform = translationMat * rotationMat * scaleMat;

            const ofbx::Object* parent = link->getParent();
            if (parent != nullptr)
            {
                const auto iter = boneMap.find(parent->name);
                if (iter != boneMap.end())
                {
                    bone.Parent = iter->second;
                }
            }

            a_bones->push_back(bone);
        }

        return true;
    }
    bool FBXLoader_LoadBoneFile(const std::filesystem::path& a_path, std::vector<BoneData>* a_bones)
    {
        if (std::filesystem::exists(a_path))
        {
            std::ifstream file = std::ifstream(a_path, std::ios_base::binary);

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

                return FBXLoader_LoadBoneData(data, (uint32_t)size, a_bones);
            }
        }

        return false;
    }
}