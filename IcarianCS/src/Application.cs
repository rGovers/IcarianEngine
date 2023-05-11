using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Monitor
    {
        public uint Index;
        public string Name;
        public uint Width;
        public uint Height;
        IntPtr Handle;
    }

    public static class Application
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetWidth();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetHeight();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetHeadlessState();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetEditorState();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetFullscreenState(Monitor a_monitor, uint a_state, uint a_width, uint a_height);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void Close(); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void Resize(uint a_width, uint a_height);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Monitor[] GetMonitors();

        public static string WorkingDirectory
        {
            get;
            internal set;
        }

        public static uint Width
        {
            get
            {
                return GetWidth();
            }
        }
        public static uint Height
        {
            get
            {
                return GetHeight();
            }
        }

        public static bool IsHeadless
        {
            get
            {
                return GetHeadlessState() != 0;
            }
        }

        public static bool IsEditor
        {
            get
            {
                return GetEditorState() != 0;
            }
        }

        public static void SetFullscreen(Monitor a_monitor, bool a_state, uint a_width, uint a_height)
        {
            if (a_state)
            {
                SetFullscreenState(a_monitor, 1, a_width, a_height);
            }
            else
            {
                SetFullscreenState(a_monitor, 0, a_width, a_height);
            }
        }
    }
}