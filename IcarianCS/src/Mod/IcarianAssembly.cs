using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Xml;

#include "EngineIcarianAssemblyInterop.h"
#include "InteropBinding.h"

ENGINE_ICARIANASSEMBLY_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Mod
{
    public class IcarianAssembly
    {
        AssemblyControl     m_assemblyControl;

        IcarianAssemblyInfo m_assemblyInfo;

        List<Assembly>      m_assemblies;

        internal AssemblyControl AssemblyControl
        {
            get
            {
                return m_assemblyControl;
            }
        }

        /// <summary>
        /// Information about the loaded assembly
        /// </summary>
        public IcarianAssemblyInfo AssemblyInfo
        {
            get
            {
                return m_assemblyInfo;
            }
        }

        /// <summary>
        /// C# assemblies loaded by the IcarianAssembly
        /// </summary>
        public IEnumerable<Assembly> Assemblies
        {
            get
            {
                return m_assemblies;
            }
        }

        IcarianAssembly(IcarianAssemblyInfo a_info)
        {
            m_assemblyInfo = a_info;

            m_assemblyControl = null;
            m_assemblies = new List<Assembly>();
        }

        /// <summary>
        /// Gets a type from the IcarianAssembly
        /// </summary>
        /// <param name="a_name">The name of the type to get</param>
        /// <returns>The type. Null on failure</returns>
        public Type GetTypeValue(string a_name)
        {
            foreach (Assembly asm in m_assemblies)
            {
                Type t = asm.GetType(a_name, false);
                if (t != null)
                {
                    return t;
                }
            }

            return null;
        }

        void LoadData(string a_path)
        {
            string assemblyPath = Path.Combine(a_path, "Assemblies");
            if (Directory.Exists(assemblyPath))
            {
                string[] paths;

                string nativeAssemblies = Path.Combine(assemblyPath, "Native");
                if (Directory.Exists(nativeAssemblies))
                {
                    paths = Directory.GetFiles(nativeAssemblies);
                    foreach (string str in paths)
                    {
                        string ext = Path.GetExtension(str);

                        if (ext != ".dll" && ext != ".so")
                        {
                            continue;
                        }

                        IcarianAssemblyInterop.LoadNativeAssembly(str);
                    }
                }

                paths = Directory.GetFiles(assemblyPath);
                foreach (string str in paths)
                {
                    if (Path.GetExtension(str) != ".dll")
                    {
                        continue;
                    }

                    // Already loaded because we are it so can skip
                    // Some compilers like to add to the output for some reason
                    if (Path.GetFileNameWithoutExtension(str) == "IcarianCS")
                    {
                        continue;
                    }

                    m_assemblies.Add(Assembly.LoadFile(str));
                }

                foreach (Assembly assembly in m_assemblies)
                {
                    Type[] types = assembly.GetTypes();

                    foreach (Type type in types)
                    {
                        if (type.IsSubclassOf(typeof(AssemblyControl)))
                        {
                            m_assemblyControl = Activator.CreateInstance(type) as AssemblyControl;

                            // Prefer not initialize multiple so early exit
                            // If the user adds mutliple we have bigger issues as it is an "Entry Point" air quotes being important
                            return;
                        }
                    }
                }
            }
        }

        internal static IcarianAssembly LoadIcarianAssembly(IcarianAssemblyInfo a_info)
        {
            string path = a_info.Path;

            if (Directory.Exists(path))
            {
                IcarianAssembly asm = new IcarianAssembly(a_info);
                asm.LoadData(path);

                return asm;
            }

            return null;
        }
        internal static IcarianAssembly LoadIcarianAssembly(string a_path)
        {
            if (Directory.Exists(a_path))
            {
                IcarianAssemblyInfo assemblyInfo = IcarianAssemblyInfo.Load(a_path);

                if (assemblyInfo == null)
                {
                    Logger.IcarianError($"Failed to load assembly about: {a_path}");

                    return null;
                }

                IcarianAssembly asm = new IcarianAssembly(assemblyInfo);
                asm.LoadData(a_path);

                return asm;
            }

            return null;
        }
    }
}