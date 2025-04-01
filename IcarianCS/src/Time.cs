// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System.Runtime.CompilerServices;

#include "EngineTimeInterop.h"
#include "InteropBinding.h"

ENGINE_TIME_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine
{
    public static class Time
    {
        static float s_timeScale = 1.0f;

        static double s_deltaTime = 0.0;
        static double s_time = 0.0;

        static double s_fixedDeltaTime = 0.0;
        static double s_fixedTime = 0.0;

        static double s_frameDeltaTime = 0.0;
        static double s_frameTime = 0.0;

        public static float TimeScale
        {
            get
            {
                return s_timeScale;
            }
            set
            {
                if (s_timeScale != value)
                {
                    s_timeScale = value;

                    TimeInterop.SetTimeScale(s_timeScale);
                }
            }
        }

        /// <summary>
        /// Delta time in seconds as a double scaled by TimeScale
        /// </summary>
        public static double DDeltaTime
        {
            get
            {
                return s_deltaTime * s_timeScale;
            }
            internal set
            {
                s_deltaTime = value;
            }
        }
        /// <summary>
        /// Time passed in seconds as a double.
        /// </summary>
        public static double DTimePassed
        {
            get
            {
                return s_time;
            }
            internal set
            {
                s_time = value;
            }
        }

        /// <summary>
        /// Delta time in seconds as a float scaled by TimeScale
        /// </summary>
        public static float DeltaTime
        {
            get
            {
                return (float)s_deltaTime * s_timeScale;
            }
        }
        public static float UnscaledDeltaTime
        {
            get
            {
                return (float)s_deltaTime;
            }
        }

        /// <summary>
        /// Time passed in seconds as a float
        /// </summary>
        public static float TimePassed
        {
            get
            {
                return (float)s_time;
            }
        }

        /// <summary>
        /// Fixed delta time in seconds as a double scaled by TimeScale
        /// </summary>
        public static double DFixedDeltaTime
        {
            get
            {
                return s_fixedDeltaTime * s_timeScale;
            }
            internal set
            {
                s_fixedDeltaTime = value;
            }
        }
        /// <summary>
        /// Fixed time passed in seconds as a double
        /// </summary>
        public static double DFixedTimePassed
        {
            get
            {
                return s_fixedTime * s_timeScale;
            }
            internal set
            {
                s_fixedTime = value;
            }
        }

        /// <summary>
        /// Fixed delta time in seconds as a float scaled by TimeScale
        /// </summary>
        public static float FixedDeltaTime
        {
            get
            {
                return (float)s_fixedDeltaTime * s_timeScale;
            }
        }
        public static float UnscaledFixedDeltaTime
        {
            get
            {
                return (float)s_fixedDeltaTime;
            }
        }

        /// <summary>
        /// Fixed time passed in seconds as a float
        /// </summary>
        public static float FixedTimePassed
        {
            get
            {
                return (float)s_fixedTime;
            }
        }

        /// <summary>
        /// Frame delta time in seconds as a double scaled by TimeScale
        /// </summary>
        public static double DFrameDeltaTime
        {
            get
            {
                return s_frameDeltaTime * s_timeScale;
            }
            internal set
            {
                s_frameDeltaTime = value;
            }
        }
        /// <summary>
        /// Frame time passed in seconds as a double
        /// </summary>
        public static double DFrameTimePassed
        {
            get
            {
                return s_frameTime;
            }
            internal set
            {
                s_frameTime = value;
            }
        }

        /// <summary>
        /// Frame delta time in seconds as a float scaled by TimeScale
        /// </summary>
        public static float FrameDeltaTime
        {
            get
            {
                return (float)s_frameDeltaTime * s_timeScale;
            }
        }
        public static float UnscaledFrameDeltaTime
        {
            get
            {
                return (float)s_frameDeltaTime;
            }
        }

        /// <summary>
        /// Frame time passed in seconds as a float
        /// </summary>
        public static float FrameTimePassed
        {
            get
            {
                return (float)s_frameTime;
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