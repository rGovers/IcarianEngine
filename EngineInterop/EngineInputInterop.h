#pragma once

#include "InteropTypes.h"

#define ENGINEINPUT_EXPORT_TABLE(F) \
    F(IOP_VEC2, IcarianEngine, InputInterop, GetCursorPos, \
    { \
        return Instance->GetCursorPos(); \
    }) \
    \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetMouseDownState, \
    { \
        return (uint32_t)Instance->IsMouseDown((FlareBase::e_MouseButton)a_button); \
    }, IOP_UINT32 a_button) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetMousePressedState, \
    { \
        return (uint32_t)Instance->IsMousePressed((FlareBase::e_MouseButton)a_button); \
    }, IOP_UINT32 a_button) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetMouseReleasedState, \
    { \
        return (uint32_t)Instance->IsMouseReleased((FlareBase::e_MouseButton)a_button); \
    }, IOP_UINT32 a_button) \
    \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetKeyDownState, \
    { \
        return (uint32_t)Instance->IsKeyDown((FlareBase::e_KeyCode)a_keyCode); \
    }, IOP_UINT32 a_keyCode) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetKeyPressedState, \
    { \
        return (uint32_t)Instance->IsKeyPressed((FlareBase::e_KeyCode)a_keyCode); \
    }, IOP_UINT32 a_keyCode) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetKeyReleasedState, \
    { \
        return (uint32_t)Instance->IsKeyReleased((FlareBase::e_KeyCode)a_keyCode); \
    }, IOP_UINT32 a_keyCode) \
    \
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

#define ENGINEAPPINPUT_EXPORT_TABLE(F) \
    F(IOP_UINT32, IcarianEngine, InputInterop, GetCursorState, \
    { \
        return (uint32_t)Instance->GetCursorState(); \
    }) \
    F(void, IcarianEngine, InputInterop, SetCursorState, \
    { \
        Instance->SetCursorState((FlareBase::e_CursorState)a_state); \
    }, IOP_UINT32 a_state) 
