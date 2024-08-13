// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <string>
#include <string_view>

#include "Rendering/RenderEngine.h"

class Config
{
private:
    static constexpr char DefaultAppName[] = "IcarianEngine";

    bool              m_headless = false;

    double            m_fixedTimeStep = 1.0 / 50.0;
    uint32_t          m_fileCacheSize = 256;

    std::string       m_appName = std::string(DefaultAppName);

    e_RenderingEngine m_renderingEngine = RenderingEngine_Vulkan;

protected:

public:
    Config(const std::string_view& a_path);
    ~Config();

    inline double GetFixedTimeStep() const
    {
        return m_fixedTimeStep;
    }

    inline uint32_t GetFileCacheSize() const
    {
        return m_fileCacheSize;
    }

    inline const std::string GetApplicationName() const
    {
        return m_appName;
    }
    inline e_RenderingEngine GetRenderingEngine() const
    {
        return m_renderingEngine;
    }
    inline bool IsHeadless() const
    {
        return m_headless;
    }
    inline void SetHeadless(bool a_value)
    {
        m_headless = a_value;
    }
};

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