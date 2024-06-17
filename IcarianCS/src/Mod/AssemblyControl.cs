namespace IcarianEngine.Mod
{
    public abstract class AssemblyControl
    {
        /// <summary>
        /// The assembly that this control is for. Contains the assembly's info.
        /// </summary>
        public IcarianAssembly MainAssembly;

        /// <summary>
        /// Called on application initialization.
        /// </summary>
        public abstract void Init();
        /// <summary>
        /// Called on update.
        /// </summary>
        public abstract void Update();
        /// <summary>
        /// Called on late update.
        /// </summary>
        public virtual void LateUpdate() { }
        /// <summary>
        /// Called on fixed update.
        /// </summary>
        public abstract void FixedUpdate();
        /// <summary>
        /// Called on frame update.
        /// </summary>
        public virtual void FrameUpdate() { }
        /// <summary>
        /// Called on application shutdown.
        /// </summary>
        public abstract void Close();
    }
}