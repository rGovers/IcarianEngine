using System;
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
        static ConcurrentDictionary<uint, NetworkClient> s_clients = new ConcurrentDictionary<uint, NetworkClient>();

        uint m_bufferAddr;

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

        NetworkClient(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

            s_clients.TryAdd(a_bufferAddr, this);
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