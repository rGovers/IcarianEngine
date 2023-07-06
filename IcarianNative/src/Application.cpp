#include "Application.h"

#include "AppWindow/GLFWAppWindow.h"
#include "AppWindow/HeadlessAppWindow.h"
#include "Config.h"
#include "Flare/IcarianAssert.h"
#include "InputManager.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Physics/PhysicsEngine.h"
#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/UI/UIControl.h"
#include "Runtime/RuntimeManager.h"
#include "Scribe.h"
#include "Trace.h"
#include "ThreadPool.h"

static Application* Instance = nullptr;

// Windows fixes
#undef min

struct Monitor
{
    uint32_t Index;
    MonoString* Name;
    uint32_t Width;
    uint32_t Height;
    void* Handle;
};

#define APPLICATION_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) m_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define APPLICATION_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine, Application, GetWidth, { return Instance->GetWidth(); }) \
    F(uint32_t, IcarianEngine, Application, GetHeight, { return Instance->GetHeight(); }) \
    F(void, IcarianEngine, Application, Resize, { Instance->Resize(a_width, a_height); }, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine, Application, GetHeadlessState, { return (uint32_t)Instance->IsHeadless(); }) \
    F(uint32_t, IcarianEngine, Application, GetEditorState, { return 0; }) \
    \
    F(void, IcarianEngine, Application, Close, { Instance->Close(); }) 

APPLICATION_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

FLARE_MONO_EXPORT(MonoArray*, RUNTIME_FUNCTION_NAME(Application, GetMonitors))
{
    MonoArray* arr = nullptr;

    int monitorCount;
    const AppMonitor* appMonitors = Instance->GetMonitors(&monitorCount);
    if (monitorCount > 0 && appMonitors != nullptr)
    {
        const RuntimeManager* runtime = Instance->GetRuntime();
        MonoDomain* domain = runtime->GetDomain();

        MonoClass* monitorClass = runtime->GetClass("IcarianEngine", "Monitor");

        arr = mono_array_new(domain, monitorClass, (uintptr_t)monitorCount);
        for (int i = 0; i < monitorCount; ++i)
        {
            Monitor monitor;
            monitor.Index = i;
            monitor.Name = mono_string_new(domain, appMonitors[i].Name.c_str());
            monitor.Width = appMonitors[i].Width;
            monitor.Height = appMonitors[i].Height;
            monitor.Handle = appMonitors[i].Handle;

            mono_array_set(arr, Monitor, i, monitor);
        }
    }

    if (appMonitors != nullptr)
    {
        delete[] appMonitors;
    }

    return arr;
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Application, SetFullscreenState), Monitor a_monitor, uint32_t a_state, uint32_t a_width, uint32_t a_height)
{
    AppMonitor appMonitor;
    appMonitor.Width = a_monitor.Width;
    appMonitor.Height = a_monitor.Height;
    appMonitor.Handle = a_monitor.Handle;

    Instance->SetFullscreen(appMonitor, (bool)a_state, a_width, a_height);
}

// Compiler why 
static void PlzNoReorder(RuntimeManager* a_runtime)
{
    delete a_runtime;
}

static void AppAssertCallback(const std::string& a_string)
{
    Logger::Error(a_string);
}

Application::Application(Config* a_config)
{
    Instance = this;

    m_close = false;

    TRACE("Starting Application");
    m_config = a_config;

    AssertCallbackFunc = (AssertCallback)AppAssertCallback;

    if (a_config->IsHeadless())
    {
        m_appWindow = new HeadlessAppWindow(this);
    }
    else
    {
        m_appWindow = new GLFWAppWindow(this, a_config);
    }

    m_runtime = new RuntimeManager();
        
    ThreadPool::Init(m_runtime);

    Logger::InitRuntime(m_runtime);
    
    Profiler::Init(m_runtime);
    Scribe::Init(m_runtime);

    UIControl::Init(m_runtime);

    m_inputManager = new InputManager(m_runtime);

    m_objectManager = new ObjectManager(m_runtime);

    m_physicsEngine = new PhysicsEngine(m_runtime, m_objectManager);
    m_renderEngine = new RenderEngine(m_runtime, m_objectManager, m_appWindow, m_config);

    APPLICATION_BINDING_FUNCTION_TABLE(APPLICATION_RUNTIME_ATTACH);

    BIND_FUNCTION(m_runtime, IcarianEngine, Application, GetMonitors);
    BIND_FUNCTION(m_runtime, IcarianEngine, Application, SetFullscreenState);
}
Application::~Application()
{
    // TODO: Proper fix for this problem
    // Seems that semaphores in mono cause a crash with stl conditional variables so we need to stop the thread pool first
    ThreadPool::Stop();

    TRACE("Disposing App");

    // This may seem odd but it seem that with the more recent changes GCC has decided to reorder the calls for some reason
    // Had to move the delete to a seperate computation unit to prevent reordering
    // Gonna guess a side effect of having execution outside of the scope of what GCC can predict at compile time
    // Do not know why C++ does not have a standard way to disable reordering
    // TLDR: Do not inline otherwise crash
    PlzNoReorder(m_runtime);

    UIControl::Destroy();

    delete m_physicsEngine;
    delete m_renderEngine;
    delete m_objectManager;
    delete m_inputManager;
    delete m_config;

    Profiler::Destroy();
    Scribe::Destroy();

    ThreadPool::Destroy();

    TRACE("Final Disposal");
    delete m_appWindow;
}

void Application::Run(int32_t a_argc, char* a_argv[])
{
    // TODO: Figure out how the thread get locked to refresh rate after running for a while
    // unlocks again after interacting with editor window
    // the weird part is that occurs across all threads so suspect os/kernel level throttling
    // If that is the case should not be an issue but need to confirm not a bug in the engine
    m_runtime->Exec(a_argc, a_argv);

    m_renderEngine->Start();

    while (!m_appWindow->ShouldClose() && !m_close)
    {
        Profiler::Start("Update Thread");

        {
            PROFILESTACK("Update");
            {
                PROFILESTACK("Window Update");

                m_inputManager->Update();
                m_appWindow->Update();
            }

            // Naive approach but helps fix weirdness from large time scales
            const double delta = glm::min(0.1, m_appWindow->GetDelta());

            m_runtime->Update(delta, m_appWindow->GetTime());

            {
                PROFILESTACK("Physics");
                
                // Considering down the line using a fixed time step instead of a dynamic for physics simulation
                m_physicsEngine->Update(delta);
            }
        }

        Profiler::Stop();
    }

    m_renderEngine->Stop();
}