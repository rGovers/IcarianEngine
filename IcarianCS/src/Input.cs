using IcarianEngine.Maths;
using System.Runtime.CompilerServices;

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
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static Vector2 GetCursorPos();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetCursorState();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetCursorState(uint a_state);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetMouseDownState(uint a_button);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetMousePressedState(uint a_button);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetMouseReleasedState(uint a_button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetKeyDownState(uint a_button);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetKeyPressedState(uint a_button);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetKeyReleasedState(uint a_button);

        public delegate void MouseCallback(MouseButton a_button);
        public delegate void KeyCallback(KeyCode a_key);

        public static MouseCallback MousePressedCallback;
        public static MouseCallback MouseReleasedCallback;

        public static KeyCallback KeyPressedCallback;
        public static KeyCallback KeyReleaseCallback;

        public static Vector2 CursorPosition
        {
            get
            {
                return GetCursorPos();
            }
        }

        public static CursorState CursorState
        {
            get
            {
                return (CursorState)GetCursorState();
            }
            set
            {
                SetCursorState((uint)value);
            }
        }

        public static bool ShiftModifer
        {
            get
            {
                return GetKeyDownState((uint)KeyCode.LeftShift) != 0 || GetKeyDownState((uint)KeyCode.RightShift) != 0;
            }
        }
        public static bool CtrlModifier
        {
            get
            {
                return GetKeyDownState((uint)KeyCode.LeftCtrl) != 0 || GetKeyDownState((uint)KeyCode.RightCtrl) != 0;
            }
        }
        public static bool AltModifier
        {
            get
            {
                return GetKeyDownState((uint)KeyCode.LeftAlt) != 0 || GetKeyDownState((uint)KeyCode.RightAlt) != 0;
            }
        }

        public static bool IsMouseDown(MouseButton a_button)
        {
            return GetMouseDownState((uint)a_button) != 0;
        }
        public static bool IsMouseUp(MouseButton a_button)
        {
            return GetMouseDownState((uint)a_button) == 0;
        }
        public static bool IsMousePressed(MouseButton a_button)
        {
            return GetMousePressedState((uint)a_button) != 0;
        }
        public static bool IsMouseReleased(MouseButton a_button)
        {
            return GetMouseReleasedState((uint)a_button) != 0;
        }

        public static bool IsKeyDown(KeyCode a_keyCode)
        {
            return GetKeyDownState((uint)a_keyCode) != 0;
        }
        public static bool IsKeyUp(KeyCode a_keyCode)
        {
            return GetKeyDownState((uint)a_keyCode) == 0;
        }
        public static bool IsKeyPressed(KeyCode a_keyCode)
        {
            return GetKeyPressedState((uint)a_keyCode) != 0;
        }
        public static bool IsKeyReleased(KeyCode a_keyCode)
        {
            return GetKeyReleasedState((uint)a_keyCode) != 0;
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