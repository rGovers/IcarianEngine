#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>

#include "DataTypes/Array.h"

class Navigation;

class NavigationBindings
{
private:
    Navigation* m_navigation;

protected:

public:
    NavigationBindings(Navigation* a_navigation);
    ~NavigationBindings();

    uint32_t CreateNavMesh(const std::filesystem::path& a_path) const;
    void DestroyNavMesh(uint32_t a_addr) const;
    Array<glm::vec3> GetNavMeshPath(uint32_t a_addr, const glm::vec3& a_startPoint, const glm::vec3& a_endPoint) const;

    Array<glm::vec3> GetNavigationPath(const glm::vec3& a_startPoint, const glm::vec3& a_endPoint) const;
};
