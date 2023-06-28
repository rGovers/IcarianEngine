#pragma once

#define GLM_SWIZZLE
#include <glm/glm.hpp>

struct Frustum
{
    glm::vec4 Planes[6];

    bool CompareSphere(const glm::vec3& a_pos, float a_radius) const
    {
        for (int i = 0; i < 6; ++i)
        {
            const float d = glm::dot(Planes[i].xyz(), a_pos) + Planes[i].w;

            if (d + a_radius + 0.01f < 0)
            {
                return false;
            }
        }

        return true;
    }

    static Frustum FromMat4(const glm::mat4& a_matrix)
    {
        Frustum result;

        for (int i = 0; i < 3; ++i)
        {
            result.Planes[i * 2] = glm::vec4
            (
                a_matrix[0][3] - a_matrix[0][i],
                a_matrix[1][3] - a_matrix[1][i],
                a_matrix[2][3] - a_matrix[2][i],
                a_matrix[3][3] - a_matrix[3][i]
            );
            result.Planes[i * 2] /= glm::length(result.Planes[i * 2].xyz());

            result.Planes[i * 2 + 1] = glm::vec4
            (
                a_matrix[0][3] + a_matrix[0][i],
                a_matrix[1][3] + a_matrix[1][i],
                a_matrix[2][3] + a_matrix[2][i],
                a_matrix[3][3] + a_matrix[3][i]
            );
            result.Planes[i * 2 + 1] /= glm::length(result.Planes[i * 2 + 1].xyz());
        }

        return result;
    }
};