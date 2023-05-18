using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Xml;

namespace IcarianEngine.Mod
{
    public class IcarianAssembly
    {
        AssemblyControl     m_assemblyControl = null;

        IcarianAssemblyInfo m_assemblyInfo;

        List<Assembly>      m_assemblies = null;

        internal AssemblyControl AssemblyControl
        {
            get
            {
                return m_assemblyControl;
            }
        }

        public IcarianAssemblyInfo AssemblyInfo
        {
            get
            {
                return m_assemblyInfo;
            }
        }

        public IEnumerable<Assembly> Assemblies
        {
            get
            {
                return m_assemblies;
            }
        }

        IcarianAssembly()
        {

        }

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

        internal static IcarianAssembly GetIcarianAssembly(string a_path)
        {
            if (Directory.Exists(a_path))
            {
                IcarianAssembly asm = new IcarianAssembly();

                string assemblyPath = Path.Combine(a_path, "Assemblies");
                string aboutPath = Path.Combine(a_path, "about.xml");

                if (File.Exists(aboutPath))
                {
                    XmlDocument doc = new XmlDocument();
                    doc.Load(aboutPath);

                    if (doc.DocumentElement is XmlElement root)
                    {
                        string pathName = Path.GetFileName(a_path);

                        string id = null;
                        string name = pathName;
                        string desciption = string.Empty;

                        foreach (XmlNode node in root.ChildNodes)
                        {
                            if (node is XmlElement element)
                            {
                                switch (element.Name)
                                {
                                case "ID":
                                {
                                    id = element.InnerText;

                                    break;
                                }
                                case "Name":
                                {
                                    name = element.InnerText;

                                    break;
                                }
                                case "Description":
                                {
                                    desciption = element.InnerText;

                                    break;
                                }
                                default:
                                {
                                    Logger.IcarianError($"Invalid about element: {element.Name}");

                                    break;
                                }
                                }
                            }
                        }

                        if (string.IsNullOrWhiteSpace(id))
                        {
                            Logger.IcarianError($"Invalid mod id in about: {aboutPath}");

                            return null;
                        }

                        asm.m_assemblyInfo = new IcarianAssemblyInfo(id, name, a_path, desciption);
                    }
                }
                else
                {
                    Logger.IcarianError($"No mod about: {a_path}");

                    return null;
                }

                if (Directory.Exists(assemblyPath))
                {
                    asm.m_assemblies = new List<Assembly>();

                    string[] paths = Directory.GetFiles(assemblyPath);
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

                        asm.m_assemblies.Add(Assembly.LoadFile(str));
                    }

                    foreach (Assembly assembly in asm.m_assemblies)
                    {
                        Type[] types = assembly.GetTypes();

                        foreach (Type type in types)
                        {
                            if (type.IsSubclassOf(typeof(AssemblyControl)))
                            {
                                asm.m_assemblyControl = Activator.CreateInstance(type) as AssemblyControl;

                                return asm;
                            }
                        }
                    }
                }

                return asm;
            }

            return null;
        }
    }
}