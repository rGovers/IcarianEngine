using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using IcarianEngine.Rendering;

namespace IcarianEngine
{
    class Program
    {
        const string WorkingDirArg = "--wDir";

        static void Main(string[] a_args)
        {
            ThreadPool.Init();

            Logger.IcarianMessage("Started");

            Application.WorkingDirectory = string.Empty;

            foreach (string arg in a_args)
            {
                if (arg.StartsWith(WorkingDirArg))
                {
                    Application.WorkingDirectory = arg.Substring(WorkingDirArg.Length + 1);
                }
            }

            Time.Init();

            Material.Init();

            RenderPipeline.Init(new DefaultRenderPipeline());

            AssetLibrary.Init();
            DefLibrary.Init();

            ModControl.Init();

            DefLibrary.ResolveDefs();
            Scribe.SetLanguage("en-us");

            ModControl.InitAssemblies();

            Logger.IcarianMessage("Initialized");
        }

        static void Shutdown()
        {
            ModControl.Close();

            DefLibrary.Clear();
            AssetLibrary.ClearAssets();

            GameObject.DestroyObjects();

            RenderPipeline.Destroy();

            Material.Destroy();

            Logger.IcarianMessage("Shutdown");

            ThreadPool.Destroy();
        }

        static void Update(double a_delta, double a_time)
        {
            Time.DDeltaTime = a_delta;
            Time.DTimePassed = a_time;

            ModControl.Update();

            GameObject.UpdateObjects();
            GameObject.UpdateScripts();
        }
    }
}
