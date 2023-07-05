using IcarianEngine.Definitions;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;

namespace IcarianEngine.Mod
{
    public static class ModControl
    {
        public static IcarianAssembly CoreAssembly
        {
            get;
            private set;
        }

        public static List<IcarianAssembly> Assemblies
        {
            get;
            private set;
        }

        internal static void Init()
        {
            Assemblies = new List<IcarianAssembly>();

            bool working = !string.IsNullOrWhiteSpace(Application.WorkingDirectory);

            string corePath = Path.Combine(Application.WorkingDirectory, "Core");

            if (working)
            {
                CoreAssembly = IcarianAssembly.GetIcarianAssembly(corePath);
            }

            if (CoreAssembly == null)
            {
                corePath = Path.Combine(Directory.GetCurrentDirectory(), "Core");
                CoreAssembly = IcarianAssembly.GetIcarianAssembly(corePath);       
            }

            if (CoreAssembly == null)
            {
                Logger.IcarianError("Failed to load core assembly");
            }

            string coreDefPath = Path.Combine(corePath, "Defs");
            if (Directory.Exists(coreDefPath))
            {
                DefLibrary.LoadDefs(coreDefPath);
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

        public static string GetAssetPath(string a_path)
        {
            if (File.Exists(a_path))
            {
                return a_path;
            }

            for (int i = Assemblies.Count - 1; i >= 0; --i)
            {
                string mPath = Path.Combine(Assemblies[i].AssemblyInfo.Path, "Assets", a_path);
                if (File.Exists(mPath))
                {
                    return mPath;
                }
            }

            string cPath = Path.Combine(CoreAssembly.AssemblyInfo.Path, "Assets", a_path);
            if (File.Exists(cPath))
            {
                return cPath;
            }

            return null;
        } 
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
                    return Path.Combine(asm.AssemblyInfo.ID, "Scenes", a_path);
                }
            }

            return null;
        }
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