// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Audio
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    struct AudioMixerBuffer
    {
        public float Gain;
    };

    public class AudioMixer : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateAudioMixer();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyAudioMixer(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static AudioMixerBuffer GetAudioMixerBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetAudioMixerBuffer(uint a_addr, AudioMixerBuffer a_buffer);

        string m_name = string.Empty;

        uint   m_bufferAddr = uint.MaxValue;

        internal uint InternalAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        /// <summary>
        /// Whether or not the AudioMixer has been disposed.
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// The name of the AudioMixer.
        /// </summary>
        public string Name
        {
            get
            {
                return m_name;
            }
        }

        /// <summary>
        /// The gain of the AudioMixer.
        /// </summary>
        public float Gain
        {
            get
            {
                AudioMixerBuffer buffer = GetAudioMixerBuffer(m_bufferAddr);

                return buffer.Gain;
            }
            set
            {
                AudioMixerBuffer buffer = GetAudioMixerBuffer(m_bufferAddr);

                buffer.Gain = value;

                SetAudioMixerBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Creates a new AudioMixer.
        /// </summary>
        /// <param name="a_name">The name of the AudioMixer.</param>
        public AudioMixer(string a_name = "")
        {
            m_name = a_name;

            m_bufferAddr = GenerateAudioMixer();
        }

        /// <summary>
        /// Destroys the AudioMixer.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Called when the AudioMixer is destroyed.
        /// </summary>
        /// <param name="a_disposing">Whether or not the AudioMixer is being disposed.</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    DestroyAudioMixer(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("AudioMixer failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("AudioMixer has already been Disposed");
            }
        }

        ~AudioMixer()
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