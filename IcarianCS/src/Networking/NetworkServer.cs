using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineNetworkServerInterop.h"
#include "InteropBinding.h"

ENGINE_NETWORKSERVER_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Networking
{
    public class NetworkServer : NetworkSocket
    {
        /// <summary>
        /// Delegate for when the NetworkServer receives a connection
        /// </summary>
        /// <param name="a_server">NetworkServer that received the connection</param>
        /// <param name="a_client">NetworkClient that connected</param>
        public delegate void ConnectCallback(NetworkServer a_server, NetworkClient a_client);

        static ConcurrentDictionary<uint, NetworkServer> s_servers = new ConcurrentDictionary<uint, NetworkServer>();

        uint m_bufferAddr;

        /// <summary>
        /// Called when the NetworkServer receives a connection
        /// </summary>
        public ConnectCallback OnConnect;

        /// <summary>
        /// Whether the NetworkServer has been disposed
        /// </summary>
        public override bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// The maximum number of connections the NetworkServer can have
        /// </summary>
        public uint MaxConnections
        {
            get
            {
                return NetworkServerInterop.GetMaxClients(m_bufferAddr);
            }
        }

        /// <summary>
        /// The clients connected to the NetworkServer
        /// </summary>
        public IEnumerable<NetworkClient> Clients
        {
            get
            {
                uint[] clientAddrs = NetworkServerInterop.GetClients(m_bufferAddr);

                foreach (uint clientAddr in clientAddrs)
                {
                    if (clientAddr != uint.MaxValue)
                    {
                        NetworkClient client = NetworkClient.GetClient(clientAddr);
                        if (client != null)
                        {
                            yield return client;
                        }
                    }
                }
            }
        }

        NetworkServer(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

            s_servers.TryAdd(a_bufferAddr, this);
        }

        internal static NetworkServer GetServer(uint a_bufferAddr)
        {
            NetworkServer server;
            if (s_servers.TryGetValue(a_bufferAddr, out server))
            {
                return server;
            }

            return null;
        }

        static void Connect(uint a_bufferAddr, uint a_clientAddr)
        {
            NetworkServer server = null;
            if (s_servers.ContainsKey(a_bufferAddr))
            {
                s_servers.TryGetValue(a_bufferAddr, out server);
            }

            NetworkClient client = new NetworkClient(a_clientAddr);
            if (server != null && server.OnConnect != null)
            {
                server.OnConnect(server, client);
            }
        }

        /// <summary>
        /// Create a NetworkServer
        /// </summary>
        /// <param name="a_port">Port to listen on</param>
        /// <returns>NetworkServer if successful, null otherwise</returns>
        public static NetworkServer Create(ushort a_port, uint a_maxConnections = 32)
        {
            uint bufferAddr = NetworkServerInterop.Create(a_port, a_maxConnections);
            if (bufferAddr != uint.MaxValue)
            {
                return new NetworkServer(bufferAddr);
            }

            return null;
        }

        /// <summary>
        /// Send data through the NetworkServer
        /// </summary>
        /// <param name="a_data">Data to send</param>
        /// <param name="a_flags">Flags for the packet</param>
        public override void Send(byte[] a_data, PacketFlags a_flags = PacketFlags.None)
        {
            NetworkServerInterop.Send(m_bufferAddr, a_data, (uint)a_flags);
        }
        /// <summary>
        /// Called when the NetworkServer is destroyed
        /// </summary>
        /// <param name="a_disposing">True if called from Dispose, false if called from the finalizer</param>
        protected override void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    NetworkServerInterop.Destroy(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianError("NetworkServer was not disposed");
                }

                s_servers.TryRemove(m_bufferAddr, out NetworkServer _);

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("NetworkServer was already disposed");
            }
        }
    }
}