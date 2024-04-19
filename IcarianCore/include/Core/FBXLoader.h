#pragma once

#include <filesystem>
#include <vector>

#include "Core/Bones.h"

#include "EngineModelInteropStructures.h"

namespace IcarianCore
{
    struct FBXAnimationFrame
    {
        float Time;
        glm::vec4 Data;
    };

    struct FBXAnimationData
    {
        std::string Name;
        std::string PropertyName;
        std::vector<FBXAnimationFrame> Frames;
    };

    bool FBXLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
    bool FBXLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);

    bool FBXLoader_LoadSkinnedData(const char* a_data, uint32_t a_size, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
    bool FBXLoader_LoadSkinnedFile(const std::filesystem::path& a_path, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);

    bool FBXLoader_LoadBoneData(const char* a_data, uint32_t a_size, std::vector<BoneData>* a_bones);
    bool FBXLoader_LoadBoneFile(const std::filesystem::path& a_path, std::vector<BoneData>* a_bones);

    bool FBXLoader_LoadAnimationData(const char* a_data, uint32_t a_size, std::vector<FBXAnimationData>* a_animation);
    bool FBXLoader_LoadAnimationFile(const std::filesystem::path& a_path, std::vector<FBXAnimationData>* a_animation);
}