// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <enet/enet.h>

#include "EngineNetworkInteropStructures.h"

class NetworkManager;

class NetworkClient
{
private:
    NetworkManager* m_manager;

    uint32_t        m_server;
    uint32_t        m_addr;

    ENetHost*       m_host;
    ENetPeer*       m_peer;

    NetworkClient();

protected:

public:
    NetworkClient(NetworkManager* a_manager, uint32_t a_server, const ENetEvent& a_event);
    ~NetworkClient();

    static NetworkClient* Connect(NetworkManager* a_manager, const NetworkAddress& address);

    inline void SetBufferAddress(uint32_t a_addr)
    {
        m_addr = a_addr;
    }
    inline uint32_t GetServerAddress() const
    {
        return m_server;
    }

    void Send(const uint8_t* a_data, uint32_t a_size, e_PacketFlags a_flags);

    void Update();
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