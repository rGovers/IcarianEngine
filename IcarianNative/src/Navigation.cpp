#include "AI/Navigation.h"

#include "AI/NavigationBindings.h"
#include "AI/NavigationMesh.h"

Navigation::Navigation()
{
    m_bindings = new NavigationBindings(this);
}
Navigation::~Navigation()
{
    delete m_bindings;

    for (uint32_t i = 0; i < m_meshes.Size(); ++i)
    {
        if (m_meshes.Exists(i))
        {
            delete m_meshes[i];
            m_meshes[i] = nullptr;
        }
    }
}
