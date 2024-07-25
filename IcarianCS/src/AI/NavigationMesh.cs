using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Mod;
using System;
using System.Runtime.CompilerServices;

#include "EngineNavigationMeshInterop.h"
#include "InteropBinding.h"

ENGINE_NAVIGATIONMESH_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.AI
{
    public class NavigationMesh : Component, IDestroy
    {
        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether the NavigationMesh has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// The Def used to create the NavigationMesh
        /// </summary>
        public NavigationMeshDef NavigationMeshDef
        {
            get
            {
                return Def as NavigationMeshDef;
            }
        }

        /// <summary>
        /// Called when the NavigationMesh is created
        /// </summary>
        public override void Init()
        {
            NavigationMeshDef def = NavigationMeshDef;
            if (def != null)
            {
                string path = ModControl.GetAssetPath(def.MeshPath);
                if (string.IsNullOrEmpty(path))
                {
                    Logger.IcarianError($"NavigationMesh failed to find mesh path: {def.MeshPath}");

                    return;
                }

                m_bufferAddr = NavigationMeshInterop.GenerateMesh(path);
            }
            else
            {
                Logger.IcarianError("NavigationMesh null Def");
            }
        }

        /// <summary>
        /// Gets a path between points
        /// </summary>
        /// <param name="a_startPoint">The starting point of the path</param>
        /// <param name="a_endPoint">The ending point of the path</param>
        /// <returns>The points that make up the path</returns>
        public Vector3[] GetPath(Vector3 a_startPoint, Vector3 a_endPoint, float a_agentRadius = 1.0f)
        {
            return NavigationMeshInterop.GetPath(m_bufferAddr, a_startPoint, a_endPoint, a_agentRadius);
        }

        /// <summary>
        /// Disposes of the NavigationMesh
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the NavigationMesh is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether it is being called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    NavigationMeshInterop.DestroyMesh(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("NavigationMesh failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple NavigationMesh Dispose");
            }
        }
        ~NavigationMesh()
        {
            Dispose(false);
        }
    }
}