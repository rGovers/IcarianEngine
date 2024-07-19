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
    const Array<glm::vec3> path = Instance->GetNavigationPath(a_startPoint, a_endPoint);
    const uint32_t count = path.Size();
    MonoClass* klass = RuntimeManager::GetClass("IcarianEngine.Maths", "Vector3");
    MonoArray* arr = mono_array_new(mono_domain_get(), klass, count);
    for (uint32_t i = 0; i < count; ++i)
    {
        mono_array_set(arr, glm::vec3, i, path[i]);
    }
    return arr;
}, glm::vec3 a_startPoint, glm::vec3 a_endPoint)

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
Array<glm::vec3> NavigationBindings::GetNavMeshPath(uint32_t a_addr, const glm::vec3& a_startPoint, const glm::vec3& a_endPoint) const
{
    IVERIFY(a_addr < m_navigation->m_meshes.Size());
    IVERIFY(m_navigation->m_meshes.Exists(a_addr));

    const TReadLockArray<NavigationMesh*> a = m_navigation->m_meshes.ToReadLockArray();

    const NavigationMesh* mesh = a[a_addr];

    return mesh->GeneratePath(a_startPoint, a_endPoint);
}

Array<glm::vec3> NavigationBindings::GetNavigationPath(const glm::vec3& a_startPoint, const glm::vec3& a_endPoint) const
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
            return mesh->GeneratePath(a_startPoint, a_endPoint, startIndex, endIndex);
        }
    }
    
    return Array<glm::vec3>();
}