// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System.IO;
using System.Xml;

namespace IcarianEngine.Mod
{
    public class IcarianAssemblyInfo
    {
        string m_id;
        string m_name;
        string m_path;
        string m_description;

        /// <summary>
        /// The ID of the loaded mod
        /// </summary>
        public string ID
        {
            get
            {
                return m_id;
            }
        }
        /// <summary>
        /// The name of the loaded mod
        /// </summary>
        public string Name
        {
            get
            {
                return m_name;
            }
        }
        /// <summary>
        /// The path of the loaded mod
        /// </summary>
        public string Path
        {
            get
            {
                return m_path;
            }
        }
        /// <summary>
        /// The description of the loaded mod
        /// </summary>
        public string Description
        {
            get
            {
                return m_description;
            }
            set
            {
                m_description = value;
            }
        }

        internal IcarianAssemblyInfo(string a_id, string a_name, string a_path, string a_description)
        {
            m_id = a_id;
            m_name = a_name;
            m_path = a_path;
            m_description = a_description;
        }

        /// <summary>
        /// Loads the assembly info of an IcarianMod from a folder
        /// </summary>
        /// <param name="a_path">The folder to load from</param>
        /// <returns>The loaded IcarianAssemblyInfo. Null on failure</returns>
        public static IcarianAssemblyInfo Load(string a_path)
        {
            string aboutPath = System.IO.Path.Combine(a_path, "about.xml");
            if (!File.Exists(aboutPath))
            {
                Logger.IcarianError($"About does not exist: {a_path}");

                return null;
            }

            XmlDocument doc = new XmlDocument();
            doc.Load(aboutPath);

            if (doc.DocumentElement is XmlElement root)
            {
                string pathName = System.IO.Path.GetFileName(a_path);

                string id = string.Empty;
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
                            Logger.IcarianError($"Invalid about element: {element.Name}, {a_path}");

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

                return new IcarianAssemblyInfo(id, name, a_path, desciption);
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