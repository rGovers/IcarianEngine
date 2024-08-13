// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineNetworkClientInterop.h"
#include "InteropBinding.h"

ENGINE_NETWORKCLIENT_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Networking
{
    public class NetworkClient : NetworkSocket
    {
        /// <summary>
        /// Delegate for when the NetworkSocket is disconnected
        /// </summary>
        /// <param name="a_client">NetworkClient that was disconnected</param>
        /// <param name="a_error">True if the NetworkClient was disconnected due to an error</param>
        public delegate void DisconnectCallback(NetworkClient a_client, bool a_error);
        /// <summary>
        /// Delegate for when the NetworkClient receives data
        /// </summary>
        /// <param name="a_client">NetworkClient that received the data</param>
        /// <param name="a_data">Data received</param>
        public delegate void ReceiveCallback(NetworkClient a_client, byte[] a_data);

        static ConcurrentDictionary<uint, NetworkClient> s_clients = new ConcurrentDictionary<uint, NetworkClient>();

        uint m_bufferAddr;

        /// <summary>
        /// Called when the NetworkClient receives data
        /// </summary>
        public ReceiveCallback OnReceive;
        /// <summary>
        /// Called when the NetworkClient is disconnected
        /// </summary>
        public DisconnectCallback OnDisconnect;

        /// <summary>
        /// Whether the NetworkClient has been disposed
        /// </summary>
        public override bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// NetworkServer that the NetworkClient is assosiated with
        /// </summary>
        public NetworkServer Server
        {
            get
            {
                uint serverAddr = NetworkClientInterop.GetServerAddress(m_bufferAddr);
                if (serverAddr == uint.MaxValue)
                {
                    return null;
                }

                return NetworkServer.GetServer(serverAddr);
            }
        }

        internal NetworkClient(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

            s_clients.TryAdd(a_bufferAddr, this);
        }

        internal static NetworkClient GetClient(uint a_bufferAddr)
        {
            NetworkClient client;
            if (s_clients.TryGetValue(a_bufferAddr, out client))
            {
                return client;
            }
            
            return null;
        }

        static void Receive(uint a_bufferAddr, byte[] a_data)
        {
            NetworkClient client;
            if (s_clients.TryGetValue(a_bufferAddr, out client))
            {
                if (client.OnReceive != null)
                {
                    client.OnReceive(client, a_data);
                }
            }
            else
            {
                Logger.IcarianError("Failed to find NetworkClient");
            }
        }
        static void Disconnect(uint a_bufferAddr, uint a_error)
        {
            NetworkClient client;
            if (s_clients.TryGetValue(a_bufferAddr, out client))
            {
                if (client.OnDisconnect != null)
                {
                    client.OnDisconnect(client, a_error != 0);
                }
            }
            else
            {
                Logger.IcarianError("Failed to find NetworkClient");
            }
        }

        /// <summary>
        /// Connect to a server
        /// </summary>
        /// <param name="a_address">Address of the server</param>
        /// <returns>NetworkClient, null on failure</returns>
        public static NetworkClient Connect(NetworkAddress a_address)
        {
            uint bufferAddr = NetworkClientInterop.Connect(a_address);

            if (bufferAddr == uint.MaxValue)
            {
                Logger.IcarianError("Failed to create NetworkClient");

                return null;
            }

            return new NetworkClient(bufferAddr);
        }

        /// <summary>
        /// Send data through the NetworkClient
        /// </summary>
        /// <param name="a_data">Data to send</param>
        /// <param name="a_flags">Flags for the packet</param>
        public override void Send(byte[] a_data, PacketFlags a_flags = PacketFlags.None)
        {
            NetworkClientInterop.Send(m_bufferAddr, a_data, (uint)a_flags);
        }

        /// <summary>
        /// Called when the NetworkClient is destroyed
        /// </summary>
        /// <param name="a_disposing">True if called from Dispose, false if called from the finalizer</param>
        protected override void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    NetworkClientInterop.Destroy(m_bufferAddr);                    
                }
                else
                {
                    Logger.IcarianError("NetworkClient was not disposed");
                }

                s_clients.TryRemove(m_bufferAddr, out NetworkClient _);

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("NetworkClient was already disposed");
            }
        }        
    }
}

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