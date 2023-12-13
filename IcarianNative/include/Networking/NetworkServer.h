#pragma once

#include <cstdint>
#include <enet/enet.h>

#include "EngineNetworkInteropStructures.h"

class NetworkManager;

struct NetworkPeer
{
    uint32_t Addr;
    ENetPeer* Peer;
};

class NetworkServer
{
private:
    NetworkManager* m_manager;
    uint32_t        m_addr;

    uint32_t        m_maxClients;
    NetworkPeer*    m_peers;

    ENetHost*       m_host;

    NetworkServer(uint32_t a_maxClients);

protected:

public:
    ~NetworkServer();

    static NetworkServer* Create(NetworkManager* a_manager, uint16_t a_port, uint32_t a_maxClients);  

    inline uint32_t GetMaxClients() const
    {
        return m_maxClients;
    }
    inline NetworkPeer* GetPeers() const
    {
        return m_peers;
    }

    inline void SetBufferAddress(uint32_t a_addr)
    {
        m_addr = a_addr;
    }

    void Send(const uint8_t* a_data, uint32_t a_size, e_PacketFlags a_flags);

    void Update();
};