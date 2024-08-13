// Icarian Engine - C# Game Engine
// 
// License at end of file.

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