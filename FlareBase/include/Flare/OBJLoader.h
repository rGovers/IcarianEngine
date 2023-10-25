#pragma once

#include <filesystem>
#include <vector>

#include "EngineModelInteropStructures.h"

namespace FlareBase
{
    bool OBJLoader_LoadData(const char* a_data, uint32_t a_size, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
    bool OBJLoader_LoadFile(const std::filesystem::path& a_path, std::vector<Vertex>* a_vertices, std::vector<uint32_t>* a_indices, float* a_radius);
}