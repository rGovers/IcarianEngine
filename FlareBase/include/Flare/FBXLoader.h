#pragma once

#include <filesystem>
#include <vector>

#include "EngineModelInteropStructures.h"

namespace FlareBase
{
    bool FBXLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
    bool FBXLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
}