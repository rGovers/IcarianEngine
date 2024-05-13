using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class Texture : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFromFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyTexture(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether the texture has been <see cref="Disposed" />
        /// <summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        internal uint BufferAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        Texture(uint a_addr)
        {
            m_bufferAddr = a_addr;
        }
        
        /// <summary>
        /// Loads a texture from file relative to a mod directory
        /// </summary>
        /// <param name="a_path">The path to the texture</param>
        /// <returns>The texture. Null on failure</returns>
        /// Supported formats:
        ///     .png,
        ///     .ktx2
        /// @see IcarianEngine.AssetLibrary.LoadTexture
        public static Texture LoadTexture(string a_path)
        {
            uint addr = GenerateFromFile(a_path);
            if (addr != uint.MaxValue)
            {
                return new Texture(addr);
            }

            Logger.IcarianError($"Texture failed to load: {a_path}");

            return null;
        }

        /// <summary>
        /// Disposes of the texture
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the texture is being disposed
        /// </summary>
        /// <param name="a_disposing">Whether the texture is being Disposed or Finalized</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyTexture(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Texture Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Texture Dispose");
            }
        }
        ~Texture()
        {
            Dispose(false);
        }
    }
}