#pragma once

#include "InteropTypes.h"

/// @cond INTERNAL

#define ENGINE_NETWORKCLIENT_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine.Networking, NetworkClientInterop, Connect, \
    { \
        return (uint32_t)Instance->CreateNetworkClient(a_addr); \
    }, NetworkAddress a_addr) \
    F(void, IcarianEngine.Networking, NetworkClientInterop, Destroy, \
    { \
        DeletionQueue::Push(new NetworkClientDeletionObject(a_addr), DeletionIndex_Update); \
    }, IOP_UINT32 a_addr) \
    \
    F(IOP_UINT32, IcarianEngine.Networking, NetworkClientInterop, GetServerAddress, \
    { \
        return Instance->NetworkClientGetServerAddress(a_addr); \
    }, IOP_UINT32 a_addr) \
    \
    F(void, IcarianEngine.Networking, NetworkClientInterop, Send, \
    { \
        const uint32_t size = (uint32_t)mono_array_length(a_data); \
        uint8_t* data = new uint8_t[size]; \
        IDEFER(delete[] data); \
        \
        for (uint32_t i = 0; i < size; ++i) \
        { \
            data[i] = *(uint8_t*)mono_array_addr(a_data, uint8_t, i); \
        } \
        \
        Instance->NetworkClientSend(a_addr, data, size, (e_PacketFlags)a_flags); \
    }, IOP_UINT32 a_addr, IOP_ARRAY(byte[]) a_data, IOP_UINT32 a_flags) \
    
/// @endcond