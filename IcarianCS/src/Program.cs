using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.PostEffects;

namespace IcarianEngine
{
    /// @cond INTERNAL

    class Program
    {
        const string WorkingDirArg = "--wDir";

        static void Init(string[] a_args)
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

            RenderPipeline.SetPipeline(new DefaultRenderPipeline(new PostEffect[] 
            { 
                new AtmospherePostEffect(),
                new EmissionPostEffect(),
                new ToneMapPostEffect()
            }));

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

        static void LateUpdate()
        {
            ModControl.LateUpdate();
        }

        static void FixedUpdate(double a_delta, double a_time)
        {
            Time.DFixedDeltaTime = a_delta;
            Time.DFixedTimePassed = a_time;

            ModControl.FixedUpdate();

            GameObject.FixedUpdateScripts();
        }

        static void FrameUpdate(double a_delta, double a_time)
        {
            Time.DFrameDeltaTime = a_delta;
            Time.DFrameTimePassed = a_time;

            ModControl.FrameUpdate();
        }

        // Need to not crash on Windows when compiled with a .NET compiler despite being a library and not used
        // It is weird that using .NET compiler on Windows causes issues not gonna question it but
        static void Main(string[] a_args)
        {
            
        }
    }
    
    /// @endcond
}
