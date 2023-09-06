#pragma once

#include <filesystem>
#include <vector>

#include "Vertices.h"

#include "Flare/Bones.h"

struct ColladaAnimationFrame
{
    float Time;
    glm::mat4 Transform;
};
struct ColladaAnimationData
{
    std::string Name;
    std::vector<ColladaAnimationFrame> Frames;
};

namespace FlareBase
{
    bool ColladaLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
    bool ColladaLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);

    bool ColladaLoader_LoadSkinnedData(const char* a_data, uint32_t a_size, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
    bool ColladaLoader_LoadSkinnedFile(const std::filesystem::path& a_path, std::vector<SkinnedVertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);

    bool ColladaLoader_LoadBoneData(const char* a_data, uint32_t a_size, std::vector<BoneData>* a_bones);
    bool ColladaLoader_LoadBoneFile(const std::filesystem::path& a_path, std::vector<BoneData>* a_bones);

    bool ColladaLoader_LoadAnimationData(const char* a_data, uint32_t a_size, std::vector<ColladaAnimationData>* a_animation);
    bool ColladaLoader_LoadAnimationFile(const std::filesystem::path& a_path, std::vector<ColladaAnimationData>* a_animation);
}