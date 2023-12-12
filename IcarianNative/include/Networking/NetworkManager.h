#pragma once

#include "DataTypes/TNCArray.h"

class NetworkClient;
class RuntimeFunction;

#include "EngineNetworkInteropStructures.h"

class NetworkManager
{
private:
    bool                     m_initialized;

    TNCArray<NetworkClient*> m_clients;

    RuntimeFunction*         m_networkClientReceiveFunction;
    RuntimeFunction*         m_networkClientDisconnectFunction;

protected:

public:
    NetworkManager();
    ~NetworkManager();

    inline bool IsInitialized() const
    {
        return m_initialized;
    }

    uint32_t CreateNetworkClient(const NetworkAddress& a_address);
    void DestroyNetworkClient(uint32_t a_addr);

    void NetworkClientSend(uint32_t a_addr, const uint8_t* a_data, uint32_t a_size, e_PacketFlags a_flags);
    void NetworkClientReceive(uint32_t a_addr, const uint8_t* a_data, uint32_t a_size);
    void NetworkClientDisconnect(uint32_t a_addr, bool a_error = false);

    void Update();
};