using IcarianEngine.Definitions;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Xml;

namespace IcarianEngine.Mod
{
    public static class ModControl
    {
        /// <summary>
        /// The core assembly loaded by Icarian Engine.
        /// </summary>
        public static IcarianAssembly CoreAssembly
        {
            get;
            private set;
        }

        /// <summary>
        /// The list of mod assemblies loaded by Icarian Engine.
        /// </summary>
        public static List<IcarianAssembly> Assemblies
        {
            get;
            private set;
        }

        static void LoadMods(string a_path)
        {
            string modPath = Path.Combine(a_path, "Mod");
            if (Directory.Exists(modPath))
            {
                string modListPath = Path.Combine(modPath, "ModList.xml");
                if (!File.Exists(modListPath))
                {
                    return;
                }

                List<string> ids = new List<string>();
                XmlDocument doc = new XmlDocument();
                doc.Load(modListPath);

                if (doc.DocumentElement is XmlElement root)
                {
                    foreach (XmlNode node in root.ChildNodes)
                    {
                        if (node is XmlElement element)
                        {
                            if (element.Name == "Mod")
                            {
                                ids.Add(element.InnerText);
                            }
                            else
                            {
                                Logger.IcarianError($"Invalid ModList element: {element.Name}, {a_path}");
                            }
                        }
                    }
                }

                if (ids.Count <= 0)
                {
                    return;
                }

                string[] dirs = Directory.GetDirectories(modPath);
                foreach (string d in dirs)
                {
                    IcarianAssemblyInfo info = IcarianAssemblyInfo.Load(d);
                    string id = info.ID;

                    if (info != null && ids.Contains(id))
                    {
                        IcarianAssembly asm = IcarianAssembly.LoadIcarianAssembly(info);

                        ids.Remove(id);

                        Assemblies.Add(asm);
                    }
                }
            }
        }

        internal static void Init()
        {
            Assemblies = new List<IcarianAssembly>();

            bool working = !string.IsNullOrWhiteSpace(Application.WorkingDirectory);

            string corePath = Path.Combine(Application.WorkingDirectory, "Core");

            if (working)
            {
                CoreAssembly = IcarianAssembly.LoadIcarianAssembly(corePath);

                LoadMods(Application.WorkingDirectory);
            }

            string curDir = Directory.GetCurrentDirectory();

            if (CoreAssembly == null)
            {
                corePath = Path.Combine(curDir, "Core");
                CoreAssembly = IcarianAssembly.LoadIcarianAssembly(corePath);       
            }

            LoadMods(curDir);

            if (CoreAssembly == null)
            {
                Logger.IcarianError("Failed to load core assembly");
            }

            string coreDefPath = Path.Combine(corePath, "Defs");
            if (Directory.Exists(coreDefPath))
            {
                DefLibrary.LoadDefs(coreDefPath);
            }

            foreach (IcarianAssembly iAsm in Assemblies)
            {
                string defPath = Path.Combine(iAsm.AssemblyInfo.Path, "Defs");
                if (Directory.Exists(defPath))
                {
                    DefLibrary.LoadDefs(defPath);
                }
            }
        }

        internal static void InitAssemblies()
        {
            CoreAssembly.AssemblyControl.Init();

            foreach (IcarianAssembly asm in Assemblies)
            {
                if (asm.AssemblyControl != null)
                {
                    asm.AssemblyControl.Init();
                }
            }
        }

        /// <summary>
        /// Gets a function from all loaded assemblies.
        /// </summary>
        /// <typeparam name="T">The delegate type.</typeparam>
        /// <param name="a_name">The name of the function.</param>
        /// <returns>The function. Null if failed</returns>
        public static T GetFunction<T>(string a_name) where T : Delegate
        {
            string[] strings = a_name.Split(':');

            if (strings.Length != 2)
            {
                Logger.IcarianWarning($"Invalid function name {a_name}");

                return null;
            }

            foreach (Assembly asm in CoreAssembly.Assemblies)
            {
                Type type = asm.GetType(strings[0]);
                if (type != null)
                {
                    MethodInfo method = type.GetMethod(strings[1], BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);
                    if (method != null)
                    {
                        return Delegate.CreateDelegate(typeof(T), method) as T;
                    }
                }
            }

            foreach (IcarianAssembly iAsm in Assemblies)
            {
                foreach (Assembly asm in iAsm.Assemblies)
                {
                    Type type = asm.GetType(strings[0]);
                    if (type != null)
                    {
                        MethodInfo method = type.GetMethod(strings[1], BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);
                        if (method != null)
                        {
                            return Delegate.CreateDelegate(typeof(T), method) as T;
                        }
                    }
                }
            }

            Logger.IcarianWarning($"Failed to find function {a_name}");

            return null;
        }
        /// <summary>
        /// Gets a function from all loaded assemblies.
        /// </summary>
        /// <typeparam name="T">The delegate type.</typeparam>
        /// <param name="a_namespace">The namespace that contains the function.</param>
        /// <param name="a_class">The class that contains the function.</param>
        /// <param name="a_function">The function name.</param>
        /// <returns>The function. Null if failed</returns>
        public static T GetFunction<T>(string a_namespace, string a_class, string a_function) where T : Delegate
        {
            foreach (Assembly asm in CoreAssembly.Assemblies)
            {
                Type type = asm.GetType($"{a_namespace}.{a_class}");
                if (type != null)
                {
                    MethodInfo method = type.GetMethod(a_function);
                    if (method != null)
                    {
                        return Delegate.CreateDelegate(typeof(T), method) as T;
                    }
                }
            }

            foreach (IcarianAssembly iAsm in Assemblies)
            {
                foreach (Assembly asm in iAsm.Assemblies)
                {
                    Type type = asm.GetType($"{a_namespace}.{a_class}");
                    if (type != null)
                    {
                        MethodInfo method = type.GetMethod(a_function);
                        if (method != null)
                        {
                            return Delegate.CreateDelegate(typeof(T), method) as T;
                        }
                    }
                }
            }

            Logger.IcarianWarning($"Failed to find function {a_namespace}.{a_class}.{a_function}");

            return null;
        }

        /// <summary>
        /// Gets a Type in the core assembly.
        /// </summary>
        /// <param name="a_name">The name of the Type.</param>
        /// <param name="a_def">If the Type is a definition.</param>
        /// <returns>The Type. Null if failed</returns>
        public static Type GetCoreTypeValue(string a_name, bool a_def = false)
        {
            string coreName = $"IcarianEngine.{a_name}";
            if (a_def)
            {
                coreName = $"IcarianEngine.Definitions.{a_name}";
            }

            Type type = Type.GetType(coreName);
            if (type != null)
            {
                return type;
            }

            type = Type.GetType(a_name);
            if (type != null)
            {
                return type;
            }

            if (!Application.IsEditor)
            {
                return CoreAssembly.GetTypeValue(a_name);
            }
            
            return null;
        }
        /// <summary>
        /// Gets a Type in all loaded assemblies.
        /// </summary>
        /// <param name="a_name">The name of the Type.</param>
        /// <param name="a_def">If the Type is a definition.</param>
        /// <returns>The Type. Null if failed</returns>
        public static Type GetTypeValue(string a_name, bool a_def = false)
        {
            Type t = GetCoreTypeValue(a_name, a_def);
            if (t != null)
            {
                return t;
            }

            if (!Application.IsEditor)
            {
                foreach (IcarianAssembly asm in Assemblies)
                {
                    Type type = asm.GetTypeValue(a_name);
                    if (type != null)
                    {
                        return type;
                    }
                }
            }
            else
            {
                Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
                foreach (Assembly asm in assemblies)
                {
                    Type type = asm.GetType(a_name, false);
                    if (type != null)
                    {
                        return type;
                    }
                }
            }

            return null;
        }

        internal static void Update()
        {
            Profiler.StartFrame("Assembly Update");

            CoreAssembly.AssemblyControl.Update();

            foreach (IcarianAssembly asm in Assemblies)
            {
                if (asm.AssemblyControl != null)
                {
                    asm.AssemblyControl.Update();
                }
            }

            Profiler.StopFrame();
        }
        internal static void FixedUpdate()
        {
            CoreAssembly.AssemblyControl.FixedUpdate();

            foreach (IcarianAssembly asm in Assemblies)
            {
                if (asm.AssemblyControl != null)
                {
                    asm.AssemblyControl.FixedUpdate();
                }
            }
        }
        internal static void FrameUpdate()
        {
            CoreAssembly.AssemblyControl.FrameUpdate();

            foreach (IcarianAssembly asm in Assemblies)
            {
                if (asm.AssemblyControl != null)
                {
                    asm.AssemblyControl.FrameUpdate();
                }
            }
        }

        internal static void Close()
        {
            CoreAssembly.AssemblyControl.Close();

            foreach (IcarianAssembly asm in Assemblies)
            {
                if (asm.AssemblyControl != null)
                {
                    asm.AssemblyControl.Close();
                }
            }

            Assemblies.Clear();
        }

        /// <summary>
        /// Gets the full path of a mod asset. Searches LIFO.
        /// </summary>
        /// <param name="a_path">The path of the asset.</param>
        /// <returns>The full path of the asset. Null if failed</returns>
        public static string GetAssetPath(string a_path)
        {
            if (File.Exists(a_path))
            {
                return a_path;
            }

            for (int i = Assemblies.Count - 1; i >= 0; --i)
            {
                string mPath = Assemblies[i].GetAssetPath(a_path);
                if (!string.IsNullOrEmpty(mPath))
                {
                    return mPath;
                }
            }

            string cPath = CoreAssembly.GetAssetPath(a_path);
            if (!string.IsNullOrEmpty(cPath))
            {
                return cPath;
            }

            return null;
        } 
        /// <summary>
        /// Gets the full path of a mod scene in a specific mod.
        /// </summary>
        /// <param name="a_path">The path of the scene.</param>
        /// <param name="a_modID">The ID of the mod.</param>
        /// <returns>The full path of the scene. Null if failed</returns>
        public static string GetScenePath(string a_path, string a_modID)
        {
            if (a_modID == CoreAssembly.AssemblyInfo.ID)
            {
                return Path.Combine(CoreAssembly.AssemblyInfo.Path, "Scenes", a_path);
            }

            foreach (IcarianAssembly asm in Assemblies)
            {
                if (asm.AssemblyInfo.ID == a_modID)
                {
                    return Path.Combine(asm.AssemblyInfo.Path, "Scenes", a_path);
                }
            }

            return null;
        }
        /// <summary>
        /// Gets the full path of a mod scene. Searches LIFO.
        /// </summary>
        /// <param name="a_path">The path of the scene.</param>
        /// <returns>The full path of the scene. Null if failed</returns>
        public static string GetScenePath(string a_path)
        {
            if (File.Exists(a_path))
            {
                return a_path;
            }

            for (int i = Assemblies.Count - 1; i >= 0; --i)
            {
                string mPath = Path.Combine(Assemblies[i].AssemblyInfo.Path, "Scenes", a_path);
                if (File.Exists(mPath))
                {
                    return mPath;
                }
            }

            string cPath = Path.Combine(CoreAssembly.AssemblyInfo.Path, "Scenes", a_path);
            if (File.Exists(cPath))
            {
                return cPath;
            }

            return null;
        }
    }
}