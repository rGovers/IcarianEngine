using System.IO;
using System.Runtime.CompilerServices;
using System.Xml;
using IcarianEngine.Mod;
using IcarianEngine.Rendering.UI;

namespace IcarianEngine
{
    public static class Scribe
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetInternalLanguage(string a_string);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static string GetInternalLanguage();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetString(string a_key);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetStringFormated(string a_key, string[] a_args);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern uint GetFontAddr(string a_key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetString(string a_key, string a_value);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void SetFont(string a_key, uint a_value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint Exists(string a_key);

        public static string CurrentLanguage
        {
            get
            {
                return GetInternalLanguage();
            }
        }

        public static bool KeyExists(string a_key)
        {
            return Exists(a_key) != 0;
        }

        static void LoadFile(string a_path)
        {
            XmlDocument doc = new XmlDocument();
            doc.Load(a_path);

            if (doc.DocumentElement is XmlElement root)
            {
                string language = "default";
                string fontpath = null;
                foreach (XmlAttribute att in root.Attributes)
                {
                    switch (att.Name.ToLower())
                    {
                    case "language":
                    {
                        language = att.Value;

                        break;
                    }
                    case "font":
                    {
                        fontpath = att.Value;

                        break;
                    }
                    default:
                    {
                        Logger.IcarianWarning($"Invalid attribute in scribe file: {att.Name} at {a_path}");

                        break;
                    }
                    }
                }
                
                Font font = null;
                if (!string.IsNullOrWhiteSpace(fontpath))
                {
                    font = AssetLibrary.LoadFont(fontpath);
                }

                if (language.ToLower() == "default")
                {
                    foreach (XmlNode node in root.ChildNodes)
                    {
                        XmlElement element = node as XmlElement;
                        if (element == null)
                        {
                            continue;
                        }

                        string name = element.Name;
                        if (!string.IsNullOrWhiteSpace(name))
                        {
                            if (Exists(name) == 0)
                            {
                                string text = element.InnerText;
                                if (text == null)
                                {
                                    text = string.Empty;
                                }

                                SetString(name, element.InnerText);

                                if (font != null)
                                {
                                    SetFont(name, font.BufferAddr);
                                }
                            }
                        }
                        else
                        {
                            Logger.IcarianWarning($"Invalid key name in Scribe file at {a_path}");
                        }
                    }
                }
                else if (language.ToLower() == GetInternalLanguage().ToLower())
                {
                    foreach (XmlNode node in root.ChildNodes)
                    {
                        XmlElement element = node as XmlElement;
                        if (element == null)
                        {
                            continue;
                        }

                        string name = element.Name;
                        if (!string.IsNullOrWhiteSpace(name))
                        {
                            string text = element.InnerText;
                            if (text == null)
                            {
                                text = string.Empty;
                            }

                            SetString(name, text);

                            if (font != null)
                            {
                                SetFont(name, font.BufferAddr);
                            }
                        }
                        else
                        {
                            Logger.IcarianWarning($"Invalid key name in Scribe file at {a_path}");
                        }
                    }
                }
            }
        }

        internal static void LoadDirectory(string a_path)
        {
            if (!Directory.Exists(a_path))
            {
                return;
            }

            string[] files = Directory.GetFiles(a_path);
            foreach (string f in files)
            {
                if (Path.GetExtension(f) == ".scrb")
                {
                    LoadFile(f);
                }
            }

            string[] paths = Directory.GetDirectories(a_path);
            foreach (string p in paths)
            {
                LoadDirectory(p);
            }
        }

        public static void SetLanguage(string a_language)
        {
            SetInternalLanguage(a_language);

            LoadDirectory(Path.Combine(ModControl.CoreAssembly.AssemblyInfo.Path, "Scribe"));

            foreach (IcarianAssembly a in ModControl.Assemblies)
            {
                LoadDirectory(Path.Combine(a.AssemblyInfo.Path, "Scribe"));
            }
        }
        
        public static void SetFont(string a_key, Font a_font)
        {
            SetFont(a_key, a_font.BufferAddr);
        }
        public static Font GetFont(string a_key)
        {
            return Font.GetFont(GetFontAddr(a_key));
        }
    }
}