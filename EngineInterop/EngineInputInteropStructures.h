// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

#ifdef CUBE_LANGUAGE_CSHARP
namespace IcarianEngine {
#endif

/// @file EngineInputInteropStructures.h

/// <summary>
/// GamePad Axis enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(GamePadAxis) : IOP_UINT16
{
    IOP_ENUM_VALUE(GamePadAxis, LeftStick) = 0,
    IOP_ENUM_VALUE(GamePadAxis, RightStick) = 1,
    IOP_ENUM_VALUE(GamePadAxis, LeftTrigger) = 2,
    IOP_ENUM_VALUE(GamePadAxis, RightTrigger) = 3,
    IOP_ENUM_VALUE(GamePadAxis, DPad) = 4,

#ifdef CUBE_LANGUAGE_CPP
    IOP_ENUM_VALUE(GamePadAxis, Last)
#endif
};

/// <summary>
/// GamePad Button enumeration
/// </summary>
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

#ifdef CUBE_LANGUAGE_CPP
    IOP_ENUM_VALUE(GamePadButton, Last)
#endif
};

/// <summary>
/// GamePad Slot enumeration
/// </summary>
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
};

/// <summary>
/// Mouse Button enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(MouseButton) : IOP_UINT16
{
    IOP_ENUM_VALUE(MouseButton, Left) = 0,
    IOP_ENUM_VALUE(MouseButton, Middle) = 1,
    IOP_ENUM_VALUE(MouseButton, Right) = 2,

#ifdef CUBE_LANGUAGE_CPP
    IOP_ENUM_VALUE(MouseButton, Last)
#endif
};

/// <summary>
/// Cursor State enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(CursorState) : IOP_UINT16
{
    IOP_ENUM_VALUE(CursorState, Normal) = 0,
    IOP_ENUM_VALUE(CursorState, Hidden) = 1,
    IOP_ENUM_VALUE(CursorState, Locked) = 2
};

/// <summary>
/// Key Code enumeration
/// </summary>
IOP_CSPUBLIC enum IOP_ENUM_NAME(KeyCode) : IOP_UINT16
{
    IOP_ENUM_VALUE(KeyCode, Null) = IOP_UINT16_MAX,

    IOP_ENUM_VALUE(KeyCode, Space) = 0, 
    IOP_ENUM_VALUE(KeyCode, Apostrophe) = 1, /* ' */
    IOP_ENUM_VALUE(KeyCode, Comma) = 2, /* , */
    IOP_ENUM_VALUE(KeyCode, Minus) = 3, /* - */
    IOP_ENUM_VALUE(KeyCode, Equals) = 4, /* = */
    IOP_ENUM_VALUE(KeyCode, Period) = 5, /* . */
    IOP_ENUM_VALUE(KeyCode, ForwardSlash) = 6, /* / */ 
    IOP_ENUM_VALUE(KeyCode, BackSlash) = 7, /* \ */
    IOP_ENUM_VALUE(KeyCode, LeftBracket) = 8, /* [ */
    IOP_ENUM_VALUE(KeyCode, RightBracket) = 9, /* ] */
    IOP_ENUM_VALUE(KeyCode, Semicolon) = 10, /* ; */
    IOP_ENUM_VALUE(KeyCode, Accent) = 11, /* ` */

    IOP_ENUM_VALUE(KeyCode, A) = 12,
    IOP_ENUM_VALUE(KeyCode, B) = 13,
    IOP_ENUM_VALUE(KeyCode, C) = 14,
    IOP_ENUM_VALUE(KeyCode, D) = 15,
    IOP_ENUM_VALUE(KeyCode, E) = 16,
    IOP_ENUM_VALUE(KeyCode, F) = 17,
    IOP_ENUM_VALUE(KeyCode, G) = 18,
    IOP_ENUM_VALUE(KeyCode, H) = 19,
    IOP_ENUM_VALUE(KeyCode, I) = 20,
    IOP_ENUM_VALUE(KeyCode, J) = 21,
    IOP_ENUM_VALUE(KeyCode, K) = 22,
    IOP_ENUM_VALUE(KeyCode, L) = 23,
    IOP_ENUM_VALUE(KeyCode, M) = 24,
    IOP_ENUM_VALUE(KeyCode, N) = 25,
    IOP_ENUM_VALUE(KeyCode, O) = 26,
    IOP_ENUM_VALUE(KeyCode, P) = 27,
    IOP_ENUM_VALUE(KeyCode, Q) = 28,
    IOP_ENUM_VALUE(KeyCode, R) = 29,
    IOP_ENUM_VALUE(KeyCode, S) = 30,
    IOP_ENUM_VALUE(KeyCode, T) = 31,
    IOP_ENUM_VALUE(KeyCode, U) = 32,
    IOP_ENUM_VALUE(KeyCode, V) = 33,
    IOP_ENUM_VALUE(KeyCode, W) = 34,
    IOP_ENUM_VALUE(KeyCode, X) = 35,
    IOP_ENUM_VALUE(KeyCode, Y) = 36,
    IOP_ENUM_VALUE(KeyCode, Z) = 37,

    IOP_ENUM_VALUE(KeyCode, Num0) = 38,
    IOP_ENUM_VALUE(KeyCode, Num1) = 39,
    IOP_ENUM_VALUE(KeyCode, Num2) = 40,
    IOP_ENUM_VALUE(KeyCode, Num3) = 41,
    IOP_ENUM_VALUE(KeyCode, Num4) = 42,
    IOP_ENUM_VALUE(KeyCode, Num5) = 43,
    IOP_ENUM_VALUE(KeyCode, Num6) = 44,
    IOP_ENUM_VALUE(KeyCode, Num7) = 45,
    IOP_ENUM_VALUE(KeyCode, Num8) = 46,
    IOP_ENUM_VALUE(KeyCode, Num9) = 47,

    IOP_ENUM_VALUE(KeyCode, PrintableEnd) = IOP_ENUM_VALUE(KeyCode, Num9) + 1,

    IOP_ENUM_VALUE(KeyCode, KeypadDecimal) = 48, /* . */
    IOP_ENUM_VALUE(KeyCode, KeypadDivide) = 49, /* / */
    IOP_ENUM_VALUE(KeyCode, KeypadMultiply) = 50, /* * */
    IOP_ENUM_VALUE(KeyCode, KeypadSubtract) = 51, /* - */
    IOP_ENUM_VALUE(KeyCode, KeypadAdd) = 52, /* + */
    IOP_ENUM_VALUE(KeyCode, KeypadEquals) = 53, /* = */
    IOP_ENUM_VALUE(KeyCode, KeypadEnter) = 54,

    IOP_ENUM_VALUE(KeyCode, Keypad0) = 55,
    IOP_ENUM_VALUE(KeyCode, Keypad1) = 56,
    IOP_ENUM_VALUE(KeyCode, Keypad2) = 57,
    IOP_ENUM_VALUE(KeyCode, Keypad3) = 58,
    IOP_ENUM_VALUE(KeyCode, Keypad4) = 59,
    IOP_ENUM_VALUE(KeyCode, Keypad5) = 60,
    IOP_ENUM_VALUE(KeyCode, Keypad6) = 61,
    IOP_ENUM_VALUE(KeyCode, Keypad7) = 62,
    IOP_ENUM_VALUE(KeyCode, Keypad8) = 63,
    IOP_ENUM_VALUE(KeyCode, Keypad9) = 64,

    IOP_ENUM_VALUE(KeyCode, Escape) = 65,
    IOP_ENUM_VALUE(KeyCode, Enter) = 66,
    IOP_ENUM_VALUE(KeyCode, Tab) = 67,
    IOP_ENUM_VALUE(KeyCode, Backspace) = 68,
    IOP_ENUM_VALUE(KeyCode, Insert) = 69,
    IOP_ENUM_VALUE(KeyCode, Delete) = 70,
    IOP_ENUM_VALUE(KeyCode, Home) = 71,
    IOP_ENUM_VALUE(KeyCode, End) = 72,
    IOP_ENUM_VALUE(KeyCode, PageUp) = 73,
    IOP_ENUM_VALUE(KeyCode, PageDown) = 74,
    IOP_ENUM_VALUE(KeyCode, LeftArrow) = 75,
    IOP_ENUM_VALUE(KeyCode, RightArrow) = 76,
    IOP_ENUM_VALUE(KeyCode, UpArrow) = 77,
    IOP_ENUM_VALUE(KeyCode, DownArrow) = 78,
    IOP_ENUM_VALUE(KeyCode, CapsLock) = 79,
    IOP_ENUM_VALUE(KeyCode, NumLock) = 80,
    IOP_ENUM_VALUE(KeyCode, ScrollLock) = 81,
    IOP_ENUM_VALUE(KeyCode, PrintScreen) = 82,
    IOP_ENUM_VALUE(KeyCode, PauseBreak) = 83,

    IOP_ENUM_VALUE(KeyCode, LeftShift) = 84,
    IOP_ENUM_VALUE(KeyCode, LeftCtrl) = 85,
    IOP_ENUM_VALUE(KeyCode, LeftAlt) = 86,
    IOP_ENUM_VALUE(KeyCode, LeftSuper) = 87, /* Windows Key */

    IOP_ENUM_VALUE(KeyCode, RightShift) = 88,
    IOP_ENUM_VALUE(KeyCode, RightCtrl) = 89,
    IOP_ENUM_VALUE(KeyCode, RightAlt) = 90,
    IOP_ENUM_VALUE(KeyCode, RightSuper) = 91, /* Windows Key */

    IOP_ENUM_VALUE(KeyCode, F1) = 92,
    IOP_ENUM_VALUE(KeyCode, F2) = 93,
    IOP_ENUM_VALUE(KeyCode, F3) = 94,
    IOP_ENUM_VALUE(KeyCode, F4) = 95,
    IOP_ENUM_VALUE(KeyCode, F5) = 96,
    IOP_ENUM_VALUE(KeyCode, F6) = 97,
    IOP_ENUM_VALUE(KeyCode, F7) = 98,
    IOP_ENUM_VALUE(KeyCode, F8) = 99,
    IOP_ENUM_VALUE(KeyCode, F9) = 100,
    IOP_ENUM_VALUE(KeyCode, F10) = 101,
    IOP_ENUM_VALUE(KeyCode, F11) = 102,
    IOP_ENUM_VALUE(KeyCode, F12) = 103,
    IOP_ENUM_VALUE(KeyCode, F13) = 104,
    IOP_ENUM_VALUE(KeyCode, F14) = 105,
    IOP_ENUM_VALUE(KeyCode, F15) = 106,
    IOP_ENUM_VALUE(KeyCode, F16) = 107,
    IOP_ENUM_VALUE(KeyCode, F17) = 108,
    IOP_ENUM_VALUE(KeyCode, F18) = 109,
    IOP_ENUM_VALUE(KeyCode, F19) = 110,
    IOP_ENUM_VALUE(KeyCode, F20) = 111,
    IOP_ENUM_VALUE(KeyCode, F21) = 112,
    IOP_ENUM_VALUE(KeyCode, F22) = 113,
    IOP_ENUM_VALUE(KeyCode, F23) = 114,
    IOP_ENUM_VALUE(KeyCode, F24) = 115,
    IOP_ENUM_VALUE(KeyCode, F25) = 116,

    IOP_ENUM_VALUE(KeyCode, Menu) = 117,

#ifdef CUBE_LANGUAGE_CPP
    IOP_ENUM_VALUE(KeyCode, Last)
#endif
};

#ifdef CUBE_LANGUAGE_CSHARP
}
#endif

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