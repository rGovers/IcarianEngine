// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "AppWindow/HeadlessAppWindow.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include "Application.h"
#include "Config.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "Core/IcarianLambda.h"
#include "Core/IPCPipe.h"
#include "Core/SocketPipe.h"
#include "Core/TotalMemoryUsageFrame.h"
#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/DataTypes/Allocators/OSAllocator.h"
#include "Core/DataTypes/Allocators/RingAllocator.h"
#include "Core/DataTypes/Allocators/UberAllocator.h"
#include "Core/DataTypes/COWString.h"
#include "IcarianError.h"
#include "InputManager.h"
#include "IO.h"
#include "Profiler.h"
#include "Rendering/LibRenderDoc.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/UI/UIControl.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "ThreadPool.h"
#include "Trace.h"

void HeadlessAppWindow::MessageCallback
(
    const IcarianCore::COWU8String& a_message,
    IcarianCore::e_LoggerMessageType a_type,
    uint32_t a_stackTraceCount,
    const char* const* a_stackTrace
)
{
    uint32_t stackTraceSize = 0;
    uint32_t* sizes = ILAMBDA(
    {
        const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

        ILRETURN m_msgAllocator->TAllocate<uint32_t>(a_stackTraceCount);
    });
    for (uint32_t i = 0; i < a_stackTraceCount; ++i)
    {
        const char* slider = a_stackTrace[i];
        while (*slider != 0)
        {
            ++slider;
        }

        const uint32_t size = (uint32_t)(slider - a_stackTrace[i]);
        sizes[i] = size;
        stackTraceSize += size;
    }

    // Adding in space for null terminators
    // We use null terminators as seperators
    stackTraceSize += a_stackTraceCount;

    constexpr uint32_t HeaderSize = sizeof(IcarianCore::LoggerHeader);
    const uint32_t strSize = a_message.Length();

    const uint32_t stackTraceOffset = HeaderSize + strSize;
    const uint32_t size = stackTraceOffset + stackTraceSize + 1;

    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_Message,
        .Length = size,
        .Data = ILAMBDA(
        {
            uint8_t* dat;
           {
               const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

               dat = m_msgAllocator->ZTAllocate<uint8_t>(size);
           }

            IcarianCore::LoggerHeader header;
            header.Version = 0;
            header.Type = a_type;
            header.MessageOffset = HeaderSize;
            header.MessageSize = strSize;
            header.StackTraceOffset = stackTraceOffset;
            header.StackTraceSize = stackTraceSize;

            memcpy(dat, &header, sizeof(header));
            memcpy(dat + header.MessageOffset, a_message.CStr(), strSize);

            uint32_t offset = 0;
            for (uint32_t i = 0; i < a_stackTraceCount; ++i)
            {
                const uint32_t size = sizes[i];

                memcpy(dat + header.StackTraceOffset + offset, a_stackTrace[i], size);

                offset += size + 1;
            }

            ILRETURN dat;
        })
    };
    m_queuedMessages.Push(msg);
}

struct ProfileFrameData
{
    uint32_t NameID;
    float Time;
};

struct ProfileScopeHeader
{
    uint32_t Version;
    uint32_t NameID;
    uint32_t FrameCount;
};

class HeadlessCPUProfilerCallbackItem : public Profiler::CPUCallbackItem
{
private:
    HeadlessAppWindow* m_window;

protected:

public:
    HeadlessCPUProfilerCallbackItem(HeadlessAppWindow* a_window)
    {
        m_window = a_window;
    }

    virtual void Execute(const ProfilerCPUData& a_data)
    {
        m_window->ProfilerCPUCallback(a_data);
    }
};
void HeadlessAppWindow::ProfilerCPUCallback(const ProfilerCPUData& a_profilerData)
{
    const uint16_t count = (uint16_t)a_profilerData.Frames.Size();
    const uint32_t length = sizeof(ProfileScopeHeader) + (uint32_t)count * sizeof(ProfileFrameData);

    uint8_t* dat = ILAMBDA(
    {
        const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

        ILRETURN m_msgAllocator->ZTAllocate<uint8_t>(length);
    });

    const IcarianCore::COWU8String scopeStr = IcarianCore::COWU8String(a_profilerData.Name, IcarianCore::MallocAllocator::Instance);

    uint32_t scopeID;
    {
        const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_profileLock);

        if (!m_profilerScopes.GetIfExists(scopeStr, &scopeID))
        {
            scopeID = m_profileIndex++;

            m_profilerScopes.Push(scopeStr, scopeID);

            const uint32_t strLen = scopeStr.Length();
            const uint32_t length = sizeof(uint32_t) + strLen + 1;

            const IcarianCore::PipeMessage msg =
            {
                .Type = IcarianCore::PipeMessageType_ProfileNewScope,
                .Length = length,
                .Data = ILAMBDA(
                {
                    uint8_t* val;
                    {
                        const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

                        val = m_msgAllocator->ZTAllocate<uint8_t>(length);
                    }

                    memcpy(val, &scopeID, sizeof(uint32_t));

                    const char* cStr = scopeStr.CStr();
                    memcpy(val + sizeof(uint32_t), cStr, strLen);

                    ILRETURN val;
                }),
            };

            m_queuedMessages.Push(msg);
        }
    }

    const ProfileScopeHeader scope =
    {
        .Version = 0,
        .NameID = scopeID,
        .FrameCount = count,
    };

    memcpy(dat, &scope, sizeof(ProfileScopeHeader));

    IcarianCore::Array<uint32_t> lastParent = IcarianCore::Array<uint32_t>(IcarianCore::MallocAllocator::Instance);
    lastParent.Push(uint32_t(-1));

    for (uint32_t i = 0; i < count; ++i)
    {
        const ProfileFrame& f = a_profilerData.Frames[i];

        const IcarianCore::COWU8String frameStr = "[" + scopeStr + "]" + f.Name
            + IcarianCore::COWU8String::FromValue(f.Stack, 10, IcarianCore::MallocAllocator::Instance);

        uint32_t nameID;

        {
            const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_profileLock);

            if (!m_profilerFrames.GetIfExists(frameStr, &nameID))
            {
                nameID = m_frameIndex++;

                m_profilerFrames.Push(frameStr, nameID);

                const uint32_t strLen = f.Name.Length();
                const uint32_t length = sizeof(uint32_t) * 3 + strLen + 1;

                const uint32_t parent = lastParent[f.Stack];

                const IcarianCore::PipeMessage msg =
                {
                    .Type = IcarianCore::PipeMessageType_ProfileNewFrame,
                    .Length = length,
                    .Data = ILAMBDA(
                    {
                        uint8_t* val;
                        {
                            const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

                            val = m_msgAllocator->ZTAllocate<uint8_t>(length);
                        }

                        uint32_t offset = 0;

                        memcpy(val + offset, &scopeID, sizeof(uint32_t));
                        offset += sizeof(uint32_t);

                        memcpy(val + offset, &nameID, sizeof(uint32_t));
                        offset += sizeof(uint32_t);

                        memcpy(val + offset, &parent, sizeof(uint32_t));
                        offset += sizeof(uint32_t);

                        const char* cStr = f.Name.CStr();
                        memcpy(val + offset, cStr, strLen);

                        ILRETURN val;
                    }),
                };

                m_queuedMessages.Push(msg);
            }
        }

        IVERIFY(nameID != uint32_t(-1));

        const ProfileFrameData frame =
        {
            .NameID = nameID,
            .Time = (float)f.Duration,
        };

        const uint32_t offset = sizeof(ProfileScopeHeader) + i * sizeof(ProfileFrameData);
        memcpy(dat + offset, &frame, sizeof(ProfileFrameData));

        const uint32_t nextStack = f.Stack + 1;
        if (nextStack >= lastParent.Size())
        {
            lastParent.Resize(nextStack << 1);
        }

        lastParent[nextStack] = nameID;
    }

    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_ProfileScope,
        .Length = length,
        .Data = dat,
    };

    m_queuedMessages.Push(msg);
}

class HeadlessGPUProfilerCallbackItem : public Profiler::GPUCallbackItem
{
private:
    HeadlessAppWindow* m_window;

protected:

public:
    HeadlessGPUProfilerCallbackItem(HeadlessAppWindow* a_window)
    {
        m_window = a_window;
    }

    virtual void Execute(const ProfilerGPUFrameData* a_data, uint32_t a_count)
    {
        m_window->ProfilerGPUCallback(a_data, a_count);
    }
};
void HeadlessAppWindow::ProfilerGPUCallback(const ProfilerGPUFrameData* a_data, uint32_t a_count)
{
    for (uint32_t i = 0; i < a_count; ++i)
    {
        const uint32_t itemCount = a_data[i].Items.Size();
        const uint32_t length = sizeof(ProfileScopeHeader) + itemCount * sizeof(ProfileFrameData);
        uint8_t* dat = ILAMBDA(
        {
            const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

            ILRETURN m_msgAllocator->ZTAllocate<uint8_t>(length);
        });

        uint32_t passNameID = -1;

        const IcarianCore::COWU8String passStr = IcarianCore::COWU8String(a_data[i].Name, IcarianCore::MallocAllocator::Instance);

        {
            const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_gpuProfileLock);

            if (!m_gpuProfilePasses.GetIfExists(a_data[i].Name, &passNameID))
            {
                passNameID = m_gpuProfilePassIndex++;

                m_gpuProfilePasses.Push(passStr, passNameID);

                const uint32_t strLen = passStr.Length();
                const uint32_t length = sizeof(uint32_t) + strLen + 1;

                const IcarianCore::PipeMessage msg =
                {
                    .Type = IcarianCore::PipeMessageType_ProfileNewGPUPass,
                    .Length = length,
                    .Data = ILAMBDA(
                    {
                        uint8_t* val;
                        {
                            const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

                            val = m_msgAllocator->ZTAllocate<uint8_t>(length);
                        }

                        memcpy(val, &passNameID, sizeof(uint32_t));

                        const char* cStr = passStr.CStr();
                        memcpy(val + sizeof(uint32_t), cStr, strLen);

                        ILRETURN val;
                    }),
                };

                m_queuedMessages.Push(msg);
            }
        }

        IVERIFY(passNameID != uint32_t(-1));

        const ProfileScopeHeader header =
        {
            .Version = 0,
            .NameID = passNameID,
            .FrameCount = itemCount,
        };

        memcpy(dat, &header, sizeof(ProfileScopeHeader));

        for (uint32_t j = 0; j < itemCount; ++j)
        {
            const ProfilerGPUFrameItem& it = a_data[i].Items[j];

            const IcarianCore::COWU8String itemStr = "[" + passStr + "]" + it.Name;

            uint32_t itemID = -1;
            {
                const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_gpuProfileLock);

                if (!m_gpuProfileItems.GetIfExists(itemStr, &itemID))
                {
                    itemID = m_gpuProfileItemIndex++;

                    m_gpuProfileItems.Push(itemStr, itemID);

                    const uint32_t strLen = it.Name.Length();
                    const uint32_t length = sizeof(uint32_t) * 2 + strLen + 1;

                    const IcarianCore::PipeMessage msg =
                    {
                        .Type = IcarianCore::PipeMessageType_ProfileNewGPUItem,
                        .Length = length,
                        .Data = ILAMBDA(
                        {
                            uint8_t* val;
                            {
                                const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

                                val = m_msgAllocator->ZTAllocate<uint8_t>(length);
                            }

                            uint32_t offset = 0;

                            memcpy(val + offset, &passNameID, sizeof(uint32_t));
                            offset += sizeof(uint32_t);

                            memcpy(val + offset, &itemID, sizeof(uint32_t));
                            offset += sizeof(uint32_t);

                            const char* cStr = it.Name.CStr();
                            memcpy(val + offset, cStr, strLen);

                            ILRETURN val;
                        }),
                    };

                    m_queuedMessages.Push(msg);
                }
            }

            IVERIFY(itemID != uint32_t(-1));

            const ProfileFrameData item =
            {
                .NameID = itemID,
                .Time = (float)it.Duration,
            };

            const uint32_t offset = sizeof(ProfileScopeHeader) + j * sizeof(ProfileFrameData);
            memcpy(dat + offset, &item, sizeof(ProfileFrameData));
        }

        const IcarianCore::PipeMessage msg =
        {
            .Type = IcarianCore::PipeMessageType_ProfileGPUPass,
            .Length = length,
            .Data = dat,
        };

        m_queuedMessages.Push(msg);
    }
}

class HeadlessGPUETEProfilerCallbackItem : public Profiler::GPUETECallbackItem
{
private:
    HeadlessAppWindow* m_window;

protected:

public:
    HeadlessGPUETEProfilerCallbackItem(HeadlessAppWindow* a_window)
    {
        m_window = a_window;
    }

    virtual void Execute(float a_time)
    {
        m_window->ProfilerGPUETECallback(a_time);
    }
};
void HeadlessAppWindow::ProfilerGPUETECallback(float a_time)
{
    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_ProfileGPUEndToEndTime,
        .Length = sizeof(float),
        .Data = ILAMBDA(
        {
            float* val;

            {
                const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);
                val = m_msgAllocator->TAllocate<float>();
            }

            *val = a_time;

            ILRETURN (uint8_t*)val;
        }),
    };

    m_queuedMessages.Push(msg);
}

HeadlessAppWindow::HeadlessAppWindow(Application* a_app, Config* a_config) : AppWindow(a_app),
    m_gpuProfilePasses(IcarianCore::MallocAllocator::Instance),
    m_gpuProfileItems(IcarianCore::MallocAllocator::Instance),
    m_profilerScopes(IcarianCore::MallocAllocator::Instance),
    m_profilerFrames(IcarianCore::MallocAllocator::Instance)
{
    TRACE("Creating Headless Window");
    m_pipe = nullptr;
    m_flags = 0;

    m_gpuProfilePassIndex = 0;
    m_gpuProfileItemIndex = 0;
    m_profileIndex = 0;
    m_frameIndex = 0;

    m_msgAllocator = IcarianCore::MallocAllocator::Instance->Create<IcarianCore::RingAllocator>(16 << 20, IcarianCore::UberAllocator::Instance);

#ifndef ICARIANNATIVE_ENABLE_DMA
    m_frameData = nullptr;
    m_unlockWindow = false;
    m_windowFrame = 0;
    m_gpuFrame = 0;
#endif

    m_delta = 0.0;
    m_time = 0.0;

    TRACE("Initializing IPC");

    if (a_config->IsRemote())
    {
        const uint16_t port = a_config->GetRemotePort();

        m_pipe = IcarianCore::SocketPipe::Create(port);

        ISETBIT(m_flags, RemoteBit);
    }
    else
    {
#ifdef WIN32
        m_pipe = IcarianCore::SocketPipe::Create(9001);
#else
        const uint32_t ipcPipeID = a_config->GetIPCID();
        if (ipcPipeID == uint32_t(-1))
        {
            IERROR("Invalid IPC pipe ID");
        }

        const IcarianCore::COWU8String tempDir = IO::GetTemporaryDirectory(IcarianCore::MallocAllocator::Instance);
        const IcarianCore::COWU8String pipeName = PipeName + IcarianCore::COWU8String::FromValue(ipcPipeID, 10, IcarianCore::MallocAllocator::Instance);

        const IcarianCore::COWU8String path = IO::CombinePath(tempDir, pipeName, IcarianCore::MallocAllocator::Instance);

        m_pipe = IcarianCore::IPCPipe::Connect(path.CStr());
#endif
    }

    if (m_pipe == nullptr || !m_pipe->IsAlive())
    {
        IERROR("Failed to initialize IPC pipe");
    }

    TRACE("Initializing Runtime IPC");

    // This has code smell I may want to look into this more
    // Need to think of how to do this better
    m_runtimeMessageReceive = RuntimeManager::GetFunction("IcarianEngine", "PipeMessage", ":ReceiveMessage(string,byte[])");
    IVERIFY(m_runtimeMessageReceive != nullptr);

    m_width = 1280;
    m_height = 720;

    Logger::CallbackFunc = IcarianCore::MallocAllocator::Instance->Create<Logger::Callback>(std::bind
    (
        &HeadlessAppWindow::MessageCallback,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4
    ));

    Profiler::CPUCallback = IcarianCore::MallocAllocator::Instance->Create<HeadlessCPUProfilerCallbackItem>(this);
    Profiler::GPUCallback = IcarianCore::MallocAllocator::Instance->Create<HeadlessGPUProfilerCallbackItem>(this);
    Profiler::GPUETECallback = IcarianCore::MallocAllocator::Instance->Create<HeadlessGPUETEProfilerCallbackItem>(this);

    m_prevTime = std::chrono::high_resolution_clock::now();

    TRACE("Headless Window Initialized");
}
HeadlessAppWindow::~HeadlessAppWindow()
{
    TRACE("Cleaning up Headless Window");

    PushMessageQueue();

    if (m_pipe != nullptr)
    {
        const IcarianCore::PipeMessage msg =
        {
            .Type = IcarianCore::PipeMessageType_Close
        };
        m_pipe->Send(msg);

        delete m_pipe;
        m_pipe = nullptr;
    }

#ifndef ICARIANNATIVE_ENABLE_DMA
    if (m_frameData != nullptr)
    {
        MallocAllocator::Instance->Free(m_frameData);
        m_frameData = nullptr;
    }
#endif

    IcarianCore::MallocAllocator::Instance->Destroy(m_runtimeMessageReceive);

    IcarianCore::MallocAllocator::Instance->Destroy(Logger::CallbackFunc);
    Logger::CallbackFunc = nullptr;

    IcarianCore::MallocAllocator::Instance->Destroy(Profiler::CPUCallback);
    Profiler::CPUCallback = nullptr;
    IcarianCore::MallocAllocator::Instance->Destroy(Profiler::GPUCallback);
    Profiler::GPUCallback = nullptr;
    IcarianCore::MallocAllocator::Instance->Destroy(Profiler::GPUETECallback);
    Profiler::GPUETECallback = nullptr;

    IcarianCore::MallocAllocator::Instance->Destroy(m_msgAllocator);
}

void HeadlessAppWindow::PushMessageQueue()
{
    IERRBLOCK;

    IERRDEFER(
    {
        delete m_pipe;
        m_pipe = nullptr;

        ISETBIT(m_flags, CloseBit);

        IERROR("Failed to send messages");
    });

    if (!m_queuedMessages.Empty())
    {
        TLockArray<IcarianCore::PipeMessage> a = m_queuedMessages.ToLockArray();

        for (const IcarianCore::PipeMessage& msg : a)
        {
            const IcarianCore::CommunicationPipe::e_SendError err = m_pipe->Send(msg);
            if (err != IcarianCore::CommunicationPipe::SendError_Success)
            {
                printf("Send Message error: %s \n", IcarianCore::CommunicationPipe::SendErrorString(err));

                ITRIGGERERR;
            }
        }

        m_queuedMessages.UClear();
    }
}

bool HeadlessAppWindow::ShouldClose() const
{
    return IISBITSET(m_flags, CloseBit) || m_pipe == nullptr || !m_pipe->IsAlive();
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
    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_SetCursorState,
        .Length = sizeof(e_CursorState),
        .Data = ILAMBDA(
        {
            e_CursorState* dat = m_msgAllocator->TAllocate<e_CursorState>();
            *dat = a_state;

            ILRETURN (uint8_t*)dat;
        })
    };
    m_queuedMessages.Push(msg);
}

class RuntimeMessageThreadJob : public ThreadJob
{
private:
    std::string      m_string;
    uint8_t*         m_data;
    uintptr_t        m_length;

    RuntimeFunction* m_function;

protected:

public:
    RuntimeMessageThreadJob(RuntimeFunction* a_function, const std::string_view& a_str, const uint8_t* a_data, uintptr_t a_length) : ThreadJob(JobPriority_EngineHigh)
    {
        m_function = a_function;

        m_string = std::string(a_str);

        m_length = a_length;
        m_data = nullptr;
        if (m_length > 0)
        {
            m_data = IcarianCore::MallocAllocator::Instance->TAllocate<uint8_t>(m_length);

            memcpy(m_data, a_data, m_length);
        }
    }
    virtual ~RuntimeMessageThreadJob()
    {
        if (m_data != nullptr)
        {
            IcarianCore::MallocAllocator::Instance->Free(m_data);
            m_data = nullptr;
        }
    }

    virtual void Execute()
    {
        MonoDomain* domain = RuntimeManager::GetDomain();

        MonoArray* runtimeData = ILAMBDA(
        {
            if (m_length <= 0)
            {
                ILRETURN (MonoArray*)NULL;
            }

            MonoClass* byteClass = mono_get_byte_class();
            MonoArray* val = mono_array_new(domain, byteClass, m_length);

            for (uintptr_t i = 0; i < m_length; ++i)
            {
                mono_array_set(val, mono_byte, i, m_data[i]);
            }

            ILRETURN val;
        });

        MonoString* runtimeStr = mono_string_new(domain, m_string.c_str());

        void* args[] =
        {
            runtimeStr,
            runtimeData
        };

        m_function->Exec(args);
    }
};

bool HeadlessAppWindow::PollMessage()
{
    std::queue<IcarianCore::PipeMessage> messages;
    if (!m_pipe->Receive(&messages))
    {
        return false;
    }

    while (!messages.empty())
    {
        const IcarianCore::PipeMessage msg = messages.front();
        messages.pop();
        IDEFER(
        {
            if (msg.Data != nullptr)
            {
                delete[] msg.Data;
            }
        });

        switch (msg.Type)
        {
        case IcarianCore::PipeMessageType_Close:
        {
            ISETBIT(m_flags, CloseBit);

            break;
        }
        case IcarianCore::PipeMessageType_UnlockFrame:
        {
            m_unlockWindow = true;

            break;
        }
        case IcarianCore::PipeMessageType_Resize:
        {
            const glm::ivec2 size = *(glm::ivec2*)msg.Data;

            m_width = (uint32_t)size.x;
            m_height = (uint32_t)size.y;

            const std::lock_guard g = std::lock_guard(m_fLock);

            if (m_frameData != nullptr)
            {
                IcarianCore::MallocAllocator::Instance->Free(m_frameData);
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

            const glm::vec2 cursorPos = inputManager->GetCursorPos();
            const glm::vec2 winSizeVec = glm::vec2((float)m_width, (float)m_height);

            bool leftDown = IISBITSET(mouseState, MouseButton_Left);
            if (leftDown)
            {
                if (UIControl::SubmitClick(cursorPos, winSizeVec))
                {
                    leftDown = false;
                }
            }
            else
            {
                UIControl::SubmitRelease(cursorPos, winSizeVec);
            }

            const bool middleDown = IISBITSET(mouseState, MouseButton_Middle);
            const bool rightDown = IISBITSET(mouseState, MouseButton_Right);

            inputManager->SetMouseButton(MouseButton_Left, leftDown);
            inputManager->SetMouseButton(MouseButton_Middle, middleDown);
            inputManager->SetMouseButton(MouseButton_Right, rightDown);

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
        case IcarianCore::PipeMessageType_CaptureFrame:
        {
            LibRenderDoc::CaptureFrame();

            break;
        }
        case IcarianCore::PipeMessageType_RuntimeMessage:
        {
            const char* str = (char*)msg.Data;
            const char* strEnd = str;
            while (*strEnd != 0)
            {
                ++strEnd;
            }
            ++strEnd;

            IVERIFY(strEnd - str <= msg.Length);

            const uintptr_t len = msg.Length - (strEnd - str);
            const uint8_t* data = (uint8_t*)strEnd;

            ThreadPool::PushJob(IcarianCore::MallocAllocator::Instance->Create<RuntimeMessageThreadJob>(m_runtimeMessageReceive, str, data, len));

            break;
        }
        case IcarianCore::PipeMessageType_Null:
        {
            IWARN("Null Message");

            return false;
        }
        default:
        {
            IERROR
            (
                "IcarianEngine: Invalid Pipe Message: " +
                IcarianCore::COWU8String::FromValue(msg.Type, 10, IcarianCore::MallocAllocator::Instance) + " " +
                IcarianCore::COWU8String::FromValue(msg.Length, 10, IcarianCore::MallocAllocator::Instance)
            );

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

        PollMessage();
    }

    {
        PROFILESTACK("Timing");

        std::chrono::high_resolution_clock::time_point time;
        while (true)
        {
            time = std::chrono::high_resolution_clock::now();
            m_delta = std::chrono::duration<double>(time - m_prevTime).count();

            if (m_delta >= 0.001f)
            {
                break;
            }

            std::this_thread::yield();
        }

        m_time += m_delta;
        m_prevTime = time;

        const glm::dvec2 tVec = glm::vec2(m_delta, m_time);

        const IcarianCore::PipeMessage msg =
        {
            .Type = IcarianCore::PipeMessageType_UpdateData,
            .Length = sizeof(glm::dvec2),
            .Data = (uint8_t*)&tVec
        };

        const IcarianCore::CommunicationPipe::e_SendError err = m_pipe->Send(msg);
        if (err != IcarianCore::CommunicationPipe::SendError_Success)
        {
            printf("Send Message error: %s \n", IcarianCore::CommunicationPipe::SendErrorString(err));

            ISETBIT(m_flags, CloseBit);

            delete m_pipe;
            m_pipe = nullptr;

            IERROR("Failed to send update data");

            return;
        }
    }

#ifndef ICARIANNATIVE_ENABLE_DMA
    {
        PROFILESTACK("Frame Data");

        // When on the same system we want to throttle to the Window but over the network that is too much latency to sync so just shotgun it out
        if (m_frameData != nullptr && m_windowFrame != m_gpuFrame && (m_unlockWindow || IISBITSET(m_flags, RemoteBit)))
        {
            IDEFER(m_windowFrame = m_gpuFrame);

            m_unlockWindow = false;

            const std::lock_guard g = std::lock_guard(m_fLock);

            const IcarianCore::PipeMessage msg =
            {
                .Type = IcarianCore::PipeMessageType_PushFrame,
                .Length = m_width * m_height * 4,
                .Data = (uint8_t*)m_frameData
            };

            const IcarianCore::CommunicationPipe::e_SendError err = m_pipe->Send(msg);
            if (err != IcarianCore::CommunicationPipe::SendError_Success)
            {
                printf("Send Message error: %s \n", IcarianCore::CommunicationPipe::SendErrorString(err));

                ISETBIT(m_flags, CloseBit);

                delete m_pipe;
                m_pipe = nullptr;

                IERROR("Failed to send frame data");

                return;
            }
        }
    }
#endif

    {
        PROFILESTACK("Profiler Data");

        {
            const uint64_t osUsage = IcarianCore::OSAllocator::TrackerInstance->GetTrueMemoryUsage();
            const uint64_t mallocUsage = IcarianCore::MallocAllocator::TrackerInstance->GetTrueMemoryUsage();

            const IcarianCore::TotalMemoryUsageFrame frame =
            {
                .OSUsage = osUsage,
                .MallocUsage = mallocUsage,
            };

            const IcarianCore::PipeMessage msg =
            {
                .Type = IcarianCore::PipeMessageType_TotalMemoryUsage,
                .Length = sizeof(frame),
                .Data = (uint8_t*)&frame,
            };

            const IcarianCore::CommunicationPipe::e_SendError err = m_pipe->Send(msg);
            if (err != IcarianCore::CommunicationPipe::SendError_Success)
            {
                printf("Send Message error: %s \n", IcarianCore::CommunicationPipe::SendErrorString(err));

                ISETBIT(m_flags, CloseBit);

                delete m_pipe;
                m_pipe = nullptr;

                IERROR("Failed to send total memory usage");

                return;
            }
        }

        {
            const IcarianCore::MemoryUsageFrame frame = Profiler::GetMemoryFrames();

            const IcarianCore::PipeMessage msg =
            {
                .Type = IcarianCore::PipeMessageType_MemoryFrame,
                .Length = sizeof(frame),
                .Data = (uint8_t*)&frame,
            };

            const IcarianCore::CommunicationPipe::e_SendError err = m_pipe->Send(msg);
            if (err != IcarianCore::CommunicationPipe::SendError_Success)
            {
                printf("Send Message error: %s \n", IcarianCore::CommunicationPipe::SendErrorString(err));

                ISETBIT(m_flags, CloseBit);

                delete m_pipe;
                m_pipe = nullptr;

                IERROR("Failed to send memory frame");

                return;
            }
        }
    }

    {
        PROFILESTACK("Messages");

        PushMessageQueue();
    }
}

void HeadlessAppWindow::PushFrameInfo(double a_delta, double a_time)
{
    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_FrameData,
        .Length = sizeof(glm::dvec2),
        .Data = ILAMBDA(
        {
            glm::dvec2* dat = m_msgAllocator->TAllocate<glm::dvec2>();
            dat->x = a_delta;
            dat->y = a_time;

            ILRETURN (uint8_t*)dat;
        })
    };
    m_queuedMessages.Push(msg);
}

#ifdef ICARIANNATIVE_ENABLE_DMA
void HeadlessAppWindow::PushSwapBufferFD(const IcarianCore::DMASwapBufferFD& a_swapBuffer)
{
    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(m_msgAllocatorLock);

    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_PushDMASwapFDBuffer,
        .Length = sizeof(IcarianCore::DMASwapBufferFD),
        .Data = ILAMBDA(
        {
            IcarianCore::DMASwapBufferFD* dat = m_msgAllocator->TAllocate<IcarianCore::DMASwapBufferFD>();
            *dat = a_swapBuffer;

            ILRETURN (uint8_t*)dat;
        })
    };

    m_queuedMessages.Push(msg);
}
void HeadlessAppWindow::FlushSwapBufferFD()
{
    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_FlushDMASwapFDBuffer
    };

    m_queuedMessages.Push(msg);
}

#ifdef WIN32
void HeadlessAppWindow::PushSwapBufferHandle(const IcarianCore::DMASwapBufferHandle& a_swapBuffer)
{
    const ThreadGuard g = ThreadGuard(m_msgAllocatorLock);

    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_PushDMASwapHandleBuffer,
        .Length = sizeof(DMASwapBufferHandle),
        .Data = ILAMBDA(
        {
            IcarianCore::DMASwapBufferHandle* dat = m_msgAllocator->TAllocate<IcarianCore::DMASwapBufferHandle>();
            *dat = a_swapBuffer;

            ILRETURN (uint8_t*)dat;
        })
    };

    m_queuedMessages.Push(msg);
}
void HeadlessAppWindow::FlushSwapBufferHandle()
{
    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_FlushDMASwapHandleBuffer
    };

    m_queuedMessages.Push(msg);
}
#endif
#endif

void HeadlessAppWindow::PushFrameData(uint32_t a_width, uint32_t a_height, const uint8_t* a_buffer)
{
    PROFILESTACK("Frame Data");

    const std::lock_guard g = std::lock_guard(m_fLock);
    if (m_width == a_width && m_height == a_height)
    {
        IDEFER(++m_gpuFrame);

        const uint32_t size = m_width * m_height * 4;

        if (m_frameData == nullptr)
        {
            m_frameData = IcarianCore::MallocAllocator::Instance->TAllocate<uint8_t>(size);
        }

        memcpy(m_frameData, a_buffer, size);
    }
}

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#ifdef ICARIANNATIVE_ENABLE_DMA
constexpr const char* HeadlessExtensions[] =
{
    VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
};
#endif

IcarianCore::Array<const char*> HeadlessAppWindow::GetRequiredVulkanExtenions() const
{
#ifdef ICARIANNATIVE_ENABLE_DMA
    return IcarianCore::Array<const char*>(HeadlessExtensions, sizeof(HeadlessExtensions) / sizeof(*HeadlessExtensions), IcarianCore::MallocAllocator::Instance);
#endif

    return IcarianCore::Array<const char*>(IcarianCore::MallocAllocator::Instance);
}

#endif

// MIT License
//
// Copyright (c) 2026 River Govers
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
