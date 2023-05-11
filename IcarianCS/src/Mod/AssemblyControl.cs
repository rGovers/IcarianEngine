namespace IcarianEngine.Mod
{
    public abstract class AssemblyControl
    {
        public FlareAssembly MainAssembly;

        public abstract void Init();
        public abstract void Update();
        public abstract void Close();
    }
}