#pragma once

#include <filesystem>
#include <vector>

#include "Flare/Bones.h"

#include "EngineModelInteropStructures.h"

namespace FlareBase 
{
    bool GLTFLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
    bool GLTFLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
}