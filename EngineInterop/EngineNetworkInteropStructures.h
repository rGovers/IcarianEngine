// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine.Networking {
#endif

/// @file EngineNetworkInteropStructures.h

/// <summary>
/// The packet flags.
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(PacketFlags) : IOP_UINT16
{
    IOP_ENUM_VALUE(PacketFlags, None) = 0,
    IOP_ENUM_VALUE(PacketFlags, Reliable) = 0b1 << 0,
    IOP_ENUM_VALUE(PacketFlags, Unsequenced) = 0b1 << 1,
    IOP_ENUM_VALUE(PacketFlags, UnreliableFragment) = 0b1 << 2,
};

IOP_PACKED IOP_CSPUBLIC struct NetworkAddress
{
    /// <summary>
    /// The address of the network.
    /// </summary>
    IOP_CSPUBLIC IOP_UINT32 Address;
    /// <summary>
    /// The port of the network.
    /// </summary>
    IOP_CSPUBLIC IOP_UINT16 Port;

#ifdef CUBE_LANGUAGE_CSHARP
    /// <summary>
    /// Initializes a new instance of the <see cref="NetworkAddress"/> struct.
    /// </summary>
    /// <param name="a_address">The address as combined bytes.</param>
    /// <param name="a_port">The port.</param>
    public NetworkAddress(uint a_address, ushort a_port)
    {
        Address = a_address;
        Port = a_port;
    }
    /// <summary>
    /// Initializes a new instance of the <see cref="NetworkAddress"/> struct.
    /// </summary>
    /// <param name="a_a">The first part of the address.</param>
    /// <param name="a_b">The second part of the address.</param>
    /// <param name="a_c">The third part of the address.</param>
    /// <param name="a_d">The fourth part of the address.</param>
    /// <param name="a_port">The port.</param>
    public NetworkAddress(byte a_a, byte a_b, byte a_c, byte a_d, ushort a_port)
    {
        unchecked
        {
            Address = (uint)((a_a << 24) | (a_b << 16) | (a_c << 8) | a_d);
        }
        Port = a_port;
    }
    /// <summary>
    /// Initializes a new instance of the <see cref="NetworkAddress"/> struct.
    /// </summary>
    /// <param name="a_address">The address as a string.</param>
    public NetworkAddress(string a_address)
    {
        string[] parts = a_address.Split(new char[] { '.', ':' }, StringSplitOptions.RemoveEmptyEntries);
        if (parts.Length != 5)
        {
            throw new ArgumentException("Invalid IP address.");
        }

        Address = (uint)((byte.Parse(parts[0]) << 24) | (byte.Parse(parts[1]) << 16) | (byte.Parse(parts[2]) << 8) | byte.Parse(parts[3]));
        Port = ushort.Parse(parts[4]);
    }
#endif
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif

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