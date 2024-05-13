using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineNetworkInteropStructures.h"
#include "EngineNetworkManagerInterop.h"
#include "InteropBinding.h"

ENGINE_NETWORKMANAGER_EXPORT_TABLE(IOP_BIND_FUNCTION)

namespace IcarianEngine.Networking
{
    public static class NetworkManager
    {
        /// <summary>
        /// Whether or not the network manager has been initialized.
        /// </summary>
        public static bool IsInitialized
        {
            get
            {
                return NetworkManagerInterop.IsInitialized() != 0;
            }
        }
    }
}