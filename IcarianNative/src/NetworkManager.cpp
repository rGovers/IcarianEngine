#include "Networking/NetworkManager.h"

#include <enet/enet.h>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
#include "Networking/NetworkClient.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"

#include "EngineNetworkManagerInterop.h"
#include "EngineNetworkClientInterop.h"

static NetworkManager* Instance = nullptr;

ENGINE_NETWORKMANAGER_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_NETWORKCLIENT_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

NetworkManager::NetworkManager()
{
    Instance = this;

    m_initialized = enet_initialize() == 0;

    ENGINE_NETWORKMANAGER_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_NETWORKCLIENT_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

    m_networkClientReceiveFunction = RuntimeManager::GetFunction("IcarianEngine.Networking", "NetworkClient", ":Receive(uint,byte[])");
    m_networkClientDisconnectFunction = RuntimeManager::GetFunction("IcarianEngine.Networking", "NetworkClient", ":Disconnect(uint,uint)");
}
NetworkManager::~NetworkManager()
{
    delete m_networkClientReceiveFunction;
    delete m_networkClientDisconnectFunction;

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

void NetworkManager::Update()
{
    if (!m_initialized)
    {
        return;
    }

    const TReadLockArray<NetworkClient*> clients = m_clients.ToReadLockArray();
    const std::vector<bool> states = m_clients.ToStateVector();
    const uint32_t size = clients.Size();

    for (uint32_t i = 0; i < size; ++i)
    {
        if (states[i])
        {
            clients[i]->Update();
        }
    }
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

    ICARIAN_ASSERT_MSG(a_addr < m_clients.Size(), "NetworkClientReceive out of bounds.");
    ICARIAN_ASSERT_MSG(m_clients.Exists(a_addr), "NetworkClientReceive already destroyed.");

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

    ICARIAN_ASSERT_MSG(a_addr < m_clients.Size(), "NetworkClientDisconnect out of bounds.");
    ICARIAN_ASSERT_MSG(m_clients.Exists(a_addr), "NetworkClientDisconnect already destroyed.");

    uint32_t error = 0;
    if (a_error)
    {
        error = 1;
    }

    void* args[] =
    {
        &a_addr,
        &error
    };

    m_networkClientDisconnectFunction->Exec(args);
}