// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "AI/NavigationBindings.h"

#include "AI/Navigation.h"
#include "AI/NavigationMesh.h"
#include "IcarianError.h"
#include "Runtime/RuntimeManager.h"

static NavigationBindings* Instance = nullptr;

#include "EngineNavigationMeshInterop.h"

ENGINE_NAVIGATIONMESH_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(MonoArray*, Navigation, GetPath, 
{
    const Array<glm::vec3> path = Instance->GetNavigationPath(a_startPoint, a_endPoint, a_agentRadius);
    const uint32_t count = path.Size();
    MonoClass* klass = RuntimeManager::GetClass("IcarianEngine.Maths", "Vector3");
    MonoArray* arr = mono_array_new(mono_domain_get(), klass, count);
    for (uint32_t i = 0; i < count; ++i)
    {
        mono_array_set(arr, glm::vec3, i, path[i]);
    }
    return arr;
}, glm::vec3 a_startPoint, glm::vec3 a_endPoint, float a_agentRadius)

NavigationBindings::NavigationBindings(Navigation* a_navigation)
{
    m_navigation = a_navigation;
    
    ENGINE_NAVIGATIONMESH_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

    BIND_FUNCTION(IcarianEngine.AI, Navigation, GetPath);

    Instance = this;
}
NavigationBindings::~NavigationBindings()
{

}

uint32_t NavigationBindings::CreateNavMesh(const std::filesystem::path& a_path) const
{
    NavigationMesh* mesh = new NavigationMesh(a_path);

    return m_navigation->m_meshes.PushVal(mesh);
}
void NavigationBindings::DestroyNavMesh(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_navigation->m_meshes.Size());
    IVERIFY(m_navigation->m_meshes.Exists(a_addr));

    const NavigationMesh* mesh = m_navigation->m_meshes[a_addr];
    IDEFER(delete mesh);
    m_navigation->m_meshes.Erase(a_addr);
}
Array<glm::vec3> NavigationBindings::GetNavMeshPath(uint32_t a_addr, const glm::vec3& a_startPoint, const glm::vec3& a_endPoint, float a_agentRadius) const
{
    IVERIFY(a_addr < m_navigation->m_meshes.Size());
    IVERIFY(m_navigation->m_meshes.Exists(a_addr));

    const TReadLockArray<NavigationMesh*> a = m_navigation->m_meshes.ToReadLockArray();

    const NavigationMesh* mesh = a[a_addr];

    return mesh->GeneratePath(a_startPoint, a_endPoint, a_agentRadius);
}

Array<glm::vec3> NavigationBindings::GetNavigationPath(const glm::vec3& a_startPoint, const glm::vec3& a_endPoint, float a_agentRadius) const
{
    const TReadLockArray<NavigationMesh*> a = m_navigation->m_meshes.ToReadLockArray();
    const Array<bool> state = m_navigation->m_meshes.ToStateArray();

    const uint32_t size = a.Size();

    for (uint32_t i = 0; i < size; ++i)
    {
        if (!state[i])
        {
            continue;
        }

        const NavigationMesh* mesh = a[i];

        const uint32_t startIndex = mesh->GetIndex(a_startPoint);
        const uint32_t endIndex = mesh->GetIndex(a_endPoint);

        if (startIndex != -1 && endIndex != -1)
        {
            return mesh->GeneratePath(a_startPoint, a_endPoint, startIndex, endIndex, a_agentRadius);
        }
    }
    
    return Array<glm::vec3>();
}

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.