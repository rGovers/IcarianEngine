using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

#include "EngineFontInterop.h"
#include "InteropBinding.h"

ENGINE_FONT_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Rendering.UI
{
    public class Font : IDestroy
    {
        static Dictionary<uint, Font> s_bufferLookup = new Dictionary<uint, Font>();

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether or not the Font has been Disposed/Finalised
        /// </summary>
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

        Font(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

            s_bufferLookup.Add(m_bufferAddr, this);
        }

        /// <summary>
        /// Loads a Font from file
        /// </summary>
        /// <param name="a_path">The path to load the Font from</param>
        /// <returns>The Font</returns>
        public static Font LoadFont(string a_path)
        {
            return new Font(FontInterop.GenerateFont(a_path));
        }

        internal static Font GetFont(uint a_buffer)
        {
            if (s_bufferLookup.ContainsKey(a_buffer))
            {
                return s_bufferLookup[a_buffer];
            }

            return null;
        }

        /// <summary>
        /// Disposes of the Font
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the Font is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether being called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    FontInterop.DestroyFont(m_bufferAddr);

                    s_bufferLookup.Remove(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Font Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Font Dispose");
            }
        }
        ~Font()
        {
            Dispose(false);
        }
    }
}