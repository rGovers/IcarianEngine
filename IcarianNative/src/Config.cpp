#include "Config.h"

#include <assert.h>
#include <string>
#include <tinyxml2.h>

Config::Config(const std::string_view& a_path)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(a_path.data()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement* configEle = doc.FirstChildElement("Config");
        assert(configEle != nullptr);

        for (tinyxml2::XMLElement* element = configEle->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
        {
            std::string_view name = element->Name();
            if (name == "ApplicationName")
            {
                m_appName = element->GetText();
            }
            else if (name == "RenderingEngine")
            {
                std::string_view engineName = element->GetText();
                if (engineName == "Vulkan")
                {
                    m_renderingEngine = RenderingEngine_Vulkan;
                }
                else
                {
                    m_renderingEngine = RenderingEngine_Null;
                }
            }
            else if (name == "FileCacheSize")
            {
                m_fileCacheSize = (uint32_t)element->IntText();
            }
            else if (name == "FixedTimeStep")
            {
                m_fixedTimeStep = element->DoubleText();
            }
        }
    }
}
Config::~Config()
{

}