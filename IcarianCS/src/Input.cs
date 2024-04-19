using IcarianEngine.Maths;
using System.Runtime.CompilerServices;

#include "InteropBinding.h"
#include "EngineInputInterop.h"
#include "EngineInputInteropStructures.h"

/// @cond INTERNAL
ENGINEINPUT_EXPORT_TABLE(IOP_BIND_FUNCTION);
ENGINEAPPINPUT_EXPORT_TABLE(IOP_BIND_FUNCTION);
/// @endcond

namespace IcarianEngine
{
    public static class Input
    {
        /// <summary>
        /// Delegate for mouse button events
        /// </summary>
        public delegate void MouseCallback(MouseButton a_button);
        /// <summary>
        /// Delegate for key events
        /// </summary>
        public delegate void KeyCallback(KeyCode a_key);

        /// <summary>
        /// Callback for when a mouse button is pressed
        /// </summary>
        public static MouseCallback MousePressedCallback;
        /// <summary>
        /// Callback for when a mouse button is released
        /// </summary>
        public static MouseCallback MouseReleasedCallback;

        /// <summary>
        /// Callback for when a key is pressed
        /// </summary>
        public static KeyCallback KeyPressedCallback;
        /// <summary>
        /// Callback for when a key is released
        /// </summary>
        public static KeyCallback KeyReleaseCallback;

        /// <summary>
        /// Gets the current cursor position
        /// </summary>
        /// When the cursor is locked this will return the delta of the cursor movement
        public static Vector2 CursorPosition
        {
            get
            {
                return InputInterop.GetCursorPos();
            }
        }

        /// <summary>
        /// Gets the current cursor state
        /// </summary>
        public static CursorState CursorState
        {
            get
            {
                return (CursorState)InputInterop.GetCursorState();
            }
            set
            {
                InputInterop.SetCursorState((uint)value);
            }
        }

        /// <summary>
        /// If either shift key is pressed
        /// </summary>
        public static bool ShiftModifer
        {
            get
            {
                return InputInterop.GetKeyDownState((uint)KeyCode.LeftShift) != 0 || InputInterop.GetKeyDownState((uint)KeyCode.RightShift) != 0;
            }
        }
        /// <summary>
        /// If either control key is pressed
        /// </summary>
        public static bool CtrlModifier
        {
            get
            {
                return InputInterop.GetKeyDownState((uint)KeyCode.LeftCtrl) != 0 || InputInterop.GetKeyDownState((uint)KeyCode.RightCtrl) != 0;
            }
        }
        /// <summary>
        /// If either alt key is pressed
        /// </summary>
        public static bool AltModifier
        {
            get
            {
                return InputInterop.GetKeyDownState((uint)KeyCode.LeftAlt) != 0 || InputInterop.GetKeyDownState((uint)KeyCode.RightAlt) != 0;
            }
        }

        /// <summary>
        /// If the mouse button is held down
        /// </summary>
        /// <param name="a_button">The mouse button to check</param>
        /// <returns>If the mouse button is held down</returns>
        public static bool IsMouseDown(MouseButton a_button)
        {
            return InputInterop.GetMouseDownState((uint)a_button) != 0;
        }
        /// <summary>
        /// If the mouse button is not held down
        /// </summary>
        /// <param name="a_button">The mouse button to check</param>
        /// <returns>If the mouse button is not held down</returns>
        public static bool IsMouseUp(MouseButton a_button)
        {
            return InputInterop.GetMouseDownState((uint)a_button) == 0;
        }
        /// <summary>
        /// If the mouse button was pressed this update
        /// </summary>
        /// <param name="a_button">The mouse button to check</param>
        /// <returns>If the mouse button was pressed this update</returns>
        public static bool IsMousePressed(MouseButton a_button)
        {
            return InputInterop.GetMousePressedState((uint)a_button) != 0;
        }
        /// <summary>
        /// If the mouse button was released this update
        /// </summary>
        /// <param name="a_button">The mouse button to check</param>
        /// <returns>If the mouse button was released this update</returns>
        public static bool IsMouseReleased(MouseButton a_button)
        {
            return InputInterop.GetMouseReleasedState((uint)a_button) != 0;
        }

        /// <summary>
        /// If the keyboard key is held down
        /// </summary>
        /// <param name="a_keyCode">The keyboard key to check</param>
        /// <returns>If the keyboard key is held down</returns>
        public static bool IsKeyDown(KeyCode a_keyCode)
        {
            return InputInterop.GetKeyDownState((uint)a_keyCode) != 0;
        }
        /// <summary>
        /// If the keyboard key is not held down
        /// </summary>
        /// <param name="a_keyCode">The keyboard key to check</param>
        /// <returns>If the keyboard key is not held down</returns>
        public static bool IsKeyUp(KeyCode a_keyCode)
        {
            return InputInterop.GetKeyDownState((uint)a_keyCode) == 0;
        }
        /// <summary>
        /// If the keyboard key was pressed this update
        /// </summary>
        /// <param name="a_keyCode">The keyboard key to check</param>
        /// <returns>If the keyboard key was pressed this update</returns>
        public static bool IsKeyPressed(KeyCode a_keyCode)
        {
            return InputInterop.GetKeyPressedState((uint)a_keyCode) != 0;
        }
        /// <summary>
        /// If the keyboard key was released this update
        /// </summary>
        /// <param name="a_keyCode">The keyboard key to check</param>
        /// <returns>If the keyboard key was released this update</returns>
        public static bool IsKeyReleased(KeyCode a_keyCode)
        {
            return InputInterop.GetKeyReleasedState((uint)a_keyCode) != 0;
        }

        /// <summary>
        /// Checks if a gamepad is connected
        /// </summary>
        /// <param name="a_slot">The gamepad slot(s) to check</param>
        /// <returns>If the gamepad is connected. False if any are disconnected in the case of multiple.</returns>
        public static bool IsGamePadConnected(GamePadSlot a_slot)
        {
            return InputInterop.GetGamePadConnected((uint)a_slot) != 0;
        }

        /// <summary>
        /// Gets the gamepad axis value
        /// </summary>
        /// <param name="a_slot">The gamepad slot(s) to check</param>
        /// <param name="a_axis">The axis to check</param>
        /// <returns>The axis value. The first one with input in the case of multiple.</returns>
        public static Vector2 GetGamePadAxis(GamePadSlot a_slot, GamePadAxis a_axis)
        {
            return InputInterop.GetGamePadAxis((uint)a_slot, (uint)a_axis);
        }        
        
        /// <summary>
        /// If the gamepad button is held down
        /// </summary>
        /// <param name="a_slot">The gamepad slot(s) to check</param>
        /// <param name="a_button">The button to check</param>
        /// <returns>If the gamepad button is held down. Any down if multiple.</returns>
        public static bool IsGamePadButtonDown(GamePadSlot a_slot, GamePadButton a_button)
        {
            return InputInterop.GetGamePadButtonDownState((uint)a_slot, (uint)a_button) != 0;
        }
        /// <summary>
        /// If the gamepad button is not held down
        /// </summary>
        /// <param name="a_slot">The gamepad slot(s) to check</param>
        /// <param name="a_button">The button to check</param>
        /// <returns>If the gamepad button is not held down. All up if multiple.</returns>
        public static bool IsGamePadButtonUp(GamePadSlot a_slot, GamePadButton a_button)
        {
            return InputInterop.GetGamePadButtonDownState((uint)a_slot, (uint)a_button) == 0;
        }
        /// <summary>
        /// If the gamepad button was pressed this update
        /// </summary>
        /// <param name="a_slot">The gamepad slot(s) to check</param>
        /// <param name="a_button">The button to check</param>
        /// <returns>If the gamepad button was pressed this update. Any pressed if multiple.</returns>
        public static bool IsGamePadButtonPressed(GamePadSlot a_slot, GamePadButton a_button)
        {
            return InputInterop.GetGamePadButtonPressedState((uint)a_slot, (uint)a_button) != 0;
        }
        /// <summary>
        /// If the gamepad button was released this update
        /// </summary>
        /// <param name="a_slot">The gamepad slot(s) to check</param>
        /// <param name="a_button">The button to check</param>
        /// <returns>If the gamepad button was released this update. Any released if multiple.</returns>
        public static bool IsGamePadButtonReleased(GamePadSlot a_slot, GamePadButton a_button)
        {
            return InputInterop.GetGamePadButtonReleasedState((uint)a_slot, (uint)a_button) != 0;
        }

        static void MousePressedEvent(uint a_button)
        {
            if (MousePressedCallback != null)
            {
                MousePressedCallback((MouseButton)a_button);
            }
        }
        static void MouseReleasedEvent(uint a_button)
        {
            if (MouseReleasedCallback != null)
            {
                MouseReleasedCallback((MouseButton)a_button);
            }
        }
        static void KeyPressedEvent(uint a_key)
        {
            if (KeyPressedCallback != null)
            {
                KeyPressedCallback((KeyCode)a_key);
            }
        }
        static void KeyReleasedEvent(uint a_key)
        {
            if (KeyReleaseCallback != null)
            {
                KeyReleaseCallback((KeyCode)a_key);
            }
        }
    }
}