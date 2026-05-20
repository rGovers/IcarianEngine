// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "AppWindow/HeadlessAppWindow.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <filesystem>
#include <string>

#include "Application.h"
#include "Config.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "Core/IcarianLambda.h"
#include "Core/IPCPipe.h"
#include "Core/SocketPipe.h"
#include "Core/TotalMemoryUsageFrame.h"
#include "DataTypes/Allocators/MallocAllocator.h"
#include "DataTypes/Allocators/OSAllocator.h"
#include "DataTypes/Allocators/RingAllocator.h"
#include "DataTypes/Allocators/UberAllocator.h"
#include "IcarianError.h"
#include "InputManager.h"
#include "Profiler.h"
#include "Rendering/LibRenderDoc.h"
#include "Rendering/UI/UIControl.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "ThreadPool.h"
#include "Trace.h"

[[maybe_unused]] static std::string GetAddr(const std::string_view& a_addr)
{
    const std::filesystem::path tmpPath = std::filesystem::temp_directory_path();
    const std::filesystem::path addrPath = tmpPath / a_addr;

    return addrPath.generic_string();
}

void HeadlessAppWindow::MessageCallback
(
    const COWU8String& a_message,
    IcarianCore::e_LoggerMessageType a_type,
    uint32_t a_stackTraceCount,
    const char* const* a_stackTrace
)
{
    const ThreadGuard g = ThreadGuard(m_msgAllocatorLock);

    uint32_t stackTraceSize = 0;
    uint32_t* sizes = m_msgAllocator->TAllocate<uint32_t>(a_stackTraceCount); 
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
            uint8_t* dat = m_msgAllocator->ZTAllocate<uint8_t>(size);

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
void HeadlessAppWindow::ProfilerCallback(const Profiler::PData& a_profilerData)
{
    const ThreadGuard g = ThreadGuard(m_msgAllocatorLock);

    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_ProfileScope,
        .Length = sizeof(ProfileScope),
        .Data = ILAMBDA(
        {
            ProfileScope* dat = m_msgAllocator->TAllocate<ProfileScope>();

            const uint16_t nameSize = glm::min((uint16_t)a_profilerData.Name.Length(), (uint16_t)(NameMax - 1));
            for (int i = 0; i < nameSize; ++i)
            {
                dat->Name[i] = a_profilerData.Name[i];
            }
            dat->Name[nameSize] = 0;

            dat->FrameCount = (uint16_t)glm::min((uint16_t)a_profilerData.Frames.Size(), FrameMax);
            for (uint16_t i = 0; i < dat->FrameCount; ++i)
            {
                const ProfileFrame& pFrame = a_profilerData.Frames[i];
                ProfileTFrame& frame = dat->Frames[i];

                const uint16_t frameNameSize = glm::min((uint16_t)pFrame.Name.Length(), (uint16_t)(NameMax - 1));
                for (uint16_t j = 0; j < frameNameSize; ++j)
                {
                    frame.Name[j] = pFrame.Name[j];
                }
                frame.Name[frameNameSize] = 0;
                frame.Stack = pFrame.Stack;
                frame.Time = (float)pFrame.Duration;
            }

            ILRETURN (uint8_t*)dat;
        })
    };

    m_queuedMessages.Push(msg);
}

HeadlessAppWindow::HeadlessAppWindow(Application* a_app, Config* a_config) : AppWindow(a_app)
{
    TRACE("Creating Headless Window");

    m_pipe = nullptr;
    m_flags = 0;

    m_msgAllocator = MallocAllocator::Instance->Create<RingAllocator>(4 << 20, UberAllocator::Instance);

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

        const std::string addrStr = GetAddr(PipeName + std::to_string(ipcPipeID));

        m_pipe = IcarianCore::IPCPipe::Connect(addrStr);
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

    Logger::CallbackFunc = MallocAllocator::Instance->Create<Logger::Callback>(std::bind
    (
        &HeadlessAppWindow::MessageCallback,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4
    ));
    Profiler::CallbackFunc = MallocAllocator::Instance->Create<Profiler::Callback>(std::bind(&HeadlessAppWindow::ProfilerCallback, this, std::placeholders::_1));

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

    MallocAllocator::Instance->Destroy(m_runtimeMessageReceive);

    MallocAllocator::Instance->Destroy(Logger::CallbackFunc);
    Logger::CallbackFunc = nullptr;
    MallocAllocator::Instance->Destroy(Profiler::CallbackFunc);
    Profiler::CallbackFunc = nullptr;

    MallocAllocator::Instance->Destroy(m_msgAllocator);
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

        const uint32_t size = a.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            const IcarianCore::PipeMessage& msg = a[i];

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
    const ThreadGuard g = ThreadGuard(m_msgAllocatorLock);

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
            m_data = MallocAllocator::Instance->TAllocate<uint8_t>(m_length);

            memcpy(m_data, a_data, m_length);
        }
    }
    virtual ~RuntimeMessageThreadJob()
    {
        if (m_data != nullptr)
        {
            MallocAllocator::Instance->Free(m_data);
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
        if (msg.Data != nullptr)
        {
            delete[] msg.Data;
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
#ifdef ICARIANNATIVE_ENABLE_DMA
            // IERROR("DMA enabled UnlockFrame not available");
#else
            m_unlockWindow = true;
#endif

            break;
        }
        case IcarianCore::PipeMessageType_Resize:
        {
            const glm::ivec2 size = *(glm::ivec2*)msg.Data;

            m_width = (uint32_t)size.x;
            m_height = (uint32_t)size.y;

#ifndef ICARIANNATIVE_ENABLE_DMA
            const std::lock_guard g = std::lock_guard(m_fLock);

            if (m_frameData != nullptr)
            {
                delete[] m_frameData;
                m_frameData = nullptr;
            }
#endif

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

            bool leftDown = IISBITSET(mouseState, MouseButton_Left);
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
            inputManager->SetMouseButton(MouseButton_Middle, IISBITSET(mouseState, MouseButton_Middle));
            inputManager->SetMouseButton(MouseButton_Right, IISBITSET(mouseState, MouseButton_Right));

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

            ThreadPool::PushJob(MallocAllocator::Instance->Create<RuntimeMessageThreadJob>(m_runtimeMessageReceive, str, data, len));

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
                COWU8String::FromValue(msg.Type, 10, MallocAllocator::Instance) + " " +
                COWU8String::FromValue(msg.Length, 10, MallocAllocator::Instance)
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
        if (m_pipe->Send(msg) != IcarianCore::CommunicationPipe::SendError_Success)
        {
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
            if (m_pipe->Send(msg) != IcarianCore::CommunicationPipe::SendError_Success)
            {
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
            const uint64_t osUsage = OSAllocator::TrackerInstance->GetTrueMemoryUsage();
            const uint64_t mallocUsage = MallocAllocator::TrackerInstance->GetTrueMemoryUsage();

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
            if (m_pipe->Send(msg) != IcarianCore::CommunicationPipe::SendError_Success)
            {
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
            if (m_pipe->Send(msg) != IcarianCore::CommunicationPipe::SendError_Success)
            {
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
    const ThreadGuard g = ThreadGuard(m_msgAllocatorLock);

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
void HeadlessAppWindow::PushSwapBufferFD(const DMASwapBufferFD& a_swapBuffer)
{
    const ThreadGuard g = ThreadGuard(m_msgAllocatorLock);

    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_PushDMASwapFDBuffer,
        .Length = sizeof(DMASwapBufferFD),
        .Data = ILAMBDA(
        {
            DMASwapBufferFD* dat = m_msgAllocator->TAllocate<DMASwapBufferFD>();
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
void HeadlessAppWindow::PushSwapBufferHandle(const DMASwapBufferHandle& a_swapBuffer)
{
    const ThreadGuard g = ThreadGuard(m_msgAllocatorLock);

    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_PushDMASwapHandleBuffer,
        .Length = sizeof(DMASwapBufferHandle),
        .Data = ILAMBDA(
        {
            DMASwapBufferHandle* dat = m_msgAllocator->TAllocate<DMASwapBufferHandle>();
            *dat = a_swapBuffer;

            ILRETURN (uint8_t*)dat;
        })
    };
    m_queuedMessages.Push(msg);
}
#endif
void HeadlessAppWindow::FlushSwapBufferHandle()
{
    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_FlushDMASwapHandleBuffer
    };

    m_queuedMessages.Push(msg);
}

void HeadlessAppWindow::DMASwap()
{
    const IcarianCore::PipeMessage msg =
    {
        .Type = IcarianCore::PipeMessageType_DMASwap
    };

    m_queuedMessages.Push(msg);
}
#else
void HeadlessAppWindow::PushFrameData(uint32_t a_width, uint32_t a_height, const char* a_buffer)
{
    PROFILESTACK("Frame Data");

    const std::lock_guard g = std::lock_guard(m_fLock);
    if (m_width == a_width && m_height == a_height)
    {
        IDEFER(++m_gpuFrame);

        const uint32_t size = m_width * m_height * 4;

        if (m_frameData == nullptr)
        {
            m_frameData = MallocAllocator::Instance->TAllocate<char>(size);
        }

        memcpy(m_frameData, a_buffer, size);
    }
}
#endif

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#ifdef ICARIANNATIVE_ENABLE_DMA
constexpr const char* HeadlessExtensions[] =
{
    VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
};
#endif

Array<const char*> HeadlessAppWindow::GetRequiredVulkanExtenions() const
{
#ifdef ICARIANNATIVE_ENABLE_DMA
    return Array<const char*>(HeadlessExtensions, sizeof(HeadlessExtensions) / sizeof(*HeadlessExtensions), MallocAllocator::Instance);
#endif

    return Array<const char*>(MallocAllocator::Instance);
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
