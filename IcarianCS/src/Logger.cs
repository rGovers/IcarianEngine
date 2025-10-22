// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System.Runtime.CompilerServices;

#ifdef ENABLE_STACKTRACE
using System;
using System.Diagnostics;
using System.Reflection;
#endif

namespace IcarianEngine
{
    public static class Logger
    {
        public delegate void MessageStream(string a_msg);

        public static MessageStream MessageCallback = null;
        public static MessageStream WarningCallback = null;
        public static MessageStream ErrorCallback = null;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void PushMessage(string a_message, string[] a_stackTrace);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void PushWarning(string a_message, string[] a_stackTrace);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void PushError(string a_message, string[] a_stackTrace);

        internal static void IcarianMessage(string a_message)
        {
            if (Application.IsEditor)
            {
                Message($"Editor: {a_message}");
            }
            else
            {
                Message(a_message);
            }
        }
        internal static void IcarianWarning(string a_message)
        {
            if (Application.IsEditor)
            {
                Warning($"Editor: {a_message}");
            }
            else
            {
                Warning(a_message);
            }
        }
        internal static void IcarianError(string a_message)
        {
            if (Application.IsEditor)
            {
                Error($"Editor: {a_message}");
            }
            else
            {
                Error(a_message);
            }
        }

        static string[] GetStackTrace()
        {
#ifdef ENABLE_STACKTRACE
            StackTrace stackTrace = new StackTrace(true);

            uint frameCount = (uint)stackTrace.FrameCount;

            uint startFrame = 0;
            while (true)
            {
                if (startFrame >= frameCount)
                {
                    return null;
                }

                StackFrame sf = stackTrace.GetFrame((int)startFrame);

                MethodBase method = sf.GetMethod();
                Type decType = method.DeclaringType;
                if (decType != typeof(Logger))
                {
                    break;
                }

                ++startFrame;
            }

            uint stackSize = frameCount - startFrame;
            string[] stack = new string[stackSize];

            for (uint i = 0; i < stackSize; ++i)
            {
                string stackStr = string.Empty;

                StackFrame sf = stackTrace.GetFrame((int)(i + startFrame));

                MethodBase method = sf.GetMethod();
                Type decType = method.DeclaringType;

                stackStr += $"{decType}:{method.Name}";

                string filename = sf.GetFileName();
                if (!string.IsNullOrWhiteSpace(filename))
                {
                    int lineNum = sf.GetFileLineNumber();
                    int columnNum = sf.GetFileColumnNumber();

                    if (lineNum != 0 && columnNum != 0)
                    {
                        stackStr += $" [{filename}:{lineNum},{columnNum}]";
                    }
                    else
                    {
                        stackStr += $" [{filename}]";
                    }
                }

                stack[i] = stackStr;
            }

            return stack;
#endif

            return null;
        }

        static string FormatMessage(string a_msg)
        {
#ifdef ENABLE_STACKTRACE
            StackTrace stackTrace = new StackTrace(true);

            uint frameCount = (uint)stackTrace.FrameCount;
            for (uint i = 0; i < frameCount; ++i)
            {
                StackFrame sf = stackTrace.GetFrame((int)i);

                MethodBase method = sf.GetMethod();
                Type decType = method.DeclaringType;
                if (decType == typeof(Logger))
                {
                    continue;
                }

                Assembly asm = decType.Assembly;
                AssemblyName name = asm.GetName();

                return $"[{name.Name}] {a_msg}";
            }
#endif

            return a_msg;
        }

        public static void Message(string a_message)
        {
            string pMsg = FormatMessage(a_message);
            string[] stackTrace = GetStackTrace();

            PushMessage(pMsg, stackTrace);
            if (MessageCallback != null)
            {
                MessageCallback(a_message);
            }
        }
        public static void Warning(string a_message)
        {
            string pMsg = FormatMessage(a_message);
            string[] stackTrace = GetStackTrace();

            PushWarning(pMsg, stackTrace);
            if (WarningCallback != null)
            {
                WarningCallback(a_message);
            }
        }
        public static void Error(string a_message)
        {
            string pMsg = FormatMessage(a_message);
            string[] stackTrace = GetStackTrace();

            PushError(pMsg, stackTrace);
            if (ErrorCallback != null)
            {
                ErrorCallback(a_message);
            }
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
