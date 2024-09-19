// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Mod;
using IcarianEngine.Rendering.UI;
using System.Collections.Concurrent;
using System.IO;
using System.Xml;

namespace IcarianEngine
{
    public static class Scribe
    {
        static ConcurrentDictionary<string, Font> s_fonts;
        static ConcurrentDictionary<string, string> s_strings;

        static string s_curLanguage;

        /// <summary>
        /// The currently selected language for localization
        /// </summary>
        public static string CurrentLanguage
        {
            get
            {
                return s_curLanguage;
            }
        }

        /// <summary>
        /// If a string exists for the current locale
        /// </summary>
        /// <param name="a_key">The string to find</param>
        /// <returns>If the string exists</returns>
        public static bool StringKeyExists(string a_key)
        {
            return s_strings.ContainsKey(a_key);
        }
        /// <summary>
        /// If a <see cref="IcarianEngine.Rendering.UI.Font" /> exists for the string in the current locale
        /// </summary>
        /// <param name="a_key">The <see cref="IcarianEngine.Rendering.UI.Font" /> to find</param>
        /// <returns>If the <see cref="IcarianEngine.Rendering.UI.Font" /> exists</returns>
        public static bool FontKeyExists(string a_key)
        {
            return s_fonts.ContainsKey(a_key);
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
                        language = att.Value.ToLower();

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

                if (language == "default")
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
                            if (!StringKeyExists(name))
                            {
                                string text = element.InnerText;

                                SetString(name, text);

                                if (font != null)
                                {
                                    SetFont(name, font);
                                }
                            }
                        }
                        else
                        {
                            Logger.IcarianWarning($"Invalid key name in Scribe file at {a_path}");
                        }
                    }
                }
                else if (language == s_curLanguage.ToLower())
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
                                SetFont(name, font);
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

        /// <summary>
        /// Loads the locale for the language
        /// </summary>
        /// <param name="a_langauge">The language to set the locale to</param>
        public static void SetLanguage(string a_language)
        {
            s_curLanguage = a_language;

            s_fonts = new ConcurrentDictionary<string, Font>();
            s_strings = new ConcurrentDictionary<string, string>();

            LoadDirectory(Path.Combine(ModControl.CoreAssembly.AssemblyInfo.Path, "Scribe"));

            foreach (IcarianAssembly a in ModControl.Assemblies)
            {
                LoadDirectory(Path.Combine(a.AssemblyInfo.Path, "Scribe"));
            }
        }

        /// <summary>
        /// Sets a string
        /// </summary>
        /// <param name="a_key">The string to use as a key</param>
        /// <param name="a_value">The string to use as a value</param>
        public static void SetString(string a_key, string a_value)
        {
            if (StringKeyExists(a_key))
            {
                string s = s_strings[a_key];

                s_strings.TryUpdate(a_key, a_value, s);
            }
            else
            {
                s_strings.TryAdd(a_key, a_value);
            }
        }
        /// <summary>
        /// Gets a string from the locale
        /// </summary>
        /// <param name="a_key">The string to get from the locale</param>
        /// <returns>The locale string. The key on failure</returns>
        public static string GetString(string a_key)
        {
            if (StringKeyExists(a_key))
            {
                return s_strings[a_key];
            }

            return a_key;
        }
        /// <summary>
        /// Gets a formated string from the locale
        /// </summary>
        /// <param name="a_key">The string to get from the locale</param>
        /// <param name="a_args">The arguments to replace the placeholder values with</param>
        /// <returns>The formated string. The key on failure</returns>
        public static string GetStringFormated(string a_key, string[] a_args)
        {
            // Not sure if I should parse the arguments through Scribe or that should be the callers resposibility
            // I am gonna say no as the caller may not always want to do that but I may revist later

            return string.Format(GetString(a_key), a_args);
        }

        /// <summary>
        /// Sets the <see cref="IcarianEngine.Rendering.UI.Font" /> for a string key
        /// </summary>
        /// <param name="a_key">The key</param>
        /// <param name="a_font">The <see cref="IcarianEngine.Rendering.UI.Font" /> for the key</param>
        public static void SetFont(string a_key, Font a_font)
        {
            if (FontKeyExists(a_key))
            {
                Font f = s_fonts[a_key];

                s_fonts.TryUpdate(a_key, a_font, f);
            }
            else
            {
                s_fonts.TryAdd(a_key, a_font);
            }
        }
        /// <summary>
        /// Gets a <see cref="IcarianEngine.Rendering.UI.Font" /> from the locale
        /// </summary>
        /// <param name="a_key">The <see cref="IcarianEngine.Rendering.UI.Font" /> key to get from the locale</param>
        /// <returns>The locale font. Null on failure</returns>
        public static Font GetFont(string a_key)
        {
            if (FontKeyExists(a_key))
            {
                return s_fonts[a_key];
            }

            return null;
        }
    }
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