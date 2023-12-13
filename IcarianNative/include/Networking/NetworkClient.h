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