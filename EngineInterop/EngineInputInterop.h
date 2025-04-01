// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "InteropTypes.h"

/// @file EngineInputInterop.h

/// @cond INTERNAL

#ifdef CUBE_LANGUAGE_CPP
#include "DeletionQueue.h"
#endif

#define ENGINE_INPUT_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetGamePadConnected, \
    { \
        return (uint32_t)Instance->IsGamePadConnected((e_GamePadSlot)a_slot); \
    }, IOP_UINT32 a_slot) \
    F(IOP_VEC2, IcarianEngine, InputInterop, GetGamePadAxis, \
    { \
        return Instance->GetGamePadAxis((e_GamePadSlot)a_slot, (e_GamePadAxis)a_axis); \
    }, IOP_UINT32 a_slot, IOP_UINT32 a_axis) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetGamePadButtonDownState, \
    { \
        return (uint32_t)Instance->IsGamePadButtonDown((e_GamePadSlot)a_slot, (e_GamePadButton)a_button); \
    }, IOP_UINT32 a_slot, IOP_UINT32 a_button) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetGamePadButtonPressedState, \
    { \
        return (uint32_t)Instance->IsGamePadButtonPressed((e_GamePadSlot)a_slot, (e_GamePadButton)a_button); \
    }, IOP_UINT32 a_slot, IOP_UINT32 a_button) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetGamePadButtonReleasedState, \
    { \
        return (uint32_t)Instance->IsGamePadButtonReleased((e_GamePadSlot)a_slot, (e_GamePadButton)a_button); \
    }, IOP_UINT32 a_slot, IOP_UINT32 a_button) \

#define ENGINE_APPINPUT_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetCursorState, \
    { \
        return (uint32_t)Instance->GetCursorState(); \
    }) \
    F(void, IcarianEngine, InputInterop, SetCursorState, \
    { \
        IPUSHDELETIONFUNC( \
        { \
            Instance->SetCursorState((e_CursorState)a_state); \
        }, DeletionIndex_Update); \
    }, IOP_UINT32 a_state) 

/// @endcond

// MIT License
// 
// Copyright (c) 2025 River Govers
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