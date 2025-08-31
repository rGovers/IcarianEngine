// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <string>
#include <string_view>

#include "Core/Bitfield.h"
#include "Rendering/RenderEngine.h"

class Config
{
private:
    static constexpr uint32_t HeadlessBit = 0;
    static constexpr uint32_t RemoteBit = 1;
    static constexpr uint32_t DisableWaylandBit = 2;
    static constexpr uint32_t UnlockUPSBit = 3;
    static constexpr uint32_t UnlockFPSBit = 4;
    static constexpr uint32_t ForceMeshBit = 5;

    static constexpr char DefaultAppName[] = "IcarianEngine";

    double            m_fixedTimeStep = 1.0 / 50.0;
    uint32_t          m_fileCacheSize = 256;

    uint32_t          m_pipefileID = uint32_t(-1);
    uint32_t          m_ipcId = uint32_t(-1);

    std::string       m_appName = std::string(DefaultAppName);

    uint16_t          m_remotePort = 9001;
    e_RenderingEngine m_renderingEngine = RenderingEngine_Vulkan;

    uint8_t           m_flags;

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

    inline bool IsUPSUnlocked() const
    {
        return IISBITSET(m_flags, UnlockUPSBit);
    }
    inline void SetUPSUnlocked(bool a_value)
    {
        ITOGGLEBIT(a_value, m_flags, UnlockUPSBit);
    }

    inline bool IsFPSUnlocked() const
    {
        return IISBITSET(m_flags, UnlockFPSBit);
    }
    inline void SetFPSUnlocked(bool a_value)
    {
        ITOGGLEBIT(a_value, m_flags, UnlockFPSBit);
    }

    inline uint32_t GetPipefileID() const
    {
        return m_pipefileID;
    }
    inline void SetPipefileID(uint32_t a_id)
    {
        m_pipefileID = a_id;
    }

    inline uint32_t GetIPCID() const
    {
        return m_ipcId;
    }
    inline void SetIPCID(uint32_t a_value)
    {
        m_ipcId = a_value;
    }

    inline bool ForceMesh() const
    {
        return IISBITSET(m_flags, ForceMeshBit);
    }

    // Mostly exists because some tools still do not have the best Wayland support
    inline bool DisableWayland() const
    {
        return IISBITSET(m_flags, DisableWaylandBit);
    }
    inline void SetDisableWayland(bool a_value)
    {
        ITOGGLEBIT(a_value, m_flags, DisableWaylandBit);
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
        return IISBITSET(m_flags, HeadlessBit);
    }
    inline void SetHeadless(bool a_value)
    {
        ITOGGLEBIT(a_value, m_flags, HeadlessBit);
    }

    inline bool IsRemote() const
    {
        return IISBITSET(m_flags, RemoteBit);
    }
    inline void SetRemote(bool a_value)
    {
        ITOGGLEBIT(a_value, m_flags, RemoteBit);
    }

    inline uint16_t GetRemotePort() const
    {
        return m_remotePort;
    }
    inline void SetRemotePort(uint16_t a_port)
    {
        m_remotePort = a_port;
    }
};

// MIT License
// 
// Copyright (c) 2025 River Govers
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
