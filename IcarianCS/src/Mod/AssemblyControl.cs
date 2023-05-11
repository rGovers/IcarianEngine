namespace IcarianEngine.Mod
{
    public abstract class AssemblyControl
    {
        public IcarianAssembly MainAssembly;

        public abstract void Init();
        public abstract void Update();
        public abstract void Close();
    }
}