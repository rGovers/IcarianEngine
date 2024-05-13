#include "Networking/NetworkManager.h"

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "DeletionQueue.h"
#include "Logger.h"
#include "Networking/NetworkClient.h"
#include "Networking/NetworkServer.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"

#include "EngineNetworkClientInterop.h"
#include "EngineNetworkManagerInterop.h"
#include "EngineNetworkServerInterop.h"

static NetworkManager* Instance = nullptr;

class NetworkClientDeletionObject : public DeletionObject
{
private:
    uint32_t m_addr;

protected:

public:
    NetworkClientDeletionObject(uint32_t a_addr)
    {
        m_addr = a_addr;
    }
    virtual ~NetworkClientDeletionObject()
    {

    }

    virtual void Destroy()
    {
        Instance->DestroyNetworkClient(m_addr);
    }
};
class NetworkServerDeletionObject : public DeletionObject
{
private:
    uint32_t m_addr;

protected:

public:
    NetworkServerDeletionObject(uint32_t a_addr)
    {
        m_addr = a_addr;
    }
    virtual ~NetworkServerDeletionObject()
    {

    }

    virtual void Destroy()
    {
        Instance->DestroyNetworkServer(m_addr);
    }
};

ENGINE_NETWORKMANAGER_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_NETWORKCLIENT_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_NETWORKSERVER_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

NetworkManager::NetworkManager()
{
    Instance = this;

    m_initialized = enet_initialize() == 0;

    ENGINE_NETWORKMANAGER_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_NETWORKCLIENT_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_NETWORKSERVER_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

    m_networkClientReceiveFunction = RuntimeManager::GetFunction("IcarianEngine.Networking", "NetworkClient", ":Receive(uint,byte[])");
    m_networkClientDisconnectFunction = RuntimeManager::GetFunction("IcarianEngine.Networking", "NetworkClient", ":Disconnect(uint,uint)");

    m_networkServerConnectFunction = RuntimeManager::GetFunction("IcarianEngine.Networking", "NetworkServer", ":Connect(uint,uint)");
}
NetworkManager::~NetworkManager()
{
    delete m_networkClientReceiveFunction;
    delete m_networkClientDisconnectFunction;

    delete m_networkServerConnectFunction;

    for (uint32_t i = 0; i < m_servers.Size(); ++i)
    {
        if (m_servers.Exists(i))
        {
            Logger::Warning("NetworkServer was not destroyed");

            delete m_servers[i];
        }
    }

    for (uint32_t i = 0; i < m_clients.Size(); ++i)
    {
        if (m_clients.Exists(i))
        {
            Logger::Warning("NetworkClient was not destroyed");

            delete m_clients[i];
        }
    }

    if (m_initialized)
    {
        enet_deinitialize();
    }
}

uint32_t NetworkManager::CreateNetworkClient(const NetworkAddress& a_address)
{
    if (!m_initialized)
    {
        return -1;
    }

    NetworkClient* client = NetworkClient::Connect(this, a_address);
    if (client == nullptr)
    {
        return -1;
    }

    const uint32_t addr = m_clients.PushVal(client);
    client->SetBufferAddress(addr);

    return addr;
}
uint32_t NetworkManager::CreateNetworkClientConnection(uint32_t a_hostAddr, const ENetEvent& a_event)
{
    if (!m_initialized)
    {
        return -1;
    }

    NetworkClient* client = new NetworkClient(this, a_hostAddr, a_event);
    const uint32_t addr = m_clients.PushVal(client);
    client->SetBufferAddress(addr);

    return addr;
}
void NetworkManager::DestroyNetworkClient(uint32_t a_addr)
{
    if (!m_initialized)
    {
        return;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_clients.Size(), "DestroyNetworkClient out of bounds.");
    ICARIAN_ASSERT_MSG(m_clients.Exists(a_addr), "DestroyNetworkClient already destroyed.");

    const NetworkClient* client = m_clients[a_addr];
    IDEFER(delete client);

    m_clients.Erase(a_addr);
}
uint32_t NetworkManager::NetworkClientGetServerAddress(uint32_t a_addr)
{
    if (!m_initialized)
    {
        return -1;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_clients.Size(), "NetworkClientGetServerAddress out of bounds.");
    ICARIAN_ASSERT_MSG(m_clients.Exists(a_addr), "NetworkClientGetServerAddress already destroyed.");

    const TReadLockArray<NetworkClient*> a = m_clients.ToReadLockArray();

    return a[a_addr]->GetServerAddress();
}

void NetworkManager::NetworkClientSend(uint32_t a_addr, const uint8_t* a_data, uint32_t a_size, e_PacketFlags a_flags)
{
    if (!m_initialized)
    {
        return;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_clients.Size(), "NetworkClientSend out of bounds.");
    ICARIAN_ASSERT_MSG(m_clients.Exists(a_addr), "NetworkClientSend already destroyed.");

    TLockArray<NetworkClient*> a = m_clients.ToLockArray();

    NetworkClient* client = a[a_addr];
    client->Send(a_data, a_size, a_flags);
}
void NetworkManager::NetworkClientReceive(uint32_t a_addr, const uint8_t* a_data, uint32_t a_size)
{
    if (!m_initialized)
    {
        return;
    }

    MonoDomain* domain = RuntimeManager::GetDomain();
    MonoArray* array = mono_array_new(domain, mono_get_byte_class(), a_size);

    for (uint32_t i = 0; i < a_size; ++i)
    {
        mono_array_set(array, mono_byte, i, a_data[i]);
    }

    void* args[] =
    {
        &a_addr,
        array
    };

    m_networkClientReceiveFunction->Exec(args);
}
void NetworkManager::NetworkClientDisconnect(uint32_t a_addr, bool a_error)
{
    if (!m_initialized)
    {
        return;
    }

    uint32_t error = (uint32_t)a_error;

    void* args[] =
    {
        &a_addr,
        &error
    };

    m_networkClientDisconnectFunction->Exec(args);
}

uint32_t NetworkManager::CreateNetworkServer(uint16_t a_port, uint32_t a_maxClients)
{
    if (!m_initialized)
    {
        return -1;
    }

    NetworkServer* server = NetworkServer::Create(this, a_port, a_maxClients);
    if (server == nullptr)
    {
        return -1;
    }

    const uint32_t addr = m_servers.PushVal(server);
    server->SetBufferAddress(addr);

    return addr;
}
void NetworkManager::DestroyNetworkServer(uint32_t a_addr)
{
    if (!m_initialized)
    {
        return;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_servers.Size(), "DestroyNetworkServer out of bounds.");
    ICARIAN_ASSERT_MSG(m_servers.Exists(a_addr), "DestroyNetworkServer already destroyed.");

    const NetworkServer* server = m_servers[a_addr];
    IDEFER(delete server);

    m_servers.Erase(a_addr);
}

void NetworkManager::NetworkServerSend(uint32_t a_addr, const uint8_t* a_data, uint32_t a_size, e_PacketFlags a_flags)
{
    if (!m_initialized)
    {
        return;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_servers.Size(), "NetworkServerSend out of bounds.");
    ICARIAN_ASSERT_MSG(m_servers.Exists(a_addr), "NetworkServerSend already destroyed.");

    TLockArray<NetworkServer*> a = m_servers.ToLockArray();

    NetworkServer* server = a[a_addr];
    server->Send(a_data, a_size, a_flags);
}
void NetworkManager::NetworkServerConnect(uint32_t a_addr, uint32_t a_clientAddr)
{
    if (!m_initialized)
    {
        return;
    }

    void* args[] =
    {
        &a_addr,
        &a_clientAddr
    };

    m_networkServerConnectFunction->Exec(args);
}
uint32_t NetworkManager::NetworkServerGetMaxClients(uint32_t a_addr)
{
    if (!m_initialized)
    {
        return -1;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_servers.Size(), "NetworkServerGetMaxClients out of bounds.");
    ICARIAN_ASSERT_MSG(m_servers.Exists(a_addr), "NetworkServerGetMaxClients already destroyed.");

    const TReadLockArray<NetworkServer*> a = m_servers.ToReadLockArray();

    return a[a_addr]->GetMaxClients();
}
NetworkPeer* NetworkManager::NetworkServerGetClients(uint32_t a_addr)
{
    if (!m_initialized)
    {
        return nullptr;
    }

    ICARIAN_ASSERT_MSG(a_addr < m_servers.Size(), "NetworkServerGetClients out of bounds.");
    ICARIAN_ASSERT_MSG(m_servers.Exists(a_addr), "NetworkServerGetClients already destroyed.");

    const TReadLockArray<NetworkServer*> a = m_servers.ToReadLockArray();

    return a[a_addr]->GetPeers();
}

void NetworkManager::Update()
{
    if (!m_initialized)
    {
        return;
    }

    {
        const std::vector<bool> states = m_clients.ToStateVector();
        TLockArray<NetworkClient*> clients = m_clients.ToLockArray();
        const uint32_t size = (uint32_t)states.size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (states[i])
            {
                clients[i]->Update();
            }
        }
    }
    
    {
        const std::vector<bool> states = m_servers.ToStateVector();
        TLockArray<NetworkServer*> servers = m_servers.ToLockArray();
        const uint32_t size = (uint32_t)states.size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (states[i])
            {
                servers[i]->Update();
            }
        }
    }   
}