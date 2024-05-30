using System.Runtime.CompilerServices;

#include "EngineImageUIElementInterop.h"
#include "InteropBinding.h"

ENGINE_IMAGEUIELEMENT_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Rendering.UI
{
    public class ImageUIElement : UIElement
    {
        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.TextureSampler" /> of the ImageUIElement
        /// </summary>
        public TextureSampler Sampler
        {
            get
            {
                return TextureSampler.GetSampler(ImageUIElementInterop.GetSampler(BufferAddr));
            }
            set
            {
                if (value != null)
                {
                    ImageUIElementInterop.SetSampler(BufferAddr, value.BufferAddr);
                }
                else
                {
                    ImageUIElementInterop.SetSampler(BufferAddr, uint.MaxValue);
                }
            }
        }

        public ImageUIElement() : base(ImageUIElementInterop.CreateImageElement())
        {

        }
    }
}