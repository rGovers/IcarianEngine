#pragma once

#include <cstdint>

namespace FlareBase
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
        PipeMessageType_UnlockFrame,
        PipeMessageType_PushFrame,
        PipeMessageType_Message,
        PipeMessageType_End
    };

    struct PipeMessage
    {
        e_PipeMessageType Type;
        uint32_t Length;
        char* Data;

        static constexpr uint32_t Size = sizeof(Type) + sizeof(Length);

        constexpr PipeMessage(e_PipeMessageType a_type = PipeMessageType_Null, uint32_t a_dataLength = 0, char* a_data = nullptr) :
            Type(a_type),
            Length(a_dataLength),
            Data(a_data)
        {

        }
    };
}
