#include "Networking/NetworkClient.h"

#include <string>

#include "Core/IcarianDefer.h"
#include "Networking/NetworkManager.h"

NetworkClient::NetworkClient()
{
    m_server = -1;
    m_addr = -1;
}
NetworkClient::NetworkClient(NetworkManager* a_manager, uint32_t a_server, const ENetEvent& a_event)
{
    m_manager = a_manager;
    m_server = a_server;

    m_host = a_event.peer->host;
    m_peer = a_event.peer;
}
NetworkClient::~NetworkClient()
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
            }
        }

        enet_peer_reset(m_peer);

End:;
    }

    const bool isServerSocket = m_server != -1;
    if (!isServerSocket && m_host != NULL)
    {
        enet_host_destroy(m_host);
    }    
}

NetworkClient* NetworkClient::Connect(NetworkManager* a_manager, const NetworkAddress& a_address)
{
    ENetHost* host = enet_host_create(NULL, 1, 2, 0, 0);

    if (host != NULL)
    {
        ENetAddress enetAddress;

        const std::string ip = std::to_string((a_address.Address >> 24) & 0xFF ) + "." + std::to_string((a_address.Address >> 16) & 0xFF) + "." + std::to_string((a_address.Address >> 8) & 0xFF) + "." + std::to_string((a_address.Address >> 0) & 0xFF);

        // enetAddress.host = (enet_uint32)address.Address;
        enet_address_set_host_ip(&enetAddress, ip.c_str());
        enetAddress.port = (enet_uint16)a_address.Port;

        ENetPeer* peer = enet_host_connect(host, &enetAddress, 2, 0);

        if (peer != NULL)
        {
            ENetEvent event;

            if (enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
            {
                NetworkClient* client = new NetworkClient();
                client->m_manager = a_manager;
                client->m_host = host;
                client->m_peer = peer;

                return client;
            }

            enet_peer_reset(peer);
        }

        enet_host_destroy(host);
    }

    return nullptr;
}

void NetworkClient::Send(const uint8_t* a_data, uint32_t a_size, e_PacketFlags a_flags)
{
    if (m_peer == NULL)
    {
        return;
    }

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

    ENetPacket* packet = enet_packet_create(a_data, a_size, flags);
    enet_peer_send(m_peer, 0, packet);
}

void NetworkClient::Update()
{
    const bool isServerSocket = m_server != -1;
    if (isServerSocket)
    {
        return;
    }

    const bool isValid = m_host != NULL && m_peer != NULL;
    if (!isValid)
    {
        return;
    }

    ENetEvent event;

    int32_t result = enet_host_service(m_host, &event, 0);
    while (result > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
        {
            IDEFER(enet_packet_destroy(event.packet));

            m_manager->NetworkClientReceive(m_addr, (uint8_t*)event.packet->data, (uint32_t)event.packet->dataLength);

            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            m_manager->NetworkClientDisconnect(m_addr);

            break;
        }
        }

        result = enet_host_service(m_host, &event, 0);
    }

    if (result < 0)
    {
        m_manager->NetworkClientDisconnect(m_addr, true);

        enet_peer_reset(m_peer);
        m_peer = NULL;

        return;
    }
}