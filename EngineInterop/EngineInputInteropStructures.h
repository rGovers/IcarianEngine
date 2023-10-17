#pragma once

#include "InteropTypes.h"

IOP_CSPUBLIC enum IOP_ENUM_NAME(GamePadAxis) : IOP_UINT16
{
    IOP_ENUM_VALUE(GamePadAxis, LeftStick) = 0,
    IOP_ENUM_VALUE(GamePadAxis, RightStick) = 1,
    IOP_ENUM_VALUE(GamePadAxis, LeftTrigger) = 2,
    IOP_ENUM_VALUE(GamePadAxis, RightTrigger) = 3,
    IOP_ENUM_VALUE(GamePadAxis, DPad) = 4,
    IOP_ENUM_VALUE(GamePadAxis, Last)
} IOP_STRUCTURE_END

IOP_CSPUBLIC enum IOP_ENUM_NAME(GamePadButton) : IOP_UINT16
{
    IOP_ENUM_VALUE(GamePadButton, A) = 0,
    IOP_ENUM_VALUE(GamePadButton, B) = 1,
    IOP_ENUM_VALUE(GamePadButton, X) = 2,
    IOP_ENUM_VALUE(GamePadButton, Y) = 3,
    IOP_ENUM_VALUE(GamePadButton, LeftBumper) = 4,
    IOP_ENUM_VALUE(GamePadButton, RightBumper) = 5,
    IOP_ENUM_VALUE(GamePadButton, Back) = 6,
    IOP_ENUM_VALUE(GamePadButton, Start) = 7,
    IOP_ENUM_VALUE(GamePadButton, Guide) = 8,
    IOP_ENUM_VALUE(GamePadButton, LeftStick) = 9,
    IOP_ENUM_VALUE(GamePadButton, RightStick) = 10,
    IOP_ENUM_VALUE(GamePadButton, DPadUp) = 11,
    IOP_ENUM_VALUE(GamePadButton, DPadRight) = 12,
    IOP_ENUM_VALUE(GamePadButton, DPadDown) = 13,
    IOP_ENUM_VALUE(GamePadButton, DPadLeft) = 14,
    IOP_ENUM_VALUE(GamePadButton, Last)
} IOP_STRUCTURE_END

IOP_CSPUBLIC enum IOP_ENUM_NAME(GamePadSlot) : IOP_UINT16
{
    IOP_ENUM_VALUE(GamePadSlot, One) = 1,
    IOP_ENUM_VALUE(GamePadSlot, Two) = 2,
    IOP_ENUM_VALUE(GamePadSlot, Three) = 4,
    IOP_ENUM_VALUE(GamePadSlot, Four) = 8,
    IOP_ENUM_VALUE(GamePadSlot, Five) = 16,
    IOP_ENUM_VALUE(GamePadSlot, Six) = 32,
    IOP_ENUM_VALUE(GamePadSlot, Seven) = 64,
    IOP_ENUM_VALUE(GamePadSlot, Eight) = 128,
    IOP_ENUM_VALUE(GamePadSlot, All) = IOP_ENUM_VALUE(GamePadSlot, One) | IOP_ENUM_VALUE(GamePadSlot, Two) | IOP_ENUM_VALUE(GamePadSlot, Three) | IOP_ENUM_VALUE(GamePadSlot, Four) | IOP_ENUM_VALUE(GamePadSlot, Five) | IOP_ENUM_VALUE(GamePadSlot, Six) | IOP_ENUM_VALUE(GamePadSlot, Seven) | IOP_ENUM_VALUE(GamePadSlot, Eight)
} IOP_STRUCTURE_END