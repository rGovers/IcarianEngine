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

    static void FillFBXCurve(const ofbx::AnimationCurve* a_curve, std::vector<FBXAnimationFrame>* a_frames, int a_index, float a_frameRate, float a_scale)
    {
        const int keyCount = a_curve->getKeyCount();
        const ofbx::i64* keyTimes = a_curve->getKeyTime();
        const float* keyValues = a_curve->getKeyValue();
        for (int i = 0; i < keyCount - 1; ++i)
        {
            const ofbx::i64 keyTime = keyTimes[i];
            const ofbx::i64 nextKeyTime = keyTimes[i + 1];

            const double startTime = ofbx::fbxTimeToSeconds(keyTime);
            const double endTime = ofbx::fbxTimeToSeconds(nextKeyTime);
            const double diff = endTime - startTime;

            const float keyValue = keyValues[i];
            const float nextKeyValue = keyValues[i + 1];

            uint32_t frameCount = (uint32_t)(diff * a_frameRate + 0.1f);
            const uint32_t startFrame = (uint32_t)(startTime * a_frameRate);
            const int64_t frameDiff = (int64_t)(frameCount + startFrame) - (int64_t)a_frames->size();
            if (frameDiff > 0)
            {
                frameCount = (uint32_t)glm::max(frameCount - frameDiff, int64_t(0));
            } 

            for (uint32_t j = 0; j < frameCount; ++j)
            {
                const uint32_t index = startFrame + j;
                FBXAnimationFrame& frame = a_frames->at(index);

                const float lerp = (float)j / frameCount;
                const float value = glm::mix(keyValue, nextKeyValue, lerp);

                frame.Time = index / a_frameRate;
                frame.Data[a_index] = value * a_scale;
            }
        }

        const ofbx::i64 keyTime = keyTimes[keyCount - 1];
        const double startTime = ofbx::fbxTimeToSeconds(keyTime);
        const uint32_t startFrame = (uint32_t)(startTime * a_frameRate);
        FBXAnimationFrame& frame = a_frames->at(startFrame);
        frame.Time = startFrame / a_frameRate;
        frame.Data[a_index] = keyValues[keyCount - 1] * a_scale;
    }    

    bool FBXLoader_LoadAnimationData(const char* a_data, uint32_t a_size, std::vector<FBXAnimationData>* a_animation)
    {
        ofbx::IScene* scene = ofbx::load((ofbx::u8*)a_data, a_size, 0);

        if (scene == nullptr)
        {
            return false;
        }
        IDEFER(scene->destroy());

        const ofbx::GlobalSettings* settings = scene->getGlobalSettings();
        const float frameRate = scene->getSceneFrameRate();
        const double duration = settings->TimeSpanStop - settings->TimeSpanStart;
        const uint32_t totalFrameCount = (uint32_t)(duration * frameRate) + 10;

        // If memory serves me correctly, the unit scale factor is based off of centimeters.
        const float trueScale = settings->UnitScaleFactor * 0.01f;

        const int animStackCount = scene->getAnimationStackCount();
        for (int i = 0; i < animStackCount; ++i)
        {
            const ofbx::AnimationStack* animStack = scene->getAnimationStack(i);

            const ofbx::AnimationLayer* animLayer = animStack->getLayer(0);
            if (animLayer == nullptr)
            {
                continue;
            }

            for (int j = 0; animLayer->getCurveNode(j) != nullptr; ++j)
            {
                const ofbx::AnimationCurveNode* animCurveNode = animLayer->getCurveNode(j);
                const ofbx::Object* bone = animCurveNode->getBone();
                
                const ofbx::DataView boneLinkProperty = animCurveNode->getBoneLinkProperty();

                if (boneLinkProperty == "Lcl Translation")
                {
                    FBXAnimationFrame defaultFrame;
                    defaultFrame.Time = -1.0f;
                    defaultFrame.Data = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

                    FBXAnimationData animData;
                    animData.Name = bone->name;
                    animData.PropertyName = "Translation";
                    animData.Frames.resize(totalFrameCount, defaultFrame);

                    for (int k = 0; k < 3; ++k)
                    {
                        const ofbx::AnimationCurve* animCurve = animCurveNode->getCurve(k);
                        if (animCurve == nullptr)
                        {
                            continue;
                        }

                        FillFBXCurve(animCurve, &animData.Frames, k, frameRate, trueScale);
                    }

                    auto iter = animData.Frames.begin();
                    while (iter != animData.Frames.end())
                    {   
                        if (iter->Time < 0.0f)
                        {
                            iter = animData.Frames.erase(iter);

                            continue;
                        }
                        
                        iter->Data.y = -iter->Data.y;

                        ++iter;
                    }

                    a_animation->push_back(animData);
                }
                else if (boneLinkProperty == "Lcl Rotation")
                {
                    FBXAnimationFrame defaultFrame;
                    defaultFrame.Time = -1.0f;
                    defaultFrame.Data = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

                    FBXAnimationData animData;
                    animData.Name = bone->name;
                    animData.PropertyName = "Rotation";
                    animData.Frames.resize(totalFrameCount, defaultFrame);

                    for (int k = 0; k < 3; ++k)
                    {
                        const ofbx::AnimationCurve* animCurve = animCurveNode->getCurve(k);
                        if (animCurve == nullptr)
                        {
                            continue;
                        }

                        FillFBXCurve(animCurve, &animData.Frames, k, frameRate, glm::pi<float>() / 180.0f);
                    }

                    // const ofbx::DVec3 localBonePos = bone->getLocalTranslation();
                    const ofbx::DMatrix localBoneMat = bone->getLocalTransform();
                    glm::mat4 gLocalBoneMat;
                    float* t = (float*)&gLocalBoneMat;
                    for (uint32_t k = 0; k < 16; ++k) 
                    {
                        t[k] = (float)localBoneMat.m[k];
                    }

                    glm::vec3 translation;
                    glm::quat rotation;
                    glm::vec3 scale;
                    glm::vec3 skew;
                    glm::vec4 perspective;

                    glm::decompose(gLocalBoneMat, scale, rotation, translation, skew, perspective);

                    const glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, 1.0f);
                    const glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);

                    const glm::vec3 tForward = glm::vec3(forward.x, -forward.y, forward.z);
                    const glm::vec3 tRight = glm::vec3(right.x, -right.y, right.z);
                    const glm::vec3 tUp = glm::cross(tForward, tRight);

                    const glm::mat4 rotMat = glm::mat4(glm::vec4(tRight, 0.0f), glm::vec4(tUp, 0.0f), glm::vec4(tForward, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

                    auto iter = animData.Frames.begin();
                    while (iter != animData.Frames.end())
                    {                        
                        if (iter->Time < 0.0f)
                        {
                            iter = animData.Frames.erase(iter);

                            continue;
                        }
                        
                        const glm::vec3 euler = iter->Data.xyz();

                        // ofbx::DVec3 dEuler;
                        // dEuler.x = -euler.x;
                        // dEuler.y = -euler.y;
                        // dEuler.z = euler.z + 180.0;

                        // ofbx::DVec3 zero;
                        // zero.x = 0.0;
                        // zero.y = 0.0;
                        // zero.z = 0.0;

                        // const ofbx::DMatrix mtx = bone->evalLocal(zero, dEuler);
                        // glm::mat4 gMtx;
                        // float* t = (float*)&gMtx;
                        // for (uint32_t i = 0; i < 16; ++i) 
                        // {
                        //     t[i] = (float)mtx.m[i];
                        // }

                        // glm::vec3 translation;
                        // glm::quat rotation;
                        // glm::vec3 scale;
                        // glm::vec3 skew;
                        // glm::vec4 perspective;

                        // glm::decompose(gMtx, scale, rotation, translation, skew, perspective);

                        // const glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, 1.0f);
                        // const glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);

                        // const glm::vec3 tForward = glm::vec3(forward.x, -forward.y, forward.z);
                        // const glm::vec3 tRight = glm::vec3(right.x, -right.y, right.z);
                        // const glm::vec3 tUp = glm::cross(tForward, tRight);

                        // const glm::mat4 mat = glm::mat4(glm::vec4(tRight, 0.0f), glm::vec4(tUp, 0.0f), glm::vec4(tForward, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

                        // ofbx::DVec3 dEuler;
                        // dEuler.x = -euler.x;
                        // dEuler.y = -euler.y;
                        // dEuler.z = euler.z + 180.0;

                        // ofbx::DVec3 zero;
                        // zero.x = 0.0;
                        // zero.y = 0.0;
                        // zero.z = 0.0;

                        // const ofbx::DMatrix mtx = bone->evalLocal(zero, dEuler);
                        // glm::mat4 gMtx;
                        // float* t = (float*)&gMtx;
                        // for (uint32_t i = 0; i < 16; ++i) 
                        // {
                        //     t[i] = (float)mtx.m[i];
                        // }

                        // glm::vec3 translation;
                        // glm::quat rotation;
                        // glm::vec3 scale;
                        // glm::vec3 skew;
                        // glm::vec4 perspective;

                        // glm::decompose(gMtx, scale, rotation, translation, skew, perspective);

                        // const glm::vec3 forward = rotation * glm::vec3(0.0f, 0.0f, 1.0f);
                        // const glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);

                        // const glm::vec3 tForward = glm::vec3(forward.x, -forward.y, forward.z);
                        // const glm::vec3 tRight = glm::vec3(right.x, -right.y, right.z);
                        // const glm::vec3 tUp = glm::cross(tForward, tRight);

                        // const glm::mat4 mat = glm::mat4(glm::vec4(tRight, 0.0f), glm::vec4(tUp, 0.0f), glm::vec4(tForward, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

                        // const glm::quat quat = glm::toQuat(mat);

                        // iter->Data.x = quat.x;
                        // iter->Data.y = quat.y;
                        // iter->Data.z = quat.z;
                        // iter->Data.w = quat.w;

                        // I Give up FBX wins this round I will where my dunce cap.
                        const glm::mat4 xRot = glm::rotate(euler.x, glm::vec3(1.0f, 0.0f, 0.0f));
                        const glm::mat4 yRot = glm::rotate(euler.y, glm::vec3(0.0f, 1.0f, 0.0f));
                        const glm::mat4 zRot = glm::rotate(euler.z, glm::vec3(0.0f, 0.0f, 1.0f));

                        const glm::mat4 rot = (zRot * yRot * xRot) * rotMat;
                        const glm::quat quat = glm::toQuat(rot);

                        iter->Data.x = quat.x;
                        iter->Data.y = quat.y;
                        iter->Data.z = quat.z;
                        iter->Data.w = quat.w;

                        ++iter;
                    }

                    a_animation->push_back(animData);
                }
            }
        }

        return true;   
    }
    bool FBXLoader_LoadAnimationFile(const std::filesystem::path& a_path, std::vector<FBXAnimationData>* a_animation)
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

                return FBXLoader_LoadAnimationData(data, (uint32_t)size, a_animation);
            }
        }

        return false;
    }
}