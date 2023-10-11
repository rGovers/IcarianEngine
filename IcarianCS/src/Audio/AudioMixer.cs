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