// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System.Collections.Concurrent;

namespace IcarianEngine
{
    public static class PipeMessage
    {
        public delegate void MessageCallback(string a_type, byte[] a_data);

        static ConcurrentDictionary<string, MessageCallback> s_callbacks;

        static void Init()
        {
            s_callbacks = new ConcurrentDictionary<string, MessageCallback>();
        }

        static void ReceiveMessage(string a_type, byte[] a_data)
        {
            MessageCallback callback;
            if (!s_callbacks.TryGetValue(a_type, out callback))
            {
                return;
            }

            if (callback != null)
            {
                callback(a_type, a_data);
            }
        }
        public static bool AddCallback(string a_type, MessageCallback a_callback)
        {
            MessageCallback callback;
            if (s_callbacks.TryGetValue(a_type, out callback))
            {
                return s_callbacks.TryUpdate(a_type, a_callback, callback);
            }

            return s_callbacks.TryAdd(a_type, a_callback);
        }
    }
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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