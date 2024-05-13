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