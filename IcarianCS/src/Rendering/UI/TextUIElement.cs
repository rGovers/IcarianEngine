using System.Runtime.CompilerServices;

#include "EngineTextUIElementInterop.h"
#include "InteropBinding.h"

ENGINE_TEXTUIELEMENT_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Rendering.UI
{
    public class TextUIElement : UIElement
    {
        /// <summary>
        /// The text of the TextUIElement
        /// </summary>
        public string Text
        {
            get
            {
                return TextUIElementInterop.GetText(BufferAddr);
            }
            set
            {
                TextUIElementInterop.SetText(BufferAddr, value);
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.UI.Font" /> of the TextUIElement
        /// </summary>
        public Font Font
        {
            get
            {
                return Font.GetFont(TextUIElementInterop.GetFont(BufferAddr));
            }
            set
            {
                if (value != null)
                {
                    TextUIElementInterop.SetFont(BufferAddr, value.BufferAddr);
                }
                else
                {
                    TextUIElementInterop.SetFont(BufferAddr, uint.MaxValue);
                }
            }
        }

        /// <summary>
        /// The font size of the TextUIElement
        /// </summary>
        public float FontSize
        {
            get
            {
                return TextUIElementInterop.GetFontSize(BufferAddr);
            }
            set
            {
                TextUIElementInterop.SetFontSize(BufferAddr, value);
            }
        }

        public TextUIElement() : base(TextUIElementInterop.CreateTextElement())
        {
            
        }
    }
}