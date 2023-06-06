using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering.UI
{
    public class ImageUIElement : UIElement
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateImageElement();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyImageElement(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetSampler(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetSampler(uint a_addr, uint a_samplerAddr);

        public TextureSampler Sampler
        {
            get
            {
                return TextureSampler.GetSampler(GetSampler(BufferAddr));
            }
            set
            {
                if (value != null)
                {
                    SetSampler(BufferAddr, value.BufferAddr);
                }
                else
                {
                    SetSampler(BufferAddr, uint.MaxValue);
                }
            }
        }

        public ImageUIElement()
        {
            BufferAddr = CreateImageElement();

            AddLookup(BufferAddr, this);
        }

        protected override void Dispose(bool a_disposing)
        {
            bool dispose = !IsDisposed;

            base.Dispose(a_disposing);

            if (dispose)
            {
                if (a_disposing)
                {
                    DestroyImageElement(BufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Failed to dispose ImageUIElement");
                }
            }
            else
            {
                Logger.IcarianError("Multiple ImageUIElement Dispose");
            }
        }
    }
}