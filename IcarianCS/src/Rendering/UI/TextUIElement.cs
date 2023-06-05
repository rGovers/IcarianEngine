using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering.UI
{
    public class TextUIElement : UIElement
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

        protected override void Dispose(bool a_disposing)
        {
            bool dispose = !IsDisposed;

            base.Dispose(a_disposing);

            if(dispose)
            {
                if(a_disposing)
                {
                    DestroyTextElement(BufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("TextUIElement Failed to Dispose");
                }
            }
            else
            {
                Logger.IcarianError("Multiple TextUIElement Dispose");
            }
        }
    }
}