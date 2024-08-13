// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "ModInfo.h"
#include "tinyxml2.h"

ModInfo::ModInfo()
{

}
ModInfo::~ModInfo()
{

}

ModInfo* ModInfo::LoadMod(const std::filesystem::path& a_path)
{
    const std::filesystem::path aboutPath = a_path / "about.xml";
    if (std::filesystem::exists(aboutPath))
    {
        const std::string aboutPathStr = aboutPath.string();

        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(aboutPathStr.c_str()) == tinyxml2::XML_SUCCESS)
        {
            tinyxml2::XMLElement* root = doc.FirstChildElement("About");
            if (root != nullptr)
            {
                std::string id;
                std::string name;
                std::string description;
                
                for (tinyxml2::XMLElement* element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
                {
                    const std::string_view elementName = element->Name();

                    if (elementName == "ID")
                    {
                        id = element->GetText();
                    }
                    else if (elementName == "Name")
                    {
                        name = element->GetText();
                    }
                    else if (elementName == "Description")
                    {
                        description = element->GetText();
                    }
                }

                if (id.empty())
                {
                    return nullptr;
                }

                ModInfo* info = new ModInfo();
                info->m_name = name;
                info->m_id = id;
                info->m_description = description;

                return info;
            }
        }
    }

    return nullptr;
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