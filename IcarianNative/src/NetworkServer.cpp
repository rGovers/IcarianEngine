#include "Networking/NetworkServer.h"

#include <cstring>

#include "Core/IcarianDefer.h"
#include "Networking/NetworkManager.h"

constexpr static enet_uint32 GetFlags(e_PacketFlags a_flags)
{
    enet_uint32 flags = 0;
    if (a_flags & PacketFlags_Reliable)
    {
        flags |= ENET_PACKET_FLAG_RELIABLE;
    }
    if (a_flags & PacketFlags_Unsequenced)
    {
        flags |= ENET_PACKET_FLAG_UNSEQUENCED;
    }
    if (a_flags & PacketFlags_UnreliableFragment)
    {
        flags |= ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
    }

    return flags;
}

NetworkServer::NetworkServer(uint32_t a_maxClients)
{
    m_maxClients = a_maxClients;

    m_peers = new NetworkPeer[m_maxClients];
    for (uint32_t i = 0; i < m_maxClients; ++i)
    {
        m_peers[i].Addr = -1;
        m_peers[i].Peer = NULL;
    }
}
NetworkServer::~NetworkServer()
{
    for (uint32_t i = 0; i < m_maxClients; ++i)
    {
        if (m_peers[i].Addr != -1)
        {
            m_manager->DestroyNetworkClient(m_peers[i].Addr);
        }
    }

    delete[] m_peers;

    if (m_host != NULL)
    {
        enet_host_destroy(m_host);
    }
}

NetworkServer* NetworkServer::Create(NetworkManager* a_manager, uint16_t a_port, uint32_t a_maxClients)
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = a_port;

    ENetHost* host = enet_host_create(&address, a_maxClients, 2, 0, 0);
    if (host == NULL)
    {
        return NULL;
    }

    NetworkServer* server = new NetworkServer(a_maxClients);
    server->m_manager = a_manager;
    server->m_host = host;

    return server;
}

void NetworkServer::Send(const uint8_t* a_data, uint32_t a_size, e_PacketFlags a_flags)
{
    const enet_uint32 flags = GetFlags(a_flags);

    ENetPacket* packet = enet_packet_create(a_data, a_size, flags);
    enet_host_broadcast(m_host, 0, packet);
}

void NetworkServer::Update()
{
    ENetEvent event;

    int32_t ret = enet_host_service(m_host, &event, 0);
    while (ret > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            for (uint32_t i = 0; i < m_maxClients; ++i)
            {
                if (m_peers[i].Addr == -1)
                {
                    const uint32_t addr = m_manager->CreateNetworkClientConnection(m_addr, event);

                    m_peers[i].Addr = addr;
                    m_peers[i].Peer = event.peer;

                    m_manager->NetworkServerConnect(m_addr, addr);

                    break;
                }
            }

            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            IDEFER(enet_packet_destroy(event.packet));

            for (uint32_t i = 0; i < m_maxClients; ++i)
            {
                if (m_peers[i].Peer == event.peer)
                {
                    m_manager->NetworkClientReceive(m_peers[i].Addr, (uint8_t*)event.packet->data, (uint32_t)event.packet->dataLength);

                    break;
                }
            }

            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            for (uint32_t i = 0; i < m_maxClients; ++i)
            {
                if (m_peers[i].Peer == event.peer)
                {
                    m_manager->NetworkClientDisconnect(m_peers[i].Addr);
                    m_manager->DestroyNetworkClient(m_peers[i].Addr);

                    m_peers[i].Addr = -1;
                    m_peers[i].Peer = NULL;

                    break;
                }
            }

            break;
        }
        }

        ret = enet_host_service(m_host, &event, 0);
    }

    if (ret < 0)
    {
        if (event.peer == NULL)
        {
            return;
        }

        for (uint32_t i = 0; i < m_maxClients; ++i)
        {
            if (m_peers[i].Peer == event.peer)
            {
                m_manager->NetworkClientDisconnect(m_peers[i].Addr, true);
                m_manager->DestroyNetworkClient(m_peers[i].Addr);

                m_peers[i].Addr = -1;
                m_peers[i].Peer = NULL;

                break;
            }
        }
    }
}