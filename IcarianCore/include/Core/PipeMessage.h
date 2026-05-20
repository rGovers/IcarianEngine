// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

namespace IcarianCore
{
    enum e_PipeMessageType : uint32_t
    {
        PipeMessageType_Null = 0,
        PipeMessageType_Close,
        PipeMessageType_Resize,
        PipeMessageType_CursorPos,
        PipeMessageType_SetCursorState,
        PipeMessageType_MouseState,
        PipeMessageType_KeyboardState,
        PipeMessageType_FrameData,
        PipeMessageType_UpdateData,
        PipeMessageType_ProfileScope,
        PipeMessageType_MemoryFrame,
        PipeMessageType_TotalMemoryUsage,
        PipeMessageType_UnlockFrame,
        PipeMessageType_PushFrame,
        PipeMessageType_PushDMASwapFDBuffer,
        PipeMessageType_FlushDMASwapFDBuffer,
        PipeMessageType_PushDMASwapHandleBuffer,
        PipeMessageType_FlushDMASwapHandleBuffer,
        PipeMessageType_DMASwap,
        PipeMessageType_Message,
        PipeMessageType_CaptureFrame,
        PipeMessageType_RuntimeMessage,
        PipeMessageType_End
    };

    struct PipeMessage
    {
        e_PipeMessageType Type;
        uint32_t Length;
        uint8_t* Data;

        static constexpr uint32_t Size = sizeof(Type) + sizeof(Length);
    };
}

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