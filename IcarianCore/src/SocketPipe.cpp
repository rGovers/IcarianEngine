// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Core/SocketPipe.h"

#include <chrono>
#include <cstring>
#include <thread>

#include "Core/IcarianError.h"

namespace IcarianCore 
{
    SocketPipe::SocketPipe()
    {
        m_host = NULL;
        m_peer = NULL;
    }
    SocketPipe::~SocketPipe()
    {
        if (m_peer != NULL)
        {
            enet_peer_disconnect(m_peer, 0);

            ENetEvent event;
            while (enet_host_service(m_host, &event, 3000) > 0)
            {
                switch (event.type) 
                {
                case ENET_EVENT_TYPE_RECEIVE:
                {
                    enet_packet_destroy(event.packet);

                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                {
                    goto End;
                }
                default:
                {
                    break;
                }
                }
            }

            enet_peer_reset(m_peer);
End:;
        }

        enet_host_destroy(m_host);
    }

    SocketPipe* SocketPipe::Connect(const std::string_view& a_addr, uint16_t a_port)
    {
        IERRBLOCK;

        ENetHost* host = enet_host_create(NULL, 1, 2, 0, 0);
        IERRCHECKRET(host != NULL, nullptr);
        IERRDEFER(enet_host_destroy(host));

        ENetAddress addr;
        IERRCHECKRET(enet_address_set_host_ip(&addr, a_addr.data()) == 0, nullptr);
        addr.port = a_port;

        uint32_t attempts = 0;

        // Been having issues getting a connection so spin a couple times before we give up
        // We do not give up right away as sometimes once the connection is established we are fine just getting the connection is fun
        ENetPeer* peer = NULL;
        while (true)
        {
            IDEFER(++attempts);

            peer = enet_host_connect(host, &addr, 2, 0);
            if (peer != NULL)
            {
                break;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(200));

            IERRCHECKRET(attempts < 5, nullptr);
        }
        IERRDEFER(enet_peer_reset(peer));

        attempts = 0;        

        while (true)
        {
            IDEFER(++attempts);

            ENetEvent event;
            if (enet_host_service(host, &event, 1000) <= 0)
            {
                IERRCHECKRET(attempts < 5, nullptr);

                continue;
            }

            if (event.type == ENET_EVENT_TYPE_CONNECT)
            {
                break;
            }

            IERRCHECKRET(attempts < 5, nullptr);
        }

        SocketPipe* pipe = new SocketPipe();
        pipe->m_host = host;
        pipe->m_peer = peer;

        return pipe;
    }
    SocketPipe* SocketPipe::Create(uint16_t a_port)
    {
        IERRBLOCK;

        const ENetAddress addr = 
        {
            .host = ENET_HOST_ANY,
            .port = a_port
        };

        ENetHost* host = enet_host_create(&addr, 1, 2, 0, 0);
        IERRCHECKRET(host != NULL, nullptr);
        IERRDEFER(enet_host_destroy(host));

        ENetEvent event;
        while (enet_host_service(host, &event, 30000) > 0)
        {
            switch (event.type) 
            {
            case ENET_EVENT_TYPE_CONNECT:
            {
                SocketPipe* pipe = new SocketPipe();
                pipe->m_host = host;
                pipe->m_peer = event.peer;

                return pipe;
            }
            default:
            {
                break;
            }
            }
        }

        ITRIGGERERRRET(nullptr);
    }

    bool SocketPipe::IsAlive() const
    {
        return m_host != NULL && m_peer != NULL;
    }

    bool SocketPipe::Send(const PipeMessage& a_msg)
    {
        if (m_host == NULL || m_peer == NULL)
        {
            return false;
        }

        enet_uint32 flags = 0;
        switch (a_msg.Type) 
        {
        case PipeMessageType_PushFrame:
        {
            break;
        }
        default:
        {
            flags = ENET_PACKET_FLAG_RELIABLE;

            break;
        }
        }

        const uint64_t messageSize = a_msg.Length + PipeMessage::Size;

        char* dat = new char[messageSize];
        IDEFER(delete[] dat);

        memcpy(dat, &a_msg, PipeMessage::Size);
        if (a_msg.Data != nullptr)
        {
            memcpy(dat + PipeMessage::Size, a_msg.Data, a_msg.Length);
        }

        ENetPacket* packet = enet_packet_create(dat, (size_t)messageSize, flags);
        return enet_peer_send(m_peer, 0, packet) == 0;
    }
    bool SocketPipe::Receive(std::queue<PipeMessage>* a_messages)
    {
        if (m_host == NULL || m_peer == NULL)
        {
            return false;
        }

        ENetEvent event;
        while (enet_host_service(m_host, &event, 0) > 0)
        {
            switch (event.type) 
            {
            case ENET_EVENT_TYPE_RECEIVE:
            {
                ENetPacket* packet = event.packet;
                IDEFER(enet_packet_destroy(packet));

                PipeMessage msg = { };

                const uint64_t packetSize = (uint64_t)packet->dataLength;
                if (packetSize > PipeMessage::Size)
                {
                    const uint64_t dataSize = packetSize - PipeMessage::Size;

                    msg.Data = new char[dataSize];
                    memcpy(msg.Data, packet->data + PipeMessage::Size, dataSize);
                }
                else if (packetSize < PipeMessage::Size)
                {
                    // Someone is doing a fucky
                    return false;
                }

                memcpy(&msg, packet->data, PipeMessage::Size);

                a_messages->emplace(msg);

                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            {
                enet_peer_disconnect(m_peer, 0);

                // Can probably do this better but eh works for now
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));

                enet_peer_reset(m_peer);
                m_peer = NULL;

                enet_host_destroy(m_host);
                m_host = NULL;

                break;
            }
            default:
            {
                break;
            }
            }
        }

        return true;
    }
}


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