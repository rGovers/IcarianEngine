// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.