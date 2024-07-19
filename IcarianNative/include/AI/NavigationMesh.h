#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <filesystem>

#include "DataTypes/Array.h"

struct NavigationFace
{
    uint32_t Indicies[3];
    uint32_t Connections[3];
    glm::vec3 Center;
    // Do not care about the normal only the gradient
    float Dot;
};

class NavigationMesh
{
private:
    uint32_t        m_vertexCount;
    glm::vec3*      m_vertices;

    uint32_t        m_faceCount;
    NavigationFace* m_faces;

protected:

public:
    NavigationMesh(const std::filesystem::path& a_path);
    ~NavigationMesh();

    uint32_t GetIndex(const glm::vec3& a_point) const;

    Array<glm::vec3> GeneratePath(const glm::vec3& a_startPoint, const glm::vec3& a_endPoint) const;
    Array<glm::vec3> GeneratePath(const glm::vec3& a_startPoint, const glm::vec3& a_endPoint, uint32_t a_startIndex, uint32_t a_endIndex) const;
};