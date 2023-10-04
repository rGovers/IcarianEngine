namespace IcarianEngine
{
    public static class Time
    {
        static double s_deltaTime = 0.0;
        static double s_time = 0.0;

        static double s_fixedDeltaTime = 0.0;
        static double s_fixedTime = 0.0;

        static double s_frameDeltaTime = 0.0;
        static double s_frameTime = 0.0;

        /// <summary>
        /// Delta time in seconds as a double.
        /// </summary>
        public static double DDeltaTime
        {
            get
            {
                return s_deltaTime;
            }
            internal set
            {
                s_deltaTime = value;
            }
        }
        /// <summary>
        /// Time passed in seconds as a double.
        /// </summary>
        public static double DTimePassed
        {
            get
            {
                return s_time;
            }
            internal set
            {
                s_time = value;
            }
        }

        /// <summary>
        /// Delta time in seconds as a float.
        /// </summary>
        public static float DeltaTime
        {
            get
            {
                return (float)s_deltaTime;
            }
        }
        /// <summary>
        /// Time passed in seconds as a float.
        /// </summary>
        public static float TimePassed
        {
            get
            {
                return (float)s_time;
            }
        }

        /// <summary>
        /// Fixed delta time in seconds as a double.
        /// </summary>
        public static double DFixedDeltaTime
        {
            get
            {
                return s_fixedDeltaTime;
            }
            internal set
            {
                s_fixedDeltaTime = value;
            }
        }
        /// <summary>
        /// Fixed time passed in seconds as a double.
        /// </summary>
        public static double DFixedTimePassed
        {
            get
            {
                return s_fixedTime;
            }
            internal set
            {
                s_fixedTime = value;
            }
        }

        /// <summary>
        /// Fixed delta time in seconds as a float.
        /// </summary>
        public static float FixedDeltaTime
        {
            get
            {
                return (float)s_fixedDeltaTime;
            }
        }
        /// <summary>
        /// Fixed time passed in seconds as a float.
        /// </summary>
        public static float FixedTimePassed
        {
            get
            {
                return (float)s_fixedTime;
            }
        }

        /// <summary>
        /// Frame delta time in seconds as a double.
        /// </summary>
        public static double DFrameDeltaTime
        {
            get
            {
                return s_frameDeltaTime;
            }
            internal set
            {
                s_frameDeltaTime = value;
            }
        }
        /// <summary>
        /// Frame time passed in seconds as a double.
        /// </summary>
        public static double DFrameTimePassed
        {
            get
            {
                return s_frameTime;
            }
            internal set
            {
                s_frameTime = value;
            }
        }

        /// <summary>
        /// Frame delta time in seconds as a float.
        /// </summary>
        public static float FrameDeltaTime
        {
            get
            {
                return (float)s_frameDeltaTime;
            }
        }
        /// <summary>
        /// Frame time passed in seconds as a float.
        /// </summary>
        public static float FrameTimePassed
        {
            get
            {
                return (float)s_frameTime;
            }
        }
    }
}