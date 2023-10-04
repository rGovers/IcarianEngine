namespace IcarianEngine
{
    public class Scriptable : Component
    {
        /// <summary>
        /// Called on Update
        /// </summary>
        public virtual void Update() { }

        /// <summary>
        /// Called on FixedUpdate
        /// </summary>
        public virtual void FixedUpdate() { }
    }
}