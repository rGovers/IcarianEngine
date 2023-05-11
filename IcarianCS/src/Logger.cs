using System.Runtime.CompilerServices;

namespace IcarianEngine
{
    public static class Logger
    {
        public delegate void MessageStream(string a_msg);

        public static MessageStream MessageCallback = null;
        public static MessageStream WarningCallback = null;
        public static MessageStream ErrorCallback = null;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void PushMessage(string a_message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void PushWarning(string a_message);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void PushError(string a_message);

        internal static void IcarianMessage(string a_message)
        {
            if (Application.IsEditor)
            {
                Message($"IcarianCSE: {a_message}");
            }
            else
            {
                Message($"IcarianCS: {a_message}");
            }
        }
        internal static void IcarianWarning(string a_message)
        {
            if (Application.IsEditor)
            {
                Warning($"IcarianCSE: {a_message}");
            }
            else
            {
                Warning($"IcarianCS: {a_message}");
            }
        }
        internal static void IcarianError(string a_message)
        {
            if (Application.IsEditor)
            {
                Error($"IcarianCSE: {a_message}");
            }
            else
            {
                Error($"IcarianCS: {a_message}");
            }
        }

        public static void Message(string a_message)
        {
            PushMessage(a_message);
            if (MessageCallback != null)
            {
                MessageCallback(a_message);
            }
        }
        public static void Warning(string a_message)
        {
            PushWarning(a_message);
            if (WarningCallback != null)
            {
                WarningCallback(a_message);
            }
        }
        public static void Error(string a_message)
        {
            PushError(a_message);
            if (ErrorCallback != null)
            {
                ErrorCallback(a_message);
            }
        }
    }
}