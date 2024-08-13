// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Core/IcarianDefer.h"
#include "Rendering/Vulkan/VulkanLightData.h"

VulkanLightData::VulkanLightData()
{
    m_splitCount = 0;
    m_splits = nullptr;
}
VulkanLightData::~VulkanLightData()
{
    if (m_splits != nullptr)
    {
        delete[] m_splits;
    }
}

void VulkanLightData::SetLightSplits(const LightShadowSplit* a_splits, uint32_t a_splitCount)
{
    IDEFER(m_splitCount = a_splitCount);
    if (a_splitCount <= 0)
    {
        return;
    }

    if (m_splits != nullptr)
    {
        if (m_splitCount < a_splitCount)
        {
            delete[] m_splits;
            m_splits = new LightShadowSplit[a_splitCount];
        }
    }
    else
    {
        m_splits = new LightShadowSplit[a_splitCount];
    }

    for (uint32_t i = 0; i < a_splitCount; ++i)
    {
        m_splits[i] = a_splits[i];
    }
}

#endif

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