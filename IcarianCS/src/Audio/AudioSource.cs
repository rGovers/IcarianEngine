// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineAudioSourceInteropStructures.h"

namespace IcarianEngine.Audio
{   
    public class AudioSource : Component, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateAudioSource(uint a_transformAddr, uint a_audioClipAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyAudioSource(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PlayAudioSource(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetAudioSourcePlayingState(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static AudioSourceBuffer GetAudioSourceBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetAudioSourceBuffer(uint a_addr, AudioSourceBuffer a_buffer);

        bool       m_disposed = false;
        bool       m_loop = false;
        bool       m_3d = true;
        AudioMixer m_audioMixer = null;

        uint       m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether the AudioSource has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The Def used to create the AudioSource
        /// </summary>
        public AudioSourceDef AudioSourceDef
        {
            get
            {
                return Def as AudioSourceDef;
            }
        }

        /// <summary>
        /// Whether or not the AudioSource is playing
        /// </summary>
        public bool IsPlaying
        {
            get
            {
                if (m_bufferAddr == uint.MaxValue)
                {
                    return false;
                }
                
                return GetAudioSourcePlayingState(m_bufferAddr) != 0;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Audio.AudioMixer" /> the AudioSource is attached to
        /// </summary>
        public AudioMixer AudioMixer
        {
            get
            {
                return m_audioMixer;
            }
            set
            {
                m_audioMixer = value;

                if (m_bufferAddr != uint.MaxValue)
                {
                    AudioSourceBuffer buffer = GetAudioSourceBuffer(m_bufferAddr);

                    if (m_audioMixer != null)
                    {
                        buffer.AudioMixerAddr = m_audioMixer.InternalAddr;
                    }
                    else
                    {
                        buffer.AudioMixerAddr = uint.MaxValue;
                    }

                    SetAudioSourceBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The AudioClip to play
        /// </summary>
        public AudioClip AudioClip
        {
            get
            {
                if (m_bufferAddr == uint.MaxValue)
                {
                    return null;
                }

                AudioSourceBuffer buffer = GetAudioSourceBuffer(m_bufferAddr);

                if (buffer.AudioClipAddr == uint.MaxValue)
                {
                    return null;
                }

                return AudioClip.GetAudioClip(buffer.AudioClipAddr);
            }
            set
            {
                if (value != null)
                {
                    if (m_bufferAddr == uint.MaxValue)
                    {
                        m_bufferAddr = GenerateAudioSource(Transform.InternalAddr, value.InternalAddr);

                        Loop = m_loop;
                        AudioMixer = m_audioMixer;
                        Is3DAudio = m_3d;

                        return;
                    }
                }
                else
                {
                    if (m_bufferAddr != uint.MaxValue)
                    {
                        DestroyAudioSource(m_bufferAddr);

                        m_bufferAddr = uint.MaxValue;
                    }

                    return;
                }

                AudioSourceBuffer buffer = GetAudioSourceBuffer(m_bufferAddr);

                buffer.AudioClipAddr = value.InternalAddr;

                SetAudioSourceBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Whether the AudioSource should loop
        /// </summary>
        public bool Loop
        {
            get
            {
                return m_loop;
            }
            set
            {
                m_loop = value;

                if (m_bufferAddr != uint.MaxValue)
                {
                    AudioSourceBuffer buffer = GetAudioSourceBuffer(m_bufferAddr);

                    if (m_loop)
                    {
                        buffer.Flags |= (byte)(0b1 << (int)AudioSourceBuffer.LoopBitOffset);
                    }
                    else
                    {
                        buffer.Flags &= (byte)~(0b1 << (int)AudioSourceBuffer.LoopBitOffset);
                    }

                    SetAudioSourceBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Whether the AudioSource is spatial
        /// </summary>
        public bool Is3DAudio
        {
            get
            {
                return m_3d;
            }
            set
            {
                m_3d = value;

                if (m_bufferAddr != uint.MaxValue)
                {
                    AudioSourceBuffer buffer = GetAudioSourceBuffer(m_bufferAddr);

                    if (m_3d)
                    {
                        buffer.Flags |= (byte)(0b1 << (int)AudioSourceBuffer.SpatialBitOffset);
                    }
                    else
                    {
                        buffer.Flags &= (byte)~(0b1 << (int)AudioSourceBuffer.SpatialBitOffset);
                    }

                    SetAudioSourceBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Called when the AudioSource is created
        /// </summary>
        public override void Init()
        {
            AudioSourceDef def = AudioSourceDef;

            if (def != null)
            {
                m_loop = def.Loop;
                m_3d = def.Is3D;

                if (def.AudioClipPath != null)
                {
                    AudioClip clip = AssetLibrary.LoadAudioClip(def.AudioClipPath);

                    if (clip != null)
                    {
                        GenerateAudioSource(Transform.InternalAddr, clip.InternalAddr);

                        AudioSourceBuffer buffer = GetAudioSourceBuffer(m_bufferAddr);
                        
                        unchecked
                        {
                            if (m_loop)
                            {
                                buffer.Flags |= (uint)(0b1 << (int)AudioSourceBuffer.LoopBitOffset);
                            }

                            if (m_3d)
                            {
                                buffer.Flags |= (uint)(0b1 << (int)AudioSourceBuffer.SpatialBitOffset);
                            }

                            if (def.PlayOnCreation)
                            {
                                buffer.Flags |= (uint)(0b1 << (int)AudioSourceBuffer.PlayBitOffset);
                            }
                        }

                        SetAudioSourceBuffer(m_bufferAddr, buffer);
                    }
                }
            }
        }

        /// <summary>
        /// Plays the AudioSource
        /// </summary>
        public void Play()
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                PlayAudioSource(m_bufferAddr);
            }
        }

        /// <summary>
        /// Disposes the AudioSource
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the AudioSource is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposed">Whether is is being called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (!m_disposed)
            {
                if (a_disposing)
                {
                    if (m_bufferAddr != uint.MaxValue)
                    {
                        DestroyAudioSource(m_bufferAddr);

                        m_bufferAddr = uint.MaxValue;
                    }
                }
                else
                {
                    Logger.IcarianWarning("AudioSource failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.IcarianError("AudioSource already Disposed");
            }
        }
        ~AudioSource()
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