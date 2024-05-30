#include "Config.h"

#include <assert.h>
#include <string>
#include <tinyxml2.h>

#include "Core/StringUtils.h"

Config::Config(const std::string_view& a_path)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(a_path.data()) == tinyxml2::XML_SUCCESS)
    {
        tinyxml2::XMLElement* configEle = doc.FirstChildElement("Config");
        assert(configEle != nullptr);

        for (tinyxml2::XMLElement* element = configEle->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
        {
            switch (StringHash(element->Name()))
            {
            case StringHash("ApplicationName"):
            {
                m_appName = element->GetText();

                break;
            }
            case StringHash("RenderingEngine"):
            {
                switch (StringHash(element->GetText()))
                {
                case StringHash("Vulkan"):
                {
                    m_renderingEngine = RenderingEngine_Vulkan;

                    break;
                }
                default:
                {
                    m_renderingEngine = RenderingEngine_Null;

                    break;
                }
                }

                break;
            }
            case StringHash("FileCacheSize"):
            {
                m_fileCacheSize = (uint32_t)element->IntText();

                break;
            }
            case StringHash("FixedTimeStep"):
            {
                m_fixedTimeStep = element->DoubleText();

                break;
            }
            }
        }
    }
}
Config::~Config()
{

}