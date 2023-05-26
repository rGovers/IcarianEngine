using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering.UI
{
    public class TextUIElement : UIElement, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateTextElement();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyTextElement(uint a_addr);

        public bool IsDisposed
        {
            get
            {
                return BufferAddr == uint.MaxValue;
            }
        }

        public TextUIElement()
        {
            BufferAddr = CreateTextElement();

            AddLookup(BufferAddr, this);
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(BufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyTextElement(BufferAddr);

                    RemoveLookup(BufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("TextUIElement Failed to Dispose");
                }

                BufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple TextUIElement Dispose");
            }
        }

        ~TextUIElement()
        {
            Dispose(false);
        }
    }
}