// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineAudioMixerInterop.h"
#include "EngineAudioMixerInteropStructures.h"
#include "InteropBinding.h"

ENGINE_AUDIOMIXER_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Audio
{
    public class AudioMixer : IDestroy
    {
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
        /// Whether the AudioMixer has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// The name of the AudioMixer
        /// </summary>
        public string Name
        {
            get
            {
                return m_name;
            }
        }

        /// <summary>
        /// The gain of the AudioMixer
        /// </summary>
        public float Gain
        {
            get
            {
                AudioMixerBuffer buffer = AudioMixerInterop.GetAudioMixerBuffer(m_bufferAddr);

                return buffer.Gain;
            }
            set
            {
                AudioMixerBuffer buffer = AudioMixerInterop.GetAudioMixerBuffer(m_bufferAddr);

                buffer.Gain = value;

                AudioMixerInterop.SetAudioMixerBuffer(m_bufferAddr, buffer);
            }
        }

        public AudioMixer(string a_name = "")
        {
            m_name = a_name;

            m_bufferAddr = AudioMixerInterop.GenerateAudioMixer();
        }

        /// <summary>
        /// Disposes of the AudioMixer
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the AudioMixer is Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether the AudioMixer is being Disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    AudioMixerInterop.DestroyAudioMixer(m_bufferAddr);
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