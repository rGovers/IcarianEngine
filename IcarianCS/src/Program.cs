// Icarian Engine - C# Game Engine
// 
// License at end of file.

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
            Logger.IcarianMessage("Started");
            
            ThreadPool.Init();
            JobScheduler.Init();

            Application.WorkingDirectory = string.Empty;

            foreach (string arg in a_args)
            {
                if (arg.StartsWith(WorkingDirArg))
                {
                    Application.WorkingDirectory = arg.Substring(WorkingDirArg.Length + 1);
                }
            }

            ShaderImports.Init();

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
            JobScheduler.Destroy();
        }

        static void Update(double a_delta, double a_time)
        {
            Time.DDeltaTime = a_delta;
            Time.DTimePassed = a_time;

            ModControl.Update();

            GameObject.UpdateObjects();
            GameObject.UpdateScripts();

            JobScheduler.Update();
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
        // It is weird that using the .NET compiler on Windows causes issues not gonna question it but
        static void Main(string[] a_args)
        {
            
        }
    }
    
    /// @endcond
}

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.