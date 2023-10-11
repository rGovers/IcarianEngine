using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Audio
{   
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    struct AudioSourceBuffer
    {
        public const int PlayBitOffset = 0;
        public const int LoopBitOffset = 1;

        public uint TransformAddr;
        public uint AudioClipAddr;
        public uint AudioMixerAddr;
        public ulong SampleOffset;
        public uint Flags;
        uint Source;
        uint BufferA;
        uint BufferB;
        uint BufferC;
    }

    public class AudioSource : Component, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateAudioSource(uint a_transformAddr, uint a_audioClipAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyAudioSource(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PlayAudioSource(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetLoopAudioSource(uint a_addr, uint a_loop);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetAudioSourcePlayingState(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static AudioSourceBuffer GetAudioSourceBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetAudioSourceBuffer(uint a_addr, AudioSourceBuffer a_buffer);

        bool       m_disposed = false;
        bool       m_loop = false;
        AudioMixer m_audioMixer = null;

        uint       m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether or not the AudioSource has been disposed.
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The Def used to create the AudioSource.
        /// </summary>
        public AudioSourceDef AudioSourceDef
        {
            get
            {
                return Def as AudioSourceDef;
            }
        }

        /// <summary>
        /// Whether or not the AudioSource is playing.
        /// </summary>
        public bool IsPlaying
        {
            get
            {
                return GetAudioSourcePlayingState(m_bufferAddr) != 0;
            }
        }

        /// <summary>
        /// The AudioMixer the AudioSource is attached to.
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
        /// The AudioClip to play.
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
        /// Whether or not the AudioClip should loop.
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
                    if (m_loop)
                    {
                        SetLoopAudioSource(m_bufferAddr, 1);
                    }
                    else
                    {
                        SetLoopAudioSource(m_bufferAddr, 0);
                    }
                }
            }
        }

        /// <summary>
        /// Called when the AudioSource is created.
        /// </summary>
        public override void Init()
        {
            AudioSourceDef def = AudioSourceDef;

            if (def != null)
            {
                m_loop = def.Loop;

                if (def.AudioClipPath != null)
                {
                    AudioClip clip = AssetLibrary.LoadAudioClip(def.AudioClipPath);

                    if (clip != null)
                    {
                        GenerateAudioSource(Transform.InternalAddr, clip.InternalAddr);

                        AudioSourceBuffer buffer = GetAudioSourceBuffer(m_bufferAddr);
                        
                        if (m_loop)
                        {
                            buffer.Flags |= (uint)(0b1 << AudioSourceBuffer.LoopBitOffset);
                        }

                        if (def.PlayOnCreation)
                        {
                            buffer.Flags |= (uint)(0b1 << AudioSourceBuffer.PlayBitOffset);
                        }

                        SetAudioSourceBuffer(m_bufferAddr, buffer);
                    }
                }
            }
        }

        /// <summary>
        /// Plays the AudioSource.
        /// </summary>
        public void Play()
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                PlayAudioSource(m_bufferAddr);
            }
        }

        /// <summary>
        /// Destroys the AudioSource.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Called when the AudioSource is being disposed.
        /// </summary>
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