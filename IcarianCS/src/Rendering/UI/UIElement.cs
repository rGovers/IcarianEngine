using System.Collections.Concurrent;

namespace IcarianEngine.Rendering.UI
{
    public abstract class UIElement
    {
        static ConcurrentDictionary<uint, UIElement> s_elementLookup = new ConcurrentDictionary<uint, UIElement>();

        protected void AddLookup(uint a_addr, UIElement a_element)
        {
            s_elementLookup.TryAdd(a_addr, a_element);
        }
        protected void RemoveLookup(uint a_addr)
        {
            s_elementLookup.TryRemove(a_addr, out UIElement _);
        }

        internal static UIElement GetUIElement(uint a_addr)
        {
            if (s_elementLookup.ContainsKey(a_addr))
            {
                return s_elementLookup[a_addr];
            }

            return null;
        }

        internal uint BufferAddr
        {
            get;
            set;
        }

        public void AddChild(UIElement a_child)
        {
            
        }
        public void RemoveChild(UIElement a_child)
        {

        }
    }
}