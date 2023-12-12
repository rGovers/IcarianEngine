using System;

namespace IcarianEngine.Networking
{
    public abstract class NetworkSocket : IDestroy
    {
        /// <summary>
        /// Delegate for when the NetworkSocket receives data
        /// </summary>
        /// <param name="a_socket">NetworkSocket that received the data</param>
        /// <param name="a_data">Data received</param>
        public delegate void ReceiveCallback(NetworkSocket a_socket, byte[] a_data);
        /// <summary>
        /// Delegate for when the NetworkSocket is disconnected
        /// </summary>
        /// <param name="a_socket">NetworkSocket that was disconnected</param>
        /// <param name="a_error">True if the NetworkSocket was disconnected due to an error</param>
        public delegate void DisconnectCallback(NetworkSocket a_socket, bool a_error);

        /// <summary>
        /// Called when the NetworkSocket receives data
        /// </summary>
        public ReceiveCallback OnReceive;
        /// <summary>
        /// Called when the NetworkSocket is disconnected
        /// </summary>
        public DisconnectCallback OnDisconnect;

        /// <summary>
        /// Whether the NetworkSocket has been disposed
        /// </summary>
        public abstract bool IsDisposed { get; }

        ~NetworkSocket()
        {
            Dispose(false);
        }
        /// <summary>
        /// Destroy the NetworkSocket
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the NetworkSocket is destroyed
        /// </summary>
        /// <param name="a_disposing">True if called from Dispose, false if called from the finalizer</param>
        protected abstract void Dispose(bool a_disposing);

        /// <summary>
        /// Send data through the NetworkSocket
        /// </summary>
        /// <param name="a_data">Data to send</param>
        /// <param name="a_flags">Flags for the packet</param>
        public abstract void Send(byte[] a_data, PacketFlags a_flags = PacketFlags.None);
    }
}
