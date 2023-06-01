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

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static string GetText(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetText(uint a_addr, string a_text);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetFont(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetFont(uint a_addr, uint a_fontAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float GetFontSize(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetFontSize(uint a_addr, float a_size);
        
        public bool IsDisposed
        {
            get
            {
                return BufferAddr == uint.MaxValue;
            }
        }

        public string Text
        {
            get
            {
                return GetText(BufferAddr);
            }
            set
            {
                SetText(BufferAddr, value);
            }
        }

        public Font Font
        {
            get
            {
                return Font.GetFont(GetFont(BufferAddr));
            }
            set
            {
                if (value != null)
                {
                    SetFont(BufferAddr, value.BufferAddr);
                }
                else
                {
                    SetFont(BufferAddr, uint.MaxValue);
                }
            }
        }

        public float FontSize
        {
            get
            {
                return GetFontSize(BufferAddr);
            }
            set
            {
                SetFontSize(BufferAddr, value);
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