#pragma once

#include <filesystem>
#include <vector>

#include "Flare/Bones.h"

#include "EngineModelInteropStructures.h"

struct GLTFAnimationFrame
{
    float Time;
    glm::vec4 Data;
};

struct GLTFAnimationData
{
    std::string Name;
    std::string Target;
    std::vector<GLTFAnimationFrame> Frames;
};

namespace FlareBase 
{
    bool GLTFLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
    bool GLTFLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);

    bool GLTFLoader_LoadSkinnedData(const char* a_data, uint32_t a_size, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
    bool GLTFLoader_LoadSkinnedFile(const std::filesystem::path& a_path, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);

    bool GLTFLoader_LoadBones(const char* a_data, uint32_t a_size, std::vector<BoneData>* a_bones);
    bool GLTFLoader_LoadBonesFile(const std::filesystem::path& a_path, std::vector<BoneData>* a_bones);

    bool GLTFLoader_LoadAnimationData(const char* a_data, uint32_t a_size, std::vector<GLTFAnimationData>* a_animation);
    bool GLTFLoader_LoadAnimationFile(const std::filesystem::path& a_path, std::vector<GLTFAnimationData>* a_animation);
}