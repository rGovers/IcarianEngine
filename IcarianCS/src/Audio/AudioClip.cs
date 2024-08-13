// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Audio
{
    public class AudioClip : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFromFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyAudioClip(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float GetAudioClipDuration(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetAudioClipSampleRate(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetAudioClipChannelCount(uint a_addr);

        static ConcurrentDictionary<uint, AudioClip> s_audioClips = new ConcurrentDictionary<uint, AudioClip>();

        uint m_bufferAddr = uint.MaxValue;

        internal uint InternalAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }
        
        /// <summary>
        /// Whether or not the AudioClip has been disposed.
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// The duration of the AudioClip in seconds.
        /// </summary>
        public float Duration
        {
            get
            {
                return GetAudioClipDuration(m_bufferAddr);
            }
        }

        /// <summary>
        /// The sample rate of the AudioClip.
        /// </summary>
        public uint SampleRate
        {
            get
            {
                return GetAudioClipSampleRate(m_bufferAddr);
            }
        }
        /// <summary>
        /// The number of channels in the AudioClip.
        /// </summary>
        public uint ChannelCount
        {
            get
            {
                return GetAudioClipChannelCount(m_bufferAddr);
            }
        }

        internal static AudioClip GetAudioClip(uint a_addr)
        {
            if (s_audioClips.TryGetValue(a_addr, out AudioClip clip))
            {
                return clip;
            }

            return null;
        }

        AudioClip(uint a_addr)
        {
            m_bufferAddr = a_addr;

            s_audioClips.TryAdd(a_addr, this);
        }

        /// <summary>
        /// Loads an AudioClip from the given path.
        /// </summary>
        /// Supported formats: 
        ///     OGG
        ///     WAV
        /// <param name="a_path">The path to the AudioClip.</param>
        /// <returns>The AudioClip if it was loaded successfully, null otherwise.</returns>
        /// @see AssetLibrary.LoadAudioClip
        public static AudioClip LoadAudioClip(string a_path)
        {
            uint addr = GenerateFromFile(a_path);
            if (addr != uint.MaxValue)
            {
                return new AudioClip(addr);
            }

            Logger.IcarianWarning($"AudioClip failed to load: {a_path}");

            return null;
        }

        /// <summary>
        /// Destroys the AudioClip.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Called when the AudioClip is being disposed.
        /// </summary>
        /// <param name="a_disposing">Whether or not the AudioClip is being disposed.</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    DestroyAudioClip(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("AudioClip failed to Dispose");
                }

                s_audioClips.TryRemove(m_bufferAddr, out AudioClip _);
                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("AudioClip already Disposed");
            }
        }

        ~AudioClip()
        {
            Dispose(false);
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