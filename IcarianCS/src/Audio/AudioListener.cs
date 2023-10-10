using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Audio
{
    public class AudioListener : Component, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateAudioListener(uint a_transformAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyAudioListener(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether or not the AudioListener has been disposed.
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// Called when the AudioListener is created.
        /// </summary>
        public override void Init()
        {
            m_bufferAddr = GenerateAudioListener(Transform.InternalAddr);
        }

        /// <summary>
        /// Destroys the AudioListener.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Called when the AudioListener is destroyed.
        /// </summary>
        /// <param name="a_disposing">Whether or not the AudioListener is being disposed.</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    DestroyAudioListener(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("AudioListener failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("AudioListener already Disposed");
            }
        }

        ~AudioListener()
        {
            Dispose(false);
        }
    }
}