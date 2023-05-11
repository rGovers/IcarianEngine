namespace IcarianEngine.Mod
{
    public struct FlareAssemblyInfo
    {
        string m_id;
        string m_name;
        string m_path;
        string m_description;

        public string ID
        {
            get
            {
                return m_id;
            }
        }
        public string Name
        {
            get
            {
                return m_name;
            }
        }
        public string Path
        {
            get
            {
                return m_path;
            }
        }
        public string Description
        {
            get
            {
                return m_description;
            }
            set
            {
                m_description = value;
            }
        }

        internal FlareAssemblyInfo(string a_id, string a_name, string a_path, string a_description)
        {
            m_id = a_id;
            m_name = a_name;
            m_path = a_path;
            m_description = a_description;
        }
    }
}