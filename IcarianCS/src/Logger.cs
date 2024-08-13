// Icarian Engine - C# Game Engine
// 
// License at end of file.

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