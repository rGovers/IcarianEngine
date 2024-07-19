#pragma once

class NavigationBindings;
class NavigationMesh;

#include "DataTypes/TNCArray.h"

class Navigation
{
private:
    friend class NavigationBindings;

    NavigationBindings*       m_bindings;

    TNCArray<NavigationMesh*> m_meshes;

protected:

public:
    Navigation();
    ~Navigation();
};