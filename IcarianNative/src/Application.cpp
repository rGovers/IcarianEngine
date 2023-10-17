#include "Application.h"

#include "AppWindow/GLFWAppWindow.h"
#include "AppWindow/HeadlessAppWindow.h"
#include "Audio/AudioEngine.h"
#include "Config.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "InputManager.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Physics/PhysicsEngine.h"
#include "Profiler.h"
#include "Rendering/AnimationController.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/UI/UIControl.h"
#include "Runtime/RuntimeManager.h"
#include "Scribe.h"
#include "Trace.h"
#include "ThreadPool.h"

#include "EngineInputInterop.h"

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

#define APPLICATION_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine, Application, GetWidth, { return Instance->GetWidth(); }) \
    F(uint32_t, IcarianEngine, Application, GetHeight, { return Instance->GetHeight(); }) \
    F(void, IcarianEngine, Application, Resize, { Instance->Resize(a_width, a_height); }, uint32_t a_width, uint32_t a_height) \
    \
    F(uint32_t, IcarianEngine, Application, GetHeadlessState, { return (uint32_t)Instance->IsHeadless(); }) \
    F(uint32_t, IcarianEngine, Application, GetEditorState, { return 0; }) \
    \
    F(void, IcarianEngine, Application, Close, { Instance->Close(); }) \

APPLICATION_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

ENGINEAPPINPUT_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(MonoArray*, Application, GetMonitors,
{
    MonoArray* arr = NULL;

    int monitorCount;
    const AppMonitor* appMonitors = Instance->GetMonitors(&monitorCount);
    IDEFER(if (appMonitors != NULL)
    {
        delete[] appMonitors;
    });
    
    if (monitorCount > 0 && appMonitors != NULL)
    {
        MonoDomain* domain = RuntimeManager::GetDomain();

        MonoClass* monitorClass = RuntimeManager::GetClass("IcarianEngine", "Monitor");

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

    return arr;
})
RUNTIME_FUNCTION(void, Application, SetFullscreenState, 
{
    AppMonitor appMonitor;
    appMonitor.Width = a_monitor.Width;
    appMonitor.Height = a_monitor.Height;
    appMonitor.Handle = a_monitor.Handle;

    Instance->SetFullscreen(appMonitor, (bool)a_state, a_width, a_height);
}, Monitor a_monitor, uint32_t a_state, uint32_t a_width, uint32_t a_height)

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

    RuntimeManager::Init();
        
    Logger::Init();

    ThreadPool::Init();
    
    Profiler::Init();
    Scribe::Init();

    UIControl::Init();

    AnimationController::Init();

    m_inputManager = new InputManager();

    ObjectManager::Init();

    m_audioEngine = new AudioEngine();
    m_physicsEngine = new PhysicsEngine(a_config);
    m_renderEngine = new RenderEngine(m_appWindow, m_config);

    APPLICATION_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);

    ENGINEAPPINPUT_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

    BIND_FUNCTION(IcarianEngine, Application, GetMonitors);
    BIND_FUNCTION(IcarianEngine, Application, SetFullscreenState);
}
Application::~Application()
{
    TRACE("Disposing App");

    // TODO: Proper fix for this problem
    // Seems that semaphores in mono cause a crash with stl conditional variables so we need to stop the thread pool first
    ThreadPool::Stop();

    RuntimeManager::Destroy();

    AnimationController::Destroy();

    UIControl::Destroy();

    delete m_audioEngine;
    delete m_physicsEngine;
    delete m_renderEngine;
    delete m_inputManager;
    delete m_config;

    ObjectManager::Destroy();

    Profiler::Destroy();
    Scribe::Destroy();

    ThreadPool::Destroy();

    TRACE("Final Disposal");
    delete m_appWindow;
}

void Application::SetCursorState(FlareBase::e_CursorState a_state)
{
    m_cursorState = a_state;

    m_appWindow->SetCursorState(m_cursorState);
}
void Application::Run(int32_t a_argc, char* a_argv[])
{
    // TODO: Figure out how the thread get locked to refresh rate after running for a while
    // unlocks again after interacting with editor window
    // the weird part is that occurs across all threads so suspect os/kernel level throttling
    // If that is the case should not be an issue but need to confirm not a bug in the engine
    RuntimeManager::Exec(a_argc, a_argv);

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

            {
                PROFILESTACK("Animators");

                {
                    PROFILESTACK("Anim Dispatch");

                    AnimationController::DispatchUpdate((float)delta);
                }

                {
                    PROFILESTACK("Anim Update");

                    AnimationController::UpdateAnimators(AnimationUpdateMode_Update, (float)delta);
                }
            }

            {
                PROFILESTACK("Audio");
                
                m_audioEngine->Update();
            }

            RuntimeManager::Update(delta, m_appWindow->GetTime());

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