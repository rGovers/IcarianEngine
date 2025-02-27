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
        m_thread = std::thread(Run, this);

        m_host = NULL;
        m_peer = NULL;

        m_join = false;
        m_joined = false;
    }
    SocketPipe::~SocketPipe()
    {
        m_join = true;

        while (!m_joined)
        {
            std::this_thread::yield();
        }

        m_thread.join();

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
                    goto DesDisconnectEnd;
                }
                default:
                {
                    break;
                }
                }
            }

            enet_peer_reset(m_peer);
DesDisconnectEnd:;
        }

        if (m_host != NULL)
        {
            enet_host_destroy(m_host);
        }
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
        return m_host != NULL && m_peer != NULL && !m_joined;
    }

    void SocketPipe::Run(SocketPipe* a_pipe)
    {
        // Been struggling when there is a large performance gap between to systems one end get overwhelmed and packets get nuked from buffer being full
        // To counteract this dedicate a thread to handling network traffic
        while (a_pipe->m_host != NULL && a_pipe->m_peer != NULL && !a_pipe->m_join)
        {
            bool work = false;

            {
                const std::lock_guard g = std::lock_guard(a_pipe->m_writeLock);

                ENetEvent event;
                while (a_pipe->m_host != NULL && enet_host_service(a_pipe->m_host, &event, 0) > 0)
                {
                    work = true;

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
                            break;
                        }

                        memcpy((char*)&msg, packet->data, PipeMessage::Size);

                        a_pipe->m_writeQueue.emplace(msg);

                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT:
                    {
                        enet_peer_disconnect(a_pipe->m_peer, 0);

                        ENetEvent event;
                        while (enet_host_service(a_pipe->m_host, &event, 3000) > 0)
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
                                goto RunDisconnectEnd;
                            }
                            default:
                            {
                                break;
                            }
                            }
                        }
    
                        enet_peer_reset(a_pipe->m_peer);
RunDisconnectEnd:;

                        a_pipe->m_peer = NULL;

                        enet_host_destroy(a_pipe->m_host);
                        a_pipe->m_host = NULL;

                        a_pipe->m_join = true;

                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            }

            {
                const std::lock_guard g = std::lock_guard(a_pipe->m_readLock);

                while (!a_pipe->m_readQueue.empty())
                {
                    work = true;

                    const PipeMessage& msg = a_pipe->m_readQueue.front();
                    IDEFER(
                    if (msg.Data != nullptr) 
                    {
                        delete[] msg.Data;
                    });

                    a_pipe->m_readQueue.pop();

                    enet_uint8 channel = 0;
                    enet_uint32 flags = 0;
                    switch (msg.Type) 
                    {
                    case PipeMessageType_PushFrame:
                    {
                        channel = 1;
            
                        break;
                    }
                    default:
                    {
                        flags |= ENET_PACKET_FLAG_RELIABLE;
            
                        break;
                    }
                    }
            
                    ENetPacket* packet = enet_packet_create(&msg, (size_t)PipeMessage::Size, flags);
                    if (msg.Data != nullptr && msg.Length > 0)
                    {
                        enet_packet_resize(packet, (size_t)PipeMessage::Size + msg.Length);
            
                        memcpy(packet->data + PipeMessage::Size, msg.Data, msg.Length);
                    }
            
                    if (enet_peer_send(a_pipe->m_peer, channel, packet) < 0)
                    {
                        break;
                    }
                }
            }

            if (!work)
            {
                std::this_thread::yield();
            }
        }

        a_pipe->m_joined = true;
    }

    bool SocketPipe::Send(const PipeMessage& a_msg)
    {
        if (!IsAlive())
        {
            return false;
        }
        
        const std::lock_guard g = std::lock_guard(m_readLock);

        PipeMessage msg;
        msg.Type = a_msg.Type;
        if (a_msg.Length > 0 && a_msg.Data != nullptr)
        {
            msg.Length = a_msg.Length;
            msg.Data = new char[msg.Length];
            memcpy(msg.Data, a_msg.Data, msg.Length);
        }

        m_readQueue.emplace(msg);

        return true;
    }
    bool SocketPipe::Receive(std::queue<PipeMessage>* a_messages)
    {
        if (!IsAlive())
        {
            return false;
        }

        const std::lock_guard g = std::lock_guard(m_writeLock);

        while (!m_writeQueue.empty()) 
        {
            const PipeMessage& msg = m_writeQueue.front();
            a_messages->emplace(msg);
            m_writeQueue.pop();
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