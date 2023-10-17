using IcarianEngine.Maths;
using System.Runtime.CompilerServices;

#include "InteropBinding.h"
#include "EngineInputInterop.h"
#include "EngineInputInteropStructures.h"

ENGINEINPUT_EXPORT_TABLE(IOP_BIND_FUNCTION)
ENGINEAPPINPUT_EXPORT_TABLE(IOP_BIND_FUNCTION)

namespace IcarianEngine
{
    public enum MouseButton : ushort
    {
        Left = 0,
        Middle = 1,
        Right = 2
    }

    public enum KeyCode : ushort
    {
        Null = ushort.MaxValue,

        Space = 0, 
        Apostrophe = 1, /* ' */
        Comma = 2, /* , */
        Minus = 3, /* - */
        Equals = 4, /* = */
        Period = 5, /* . */
        ForwardSlash = 6, /* / */ 
        BackSlash = 7, /* \ */
        LeftBracket = 8, /* [ */
        RightBracket = 9, /* ] */
        Semicolon = 10, /* ; */
        Accent = 11, /* ` */

        A = 12,
        B = 13,
        C = 14,
        D = 15,
        E = 16,
        F = 17,
        G = 18,
        H = 19,
        I = 20,
        J = 21,
        K = 22,
        L = 23,
        M = 24,
        N = 25,
        O = 26,
        P = 27,
        Q = 28,
        R = 29,
        S = 30,
        T = 31,
        U = 32,
        V = 33,
        W = 34,
        X = 35,
        Y = 36,
        Z = 37,

        Num0 = 38,
        Num1 = 39,
        Num2 = 40,
        Num3 = 41,
        Num4 = 42,
        Num5 = 43,
        Num6 = 44,
        Num7 = 45,
        Num8 = 46,
        Num9 = 47,

        PrintableEnd = Num9 + 1,

        KeypadDecimal = 48, /* . */
        KeypadDivide = 49, /* / */
        KeypadMultiply = 50, /* * */
        KeypadSubtract = 51, /* - */
        KeypadAdd = 52, /* + */
        KeypadEquals = 53, /* = */
        KeypadEnter = 54,

        Keypad0 = 55,
        Keypad1 = 56,
        Keypad2 = 57,
        Keypad3 = 58,
        Keypad4 = 59,
        Keypad5 = 60,
        Keypad6 = 61,
        Keypad7 = 62,
        Keypad8 = 63,
        Keypad9 = 64,

        Escape = 65,
        Enter = 66,
        Tab = 67,
        Backspace = 68,
        Insert = 69,
        Delete = 70,
        Home = 71,
        End = 72,
        PageUp = 73,
        PageDown = 74,
        LeftArrow = 75,
        RightArrow = 76,
        UpArrow = 77,
        DownArrow = 78,
        CapsLock = 79,
        NumLock = 80,
        ScrollLock = 81,
        PrintScreen = 82,
        PauseBreak = 83,

        LeftShift = 84,
        LeftCtrl = 85,
        LeftAlt = 86,
        LeftSuper = 87, /* Windows Key */

        RightShift = 88,
        RightCtrl = 89,
        RightAlt = 90,
        RightSuper = 91, /* Windows Key */

        F1 = 92,
        F2 = 93,
        F3 = 94,
        F4 = 95,
        F5 = 96,
        F6 = 97,
        F7 = 98,
        F8 = 99,
        F9 = 100,
        F10 = 101,
        F11 = 102,
        F12 = 103,
        F13 = 104,
        F14 = 105,
        F15 = 106,
        F16 = 107,
        F17 = 108,
        F18 = 109,
        F19 = 110,
        F20 = 111,
        F21 = 112,
        F22 = 113,
        F23 = 114,
        F24 = 115,
        F25 = 116,

        Menu = 117,
    };

    public enum CursorState : ushort
    {
        Normal = 0,
        Hidden = 1,
        Locked = 2
    }

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