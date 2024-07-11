using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineApplicationInteropStructures.h"

namespace IcarianEngine
{
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

        /// <summary>
        /// The current working directory of the Application
        /// </summary>
        public static string WorkingDirectory
        {
            get;
            internal set;
        }

        /// <summary>
        /// The width of the Application
        /// </summary>
        public static uint Width
        {
            get
            {
                return GetWidth();
            }
        }
        /// <summary>
        /// The height of the Application
        /// </summary>
        public static uint Height
        {
            get
            {
                return GetHeight();
            }
        }

        /// <summary>
        /// Whether the Application is running is headless mode
        /// </summary>
        public static bool IsHeadless
        {
            get
            {
                return GetHeadlessState() != 0;
            }
        }

        /// <summary>
        /// Whether the Application is running in the IcarianEditor
        /// </summary>
        public static bool IsEditor
        {
            get
            {
                return GetEditorState() != 0;
            }
        }

        /// <summary>
        /// Sets the fullscreen state of the Application
        /// </summary>
        /// <param name="a_monitor">The <see cref="IcarianEngine.Monitor" /> for the Application to be on</param>
        /// <param name="a_state">The fullscreen state to set the Application to</param>
        /// <param name="a_width">The target screen resolution width for the Application</param>
        /// <param name="a_height">The target screen resolution height for the Application</param>
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