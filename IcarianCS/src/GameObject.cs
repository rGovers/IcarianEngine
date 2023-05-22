using IcarianEngine.Definitions;
using System;
using System.Collections.Generic;

namespace IcarianEngine
{
    public class GameObject : IDestroy
    {
        static List<Scriptable> ScriptableComps = new List<Scriptable>();
        static List<GameObject> Objs = new List<GameObject>();
        static Dictionary<string, GameObject> ObjDictionary = new Dictionary<string, GameObject>();

        GameObjectDef   m_def = null;

        List<Component> m_components;

        string          m_tag = null;

        bool            m_disposed = false;

        Transform       m_transform;

        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        public GameObjectDef Def
        {
            get
            {
                return m_def;
            }
        }

        public string Tag
        {
            get
            {
                return m_tag;
            }
        }

        public Transform Transform
        {
            get
            {
                return m_transform;
            }
        }

        public GameObject Parent 
        {
            get
            {
                return m_transform.Parent.Object;
            }
            set
            {
                if (value == null)
                {
                    m_transform.Parent = null;
                }
                else
                {
                    m_transform.Parent = value.m_transform;
                }
            }
        }

        public IEnumerable<Component> Components
        {
            get
            {
                return m_components;
            }
        }

        public GameObject()
        {
            m_transform = new Transform(this);

            m_components = new List<Component>();
        }

        internal static void UpdateObjects()
        {
            Profiler.StartFrame("Object Update");

            foreach (GameObject obj in Objs)
            {
                obj.Update();
            }

            Profiler.StopFrame();
        }
        internal static void UpdateScripts()
        {
            Profiler.StartFrame("Script Update");

            foreach (Scriptable script in ScriptableComps)
            {
                script.Update();
            }

            Profiler.StopFrame();
        }
        internal static void DestroyObjects() 
        {
            List<GameObject> objList = new List<GameObject>(Objs);
            foreach (GameObject obj in objList)
            {
                obj.Dispose();
            }
        }

        public virtual void Init() { }
        public virtual void Update() { }

        Component AddComponentN(ComponentDef a_def)
        {
            Component comp = Component.FromDef(a_def);
            if (comp != null)
            {
                comp.GameObject = this;

                if (comp != null)
                {
                    m_components.Add(comp);
                }

                if (comp is Scriptable script)
                {
                    ScriptableComps.Add(script);
                }
            }
            
            return comp;
        }

        public T AddComponent<T>() where T : Component
        {
            T comp = Activator.CreateInstance<T>();
            comp.GameObject = this;

            if (comp != null)
            {
                m_components.Add(comp);
            }

            if (comp is Scriptable script)
            {
                ScriptableComps.Add(script);
            }

            comp.Init();

            return comp; 
        }
        public Component AddComponent(ComponentDef a_def)
        {
            Component comp = AddComponentN(a_def);
            
            comp.Init();

            return comp;
        }
        public T AddComponent<T>(ComponentDef a_def) where T : Component
        {
            return AddComponent(a_def) as T;
        }

        public T GetComponent<T>() where T : Component
        {
            foreach (Component comp in m_components)
            {
                if (comp is T val)
                {
                    return val;
                }
            }

            return null;
        }
        public Component GetComponent(ComponentDef a_def)
        {
            return GetComponent<Component>(a_def);
        }
        public T GetComponent<T>(ComponentDef a_def) where T : Component
        {
            foreach (Component comp in m_components)
            {
                if (comp.Def == a_def)
                {
                    return comp as T;
                }
            }

            return null;
        }

        public void RemoveComponent(Component a_component)
        {
            if (a_component is Scriptable script)
            {
                ScriptableComps.Remove(script);
            }

            m_components.Remove(a_component);
        }
        public void RemoveComponent(ComponentDef a_def)
        {
            foreach (Component comp in m_components)
            {
                if (comp.Def == a_def)
                {
                    RemoveComponent(comp);

                    if (comp is IDestroy dest)
                    {
                        if (!dest.IsDisposed)
                        {
                            dest.Dispose();
                        }

                        return;
                    }

                    if (comp is IDisposable disp)
                    {
                        disp.Dispose();
                    }

                    return;
                }
            }
        }

        public static GameObject Instantiate(string a_tag = null)
        {
            GameObject obj = new GameObject()
            {
                m_tag = a_tag
            };

            if (!string.IsNullOrEmpty(a_tag))
            {
                ObjDictionary.Add(a_tag, obj);
            }

            Objs.Add(obj);

            obj.Init();

            return obj;
        }
        public static T Instantiate<T>(string a_tag = null) where T : GameObject
        {
            T obj = Activator.CreateInstance<T>();
            obj.m_tag = a_tag;

            if (!string.IsNullOrEmpty(a_tag))
            {
                ObjDictionary.Add(a_tag, obj);
            }

            Objs.Add(obj);

            obj.Init();

            return obj;
        }   

        public static GameObject ChildDef(GameObjectDef a_def, ref List<Component> a_comps, ref List<GameObject> a_objs)
        {
            GameObject obj = Activator.CreateInstance(a_def.ObjectType) as GameObject;
            if (obj != null)
            {
                obj.m_def = a_def;

                obj.m_transform.Translation = a_def.Translation;
                obj.m_transform.Rotation = a_def.Rotation;
                obj.m_transform.Scale = a_def.Scale;

                if (a_def.Children != null)
                {
                    foreach (GameObjectDef child in a_def.Children)
                    {
                        GameObject c = ChildDef(child, ref a_comps, ref a_objs);
                        if (c != null)
                        {
                            c.Parent = obj;
                        }
                    }
                }

                if (a_def.Components != null)
                {
                    foreach(ComponentDef def in a_def.Components)
                    {
                        a_comps.Add(obj.AddComponentN(def));
                    }
                }

                a_objs.Add(obj);
            }
            
            return obj;
        }
        public static GameObject FromDef(GameObjectDef a_def, string a_tag = null)
        {
            List<Component> comps = new List<Component>();
            List<GameObject> objs = new List<GameObject>();

            // Want to defer initialization until the hierarchy is built so that GetComponent works properly and can get parents and children
            GameObject obj = ChildDef(a_def, ref comps, ref objs);

            foreach (Component comp in comps)
            {
                comp.Init();
            }

            foreach (GameObject gameObject in objs)
            {
                gameObject.Init();

                Objs.Add(gameObject);
            }

            if (obj != null)
            {
                if (!string.IsNullOrEmpty(a_tag))
                {
                    ObjDictionary.Add(a_tag, obj);
                }

                return obj;
            }
            
            return null;
        }
        public static T FromDef<T>(GameObjectDef a_def, string a_tag = null) where T : GameObject
        {
            return FromDef(a_def, a_tag) as T;
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    m_transform.Dispose();
                    m_transform = null;   

                    Objs.Remove(this);

                    if (!string.IsNullOrWhiteSpace(m_tag) && ObjDictionary.ContainsKey(m_tag))
                    {
                        ObjDictionary.Remove(m_tag);
                    }

                    foreach (Component comp in m_components)
                    {
                        if (comp is Scriptable script)
                        {
                            ScriptableComps.Remove(script);
                        }

                        if (comp is IDestroy dest)
                        {
                            if (!dest.IsDisposed)
                            {
                                dest.Dispose();
                            }

                            continue;
                        }

                        if (comp is IDisposable disp)
                        {
                            disp.Dispose();
                        }
                    }
                    
                    m_components.Clear();
                }
                else
                {
                    Logger.IcarianWarning("GameObject Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.IcarianError("GameObject Multiple Dispose");
            }
        }

        ~GameObject()
        {
            Dispose(false);
        }
    }
}