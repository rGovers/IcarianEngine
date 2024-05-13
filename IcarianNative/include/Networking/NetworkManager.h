#pragma once

#include <enet/enet.h>

#include "DataTypes/TNCArray.h"

class NetworkClient;
class NetworkServer;
class RuntimeFunction;

struct NetworkPeer;

#include "EngineNetworkInteropStructures.h"

class NetworkManager
{
private:
    bool                     m_initialized;

    TNCArray<NetworkClient*> m_clients;
    TNCArray<NetworkServer*> m_servers;

    RuntimeFunction*         m_networkClientReceiveFunction;
    RuntimeFunction*         m_networkClientDisconnectFunction;

    RuntimeFunction*         m_networkServerConnectFunction;

protected:

public:
    NetworkManager();
    ~NetworkManager();

    inline bool IsInitialized() const
    {
        return m_initialized;
    }

    uint32_t CreateNetworkClient(const NetworkAddress& a_address);
    uint32_t CreateNetworkClientConnection(uint32_t a_hostAddr, const ENetEvent& a_event);
    void DestroyNetworkClient(uint32_t a_addr);
    uint32_t NetworkClientGetServerAddress(uint32_t a_addr);

    void NetworkClientSend(uint32_t a_addr, const uint8_t* a_data, uint32_t a_size, e_PacketFlags a_flags);
    void NetworkClientReceive(uint32_t a_addr, const uint8_t* a_data, uint32_t a_size);
    void NetworkClientDisconnect(uint32_t a_addr, bool a_error = false);

    uint32_t CreateNetworkServer(uint16_t a_port, uint32_t a_maxClients);
    void DestroyNetworkServer(uint32_t a_addr);

    void NetworkServerSend(uint32_t a_addr, const uint8_t* a_data, uint32_t a_size, e_PacketFlags a_flags);
    void NetworkServerConnect(uint32_t a_addr, uint32_t a_clientAddr);
    uint32_t NetworkServerGetMaxClients(uint32_t a_addr);
    NetworkPeer* NetworkServerGetClients(uint32_t a_addr);

    void Update();
};