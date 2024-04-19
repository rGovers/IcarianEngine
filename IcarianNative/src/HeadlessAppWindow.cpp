#include "AppWindow/HeadlessAppWindow.h"
#include "IcarianError.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <filesystem>
#include <string>

#include "Application.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "InputManager.h"
#include "Profiler.h"
#include "Rendering/UI/UIControl.h"
#include "Trace.h"

#undef min

static std::string GetAddr(const std::string_view& a_addr)
{
    return (std::filesystem::temp_directory_path() / a_addr).string();
}

void HeadlessAppWindow::MessageCallback(const std::string_view& a_message, e_LoggerMessageType a_type)
{
    const uint32_t strSize = (uint32_t)a_message.size();
    constexpr uint32_t TypeSize = sizeof(e_LoggerMessageType);
    const uint32_t size = strSize + TypeSize;

    IcarianCore::PipeMessage msg;
    msg.Type = IcarianCore::PipeMessageType_Message;
    msg.Length = size;
    msg.Data = new char[size];
    memcpy(msg.Data, &a_type, TypeSize);
    memcpy(msg.Data + TypeSize, a_message.data(), strSize);

    m_queuedMessages.Push(msg);
}
void HeadlessAppWindow::ProfilerCallback(const Profiler::PData& a_profilerData)
{
    constexpr uint32_t ScopeSize = sizeof(ProfileScope);

    IcarianCore::PipeMessage msg;
    msg.Type = IcarianCore::PipeMessageType_ProfileScope;
    msg.Length = ScopeSize;
    msg.Data = new char[ScopeSize];

    ProfileScope* scope = (ProfileScope*)msg.Data;
    
    const int nameSize = glm::min((int)a_profilerData.Name.size(), NameMax - 1);
    for (int i = 0; i < nameSize; ++i)
    {
        scope->Name[i] = a_profilerData.Name[i];
    }
    scope->Name[nameSize] = 0;

    scope->FrameCount = (uint16_t)glm::min((int)a_profilerData.Frames.size(), FrameMax);
    for (uint16_t i = 0; i < scope->FrameCount; ++i)
    {
        const ProfileFrame& pFrame = a_profilerData.Frames[i];
        ProfileTFrame& frame = scope->Frames[i];

        const int frameNameSize = glm::min((int)pFrame.Name.size(), NameMax - 1);
        for (int j = 0; j < frameNameSize; ++j)
        {
            frame.Name[j] = pFrame.Name[j];
        }
        frame.Name[frameNameSize] = 0;
        frame.Stack = pFrame.Stack;
        frame.Time = (float)pFrame.Duration;
    }

    m_queuedMessages.Push(msg);
}

HeadlessAppWindow::HeadlessAppWindow(Application* a_app) : AppWindow(a_app)
{
    TRACE("Creating headless window");

    m_close = false;

    m_frameData = nullptr;
    m_unlockWindow = false;
    
    m_delta = 0.0;
    m_time = 0.0;

    TRACE("Initialising IPC");

    const std::string addrStr = GetAddr(PipeName);

#if WIN32
    WSADATA wsaData = { };
    ICARIAN_ASSERT_MSG_R(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0, "Failed to start WSA");
#endif

    m_pipe = IcarianCore::IPCPipe::Connect(addrStr);
    ICARIAN_ASSERT_MSG_R(m_pipe != nullptr, "Failed to connect to pipe");

    m_width = 1280;
    m_height = 720;

    Logger::CallbackFunc = new Logger::Callback(std::bind(&HeadlessAppWindow::MessageCallback, this, std::placeholders::_1, std::placeholders::_2));
    Profiler::CallbackFunc = new Profiler::Callback(std::bind(&HeadlessAppWindow::ProfilerCallback, this, std::placeholders::_1));

    m_prevTime = std::chrono::high_resolution_clock::now();

    TRACE("Headless Window Initialised");
}
HeadlessAppWindow::~HeadlessAppWindow()
{
    TRACE("Cleaning up Headless Window");

    PushMessageQueue();

    if (m_pipe != nullptr)
    {
        m_pipe->Send({ IcarianCore::PipeMessageType_Close });

        delete m_pipe;
        m_pipe = nullptr;
    }

    if (m_frameData != nullptr)
    {
        delete[] m_frameData;
        m_frameData = nullptr;
    }

    delete Logger::CallbackFunc;
    Logger::CallbackFunc = nullptr;
    delete Profiler::CallbackFunc;
    Profiler::CallbackFunc = nullptr;
}

void HeadlessAppWindow::PushMessageQueue()
{
    if (!m_queuedMessages.Empty())
    {
        TLockArray<IcarianCore::PipeMessage> a = m_queuedMessages.ToLockArray();

        const uint32_t size = a.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            const IcarianCore::PipeMessage& msg = a[i];
            IDEFER(
            if (msg.Data != nullptr)
            {
                delete[] msg.Data;
            });

            if (!m_pipe->Send(msg))
            {
                m_close = true;

                delete m_pipe;
                m_pipe = nullptr;

                printf("Failed to send message \n");

                assert(0);

                return;
            }
        }

        m_queuedMessages.UClear();
    }
}

bool HeadlessAppWindow::ShouldClose() const
{
    return m_close || m_pipe == nullptr;
}

double HeadlessAppWindow::GetDelta() const
{
    return m_delta;
}
double HeadlessAppWindow::GetTime() const
{
    return m_time;
}

void HeadlessAppWindow::SetCursorState(e_CursorState a_state)
{
    constexpr uint32_t Size = sizeof(e_CursorState);

    IcarianCore::PipeMessage msg;
    msg.Type = IcarianCore::PipeMessageType_SetCursorState;
    msg.Length = Size;
    msg.Data = new char[Size];
    *(e_CursorState*)msg.Data = a_state;

    m_queuedMessages.Push(msg);
}

bool HeadlessAppWindow::PollMessage()
{
    std::queue<IcarianCore::PipeMessage> messages;
    if (!m_pipe->Receive(&messages))
    {
        printf("Failed to receive message \n");

        m_close = true;

        delete m_pipe;
        m_pipe = nullptr;

        assert(0);

        return false;
    }

    while (!messages.empty())
    {
        const IcarianCore::PipeMessage msg = messages.front();
        messages.pop();
        IDEFER(
        if (msg.Data != nullptr)
        {
            delete[] msg.Data;
        });

        switch (msg.Type)
        {
        case IcarianCore::PipeMessageType_Close:
        {
            m_close = true;

            break;
        }
        case IcarianCore::PipeMessageType_UnlockFrame:
        {
            m_unlockWindow = true;

            break;
        }
        case IcarianCore::PipeMessageType_Resize:
        {
            const std::lock_guard g = std::lock_guard(m_fLock);
            const glm::ivec2 size = *(glm::ivec2*)msg.Data;

            m_width = (uint32_t)size.x;
            m_height = (uint32_t)size.y;

            if (m_frameData != nullptr)
            {
                delete[] m_frameData;
                m_frameData = nullptr;
            }

            break;
        }
        case IcarianCore::PipeMessageType_CursorPos:
        {
            const Application* app = GetApplication();

            InputManager* inputManager = app->GetInputManager();

            const glm::vec2& pos = *(glm::vec2*)msg.Data;

            inputManager->SetCursorPos(pos);

            UIControl::UpdateCursor(pos, glm::vec2((float)m_width, (float)m_height));

            break;
        }
        case IcarianCore::PipeMessageType_MouseState:
        {
            const Application* app = GetApplication();

            InputManager* inputManager = app->GetInputManager();

            const unsigned char mouseState = *(unsigned char*)msg.Data;

            bool leftDown = mouseState & 0b1 << MouseButton_Left;
            if (leftDown)
            {
                if (UIControl::SubmitClick(inputManager->GetCursorPos(), glm::vec2((float)m_width, (float)m_height)))
                {
                    leftDown = false;
                }
            }
            else 
            {
                UIControl::SubmitRelease(inputManager->GetCursorPos(), glm::vec2((float)m_width, (float)m_height));
            }

            inputManager->SetMouseButton(MouseButton_Left, leftDown);
            inputManager->SetMouseButton(MouseButton_Middle, mouseState & 0b1 << MouseButton_Middle);
            inputManager->SetMouseButton(MouseButton_Right, mouseState & 0b1 << MouseButton_Right);

            break;
        }
        case IcarianCore::PipeMessageType_KeyboardState:
        {
            const Application* app = GetApplication();

            InputManager* inputManager = app->GetInputManager();

            const IcarianCore::KeyboardState state = IcarianCore::KeyboardState::FromData((unsigned char*)msg.Data);

            for (unsigned int i = 0; i < KeyCode_Last; ++i)
            {
                const e_KeyCode keyCode = (e_KeyCode)i;

                inputManager->SetKeyboardKey(keyCode, state.IsKeyDown(keyCode));
            }

            break;
        }
        case IcarianCore::PipeMessageType_Null:
        {
            IWARN("Null Message");

            return false;
        }
        default:
        {
            IERROR("IcarianEngine: Invalid Pipe Message: " + std::to_string(msg.Type) + " " + std::to_string(msg.Length));

            break;
        }
        }
    }    

    return true;
}

void HeadlessAppWindow::Update()
{
    {
        PROFILESTACK("Polling");

        if (!PollMessage())
        {
            return;
        }
    }

    {
        PROFILESTACK("Timing");
        const std::chrono::time_point time = std::chrono::high_resolution_clock::now();

        m_delta = std::chrono::duration<double>(time - m_prevTime).count();
        m_time += m_delta;

        m_prevTime = time;

        const glm::dvec2 tVec = glm::vec2(m_delta, m_time);

        if (!m_pipe->Send({ IcarianCore::PipeMessageType_UpdateData, sizeof(glm::dvec2), (char*)&tVec}))
        {
            m_close = true;

            delete m_pipe;
            m_pipe = nullptr;

            printf("Failed to send update data \n");

            assert(0);

            return;
        }
    }

    {
        PROFILESTACK("Frame Data");
        if (m_frameData != nullptr && m_unlockWindow)
        {
            m_unlockWindow = false;

            const std::lock_guard g = std::lock_guard(m_fLock);

            if (!m_pipe->Send({ IcarianCore::PipeMessageType_PushFrame, m_width * m_height * 4, m_frameData }))
            {
                m_close = true;

                delete m_pipe;
                m_pipe = nullptr;

                printf("Failed to send frame data \n");

                assert(0);

                return;
            }
        }
    }

    {
        PROFILESTACK("Messages");
        PushMessageQueue();
    }
}

glm::ivec2 HeadlessAppWindow::GetSize() const
{
    return glm::ivec2((int)m_width, (int)m_height);
}

void HeadlessAppWindow::PushFrameData(uint32_t a_width, uint32_t a_height, const char* a_buffer, double a_delta, double a_time)
{
    PROFILESTACK("Frame Data");
    constexpr int Size = sizeof(glm::dvec2);

    IcarianCore::PipeMessage msg;
    msg.Type = IcarianCore::PipeMessageType_FrameData;
    msg.Length = Size;
    msg.Data = new char[Size];
    (*(glm::dvec2*)msg.Data).x = a_delta;
    (*(glm::dvec2*)msg.Data).y = a_time;
    m_queuedMessages.Push(msg);

    // TODO: Implement a better way of doing this 
    // Can end up ~32MiB which cannot keep up with a copy
    // Assuming I can do maths ~6GiB/s so yeah not upto par
    // Probably end up with a syncronised direct push at some point
    const std::lock_guard g = std::lock_guard(m_fLock);
    if (m_width == a_width && m_height == a_height)
    {
        const uint32_t size = m_width * m_height * 4;

        if (m_frameData == nullptr)
        {
            m_frameData = new char[size];
        }

        memcpy(m_frameData, a_buffer, size);
    }
}