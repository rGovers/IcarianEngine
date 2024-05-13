#pragma once

#include "InteropTypes.h"

/// @cond INTERNAL

#define ENGINE_NETWORKSERVER_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Networking, NetworkServerInterop, Create, \
    { \
        return Instance->CreateNetworkServer((uint16_t)a_port, a_maxConnections); \
    }, IOP_UINT32 a_port, IOP_UINT32 a_maxConnections) \
    F(void, IcarianEngine.Networking, NetworkServerInterop, Destroy, \
    { \
        Instance->DestroyNetworkServer(a_addr); \
    }, IOP_UINT32 a_addr) \
    \
    F(void, IcarianEngine.Networking, NetworkServerInterop, Send, \
    { \
        const uint32_t size = (uint32_t)mono_array_length(a_data); \
        uint8_t* data = new uint8_t[size]; \
        IDEFER(delete[] data); \
        \
        for (uint32_t i = 0; i < size; ++i) \
        { \
            data[i] = *(uint8_t*)mono_array_addr(a_data, uint8_t, i); \
        } \
        Instance->NetworkServerSend(a_addr, data, size, (e_PacketFlags)a_flags); \
    }, IOP_UINT32 a_addr, IOP_ARRAY(byte[]) a_data, IOP_UINT32 a_flags) \
    F(IOP_UINT32, IcarianEngine.Networking, NetworkServerInterop, GetMaxClients, \
    { \
        return Instance->NetworkServerGetMaxClients(a_addr); \
    }, IOP_UINT32 a_addr) \
    F(IOP_ARRAY(uint[]), IcarianEngine.Networking, NetworkServerInterop, GetClients, \
    { \
        NetworkPeer* peers = Instance->NetworkServerGetClients(a_addr); \
        if (peers == NULL) \
        { \
            return NULL; \
        } \
        \
        const uint32_t size = Instance->NetworkServerGetMaxClients(a_addr); \
        MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_uint32_class(), size); \
        \
        for (uint32_t i = 0; i < size; ++i) \
        { \
            mono_array_set(arr, uint32_t, i, peers[i].Addr); \
        } \
        return arr; \
    }, IOP_UINT32 a_addr) \

/// @endcond