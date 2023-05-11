namespace IcarianEngine
{
    public class Time
    {
        static Time Instance = null;

        double m_deltaTime;
        double m_time;

        public static double DDeltaTime
        {
            get
            {
                return Instance.m_deltaTime;
            }
            internal set
            {
                Instance.m_deltaTime = value;
            }
        }
        public static double DTimePassed
        {
            get
            {
                return Instance.m_time;
            }
            internal set
            {
                Instance.m_time = value;
            }
        }

        public static float DeltaTime
        {
            get
            {
                return (float)Instance.m_deltaTime;
            }
        }
        public static float TimePassed
        {
            get
            {
                return (float)Instance.m_time;
            }
        }

        Time()
        {
            m_time = 0.0f;
            m_deltaTime = 0.0f;
        }

        internal static void Init()
        {
            Instance = new Time();
        }
    }
}