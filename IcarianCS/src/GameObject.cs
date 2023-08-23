using IcarianEngine.Definitions;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;

namespace IcarianEngine
{
    public class GameObject : IDestroy
    {
        // Naive approach but it works
        // Some concurrent types seem to cause crashes in C# for some reason often with sigbus errors
        static List<Scriptable> s_scriptableComps = new List<Scriptable>();
        static ConcurrentQueue<Scriptable> s_scriptableAddQueue = new ConcurrentQueue<Scriptable>();
        static ConcurrentQueue<Scriptable> s_scriptableRemoveQueue = new ConcurrentQueue<Scriptable>();

        static List<GameObject> s_objs = new List<GameObject>();
        static ConcurrentQueue<GameObject> s_objAddQueue = new ConcurrentQueue<GameObject>();
        static ConcurrentQueue<GameObject> s_objRemoveQueue = new ConcurrentQueue<GameObject>();

        static Dictionary<string, GameObject> s_objDictionary = new Dictionary<string, GameObject>();

        GameObjectDef   m_def = null;

        List<Component> m_components;

        string          m_tag = null;
        string          m_name = null;

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

        public string Name
        {
            get
            {
                return m_name;
            }
            set
            {
                m_name = value;
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

        static void RemoveScripts()
        {
            while (!s_scriptableRemoveQueue.IsEmpty)
            {
                Scriptable script = null;

                s_scriptableRemoveQueue.TryDequeue(out script);

                if (script != null)
                {
                    s_scriptableComps.Remove(script);
                }
                else
                {
                    Logger.IcarianWarning("Scriptable failed to Destroy");
                }
            }
        }

        internal static void UpdateObjects()
        {
            Profiler.StartFrame("Object Update");

            while (!s_objRemoveQueue.IsEmpty)
            {
                GameObject obj = null;

                s_objRemoveQueue.TryDequeue(out obj);

                if (obj != null)
                {
                    s_objs.Remove(obj);

                    if (!string.IsNullOrWhiteSpace(obj.m_tag) && s_objDictionary.ContainsKey(obj.m_tag))
                    {
                        s_objDictionary.Remove(obj.m_tag);
                    }
                }
                else
                {
                    Logger.IcarianWarning("GameObject failed to Destroy");
                }
            }
            while (!s_objAddQueue.IsEmpty)
            {
                GameObject obj = null;
                s_objAddQueue.TryDequeue(out obj);
                if (obj != null)
                {
                    s_objs.Add(obj);
                    if (!string.IsNullOrWhiteSpace(obj.m_tag))
                    {
                        s_objDictionary.Add(obj.m_tag, obj);
                    }

                    obj.Init();
                }
                else
                {
                    Logger.IcarianWarning("GameObject failed to Instantiate");
                }
            }

            foreach (GameObject obj in s_objs)
            {
                if (obj == null || obj.IsDisposed)
                {
                    continue;
                }

                obj.Update();
            }
            
            Profiler.StopFrame();
        }
        internal static void UpdateScripts()
        {
            Profiler.StartFrame("Script Update");

            RemoveScripts();

            while (!s_scriptableAddQueue.IsEmpty)
            {
                Scriptable script = null;

                s_scriptableAddQueue.TryDequeue(out script);

                if (script != null)
                {
                    s_scriptableComps.Add(script);
                }
                else
                {
                    Logger.IcarianWarning("Scriptable failed to Instantiate");
                }
            }

            foreach (Scriptable script in s_scriptableComps)
            {
                if (script == null)
                {
                    continue;
                }
                else if (script is IDestroy destroy)
                {
                    if (destroy.IsDisposed)
                    {
                        continue;
                    }
                }

                script.Update();
            }

            Profiler.StopFrame();
        }
        internal static void DestroyObjects() 
        {
            List<GameObject> objs = new List<GameObject>(s_objs);

            foreach (GameObject obj in objs)
            {
                if (obj == null || obj.IsDisposed)
                {
                    continue;
                }

                obj.Dispose();
            }

            s_objs.Clear();
            s_objDictionary.Clear();

            s_objRemoveQueue = new ConcurrentQueue<GameObject>();
            s_objAddQueue = new ConcurrentQueue<GameObject>();

            RemoveScripts();
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
                s_scriptableAddQueue.Enqueue(script);
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
        public IEnumerable<T> GetComponents<T>() where T : Component
        {
            foreach (Component comp in m_components)
            {
                if (comp is T val)
                {
                    yield return val;
                }
            }
        }
        public Component GetComponent(ComponentDef a_def)
        {
            return GetComponent<Component>(a_def);
        }

        public T GetComponentInChild<T>(bool a_recursive = false) where T : Component
        {
            if (a_recursive)
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject childObj = child.Object;
                    T comp = childObj.GetComponent<T>();
                    if (comp != null)
                    {
                        return comp;
                    }

                    comp = childObj.GetComponentInChild<T>(true);
                    if (comp != null)
                    {
                        return comp;
                    }
                }
            }
            else
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject obj = child.Object;
                    T comp = obj.GetComponent<T>();
                    if (comp != null)
                    {
                        return comp;
                    }
                }
            }

            return null;
        }
        public IEnumerable<T> GetComponentsInChildren<T>(bool a_recursive = false) where T : Component
        {
            if (a_recursive)
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject childObj = child.Object;

                    IEnumerable<T> comps = childObj.GetComponents<T>();
                    foreach (T c in comps)
                    {
                        yield return c;
                    }

                    comps = childObj.GetComponentsInChildren<T>(true);
                    foreach (T c in comps)
                    {
                        yield return c;
                    }
                }
            }
            else
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject obj = child.Object;
                    IEnumerable<T> comps = obj.GetComponents<T>();
                    foreach (T c in comps)
                    {
                        yield return c;
                    }
                }
            }
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
                s_scriptableRemoveQueue.Enqueue(script);
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

            s_objAddQueue.Enqueue(obj);

            return obj;
        }
        public static T Instantiate<T>(string a_tag = null) where T : GameObject
        {
            T obj = Activator.CreateInstance<T>();
            obj.m_tag = a_tag;

            s_objAddQueue.Enqueue(obj);

            return obj;
        }   

        public GameObject GetChildWithName(string a_name, bool a_recursive = false)
        {
            if (a_recursive)
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject childObj = child.Object;
                    if (childObj.m_name == a_name)
                    {
                        return childObj;
                    }

                    GameObject obj = childObj.GetChildWithName(a_name, true);
                    if (obj != null)
                    {
                        return obj;
                    }
                }
            }
            else
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject obj = child.Object;
                    if (obj.m_name == a_name)
                    {
                        return obj;
                    }
                }
            }

            return null;
        }
        public GameObject GetChildWithTag(string a_tag, bool a_recursive = false)
        {
            if (a_recursive)
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject childObj = child.Object;
                    if (childObj.m_tag == a_tag)
                    {
                        return childObj;
                    }

                    GameObject obj = childObj.GetChildWithTag(a_tag, true);
                    if (obj != null)
                    {
                        return obj;
                    }
                }
            }
            else
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject obj = child.Object;
                    if (obj.m_tag == a_tag)
                    {
                        return obj;
                    }
                }
            }

            return null;
        }

        public IEnumerable<GameObject> GetChildrenWithName(string a_name, bool a_recursive = false)
        {
            if (a_recursive)
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject childObj = child.Object;
                    if (childObj.m_name == a_name)
                    {
                        yield return childObj;
                    }

                    IEnumerable<GameObject> objs = childObj.GetChildrenWithName(a_name, true);
                    foreach (GameObject obj in objs)
                    {
                        yield return obj;
                    }
                }
            }
            else
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject obj = child.Object;
                    if (obj.m_name == a_name)
                    {
                        yield return obj;
                    }
                }
            }
            
        }
        public IEnumerable<GameObject> GetChildrenWithTag(string a_tag, bool a_recursive = false)
        {
            if (a_recursive)
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject childObj = child.Object;
                    if (childObj.m_tag == a_tag)
                    {
                        yield return childObj;
                    }

                    IEnumerable<GameObject> objs = childObj.GetChildrenWithTag(a_tag, true);
                    foreach (GameObject obj in objs)
                    {
                        yield return obj;
                    }
                }
            }
            else
            {
                foreach (Transform child in m_transform.Children)
                {
                    GameObject obj = child.Object;
                    if (obj.m_tag == a_tag)
                    {
                        yield return obj;
                    }
                }
            }
        }

        static GameObject ChildDef(GameObjectDef a_def, ref List<Component> a_comps, ref List<GameObject> a_objs)
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

                if (comp is Scriptable script)
                {
                    s_scriptableAddQueue.Enqueue(script);
                }
            }

            foreach (GameObject gameObject in objs)
            {
                s_objAddQueue.Enqueue(gameObject);
            }

            if (obj != null)
            {
                if (!string.IsNullOrEmpty(a_tag))
                {
                    s_objDictionary.Add(a_tag, obj);
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
                    IEnumerable<Transform> children = Transform.Children;
                    foreach (Transform t in children)
                    {
                        GameObject obj = t.Object;
                        if (obj != null && !obj.IsDisposed)
                        {
                            obj.Dispose();
                        }
                    }

                    m_transform.Dispose();
                    m_transform = null;   

                    s_objRemoveQueue.Enqueue(this);

                    foreach (Component comp in m_components)
                    {
                        if (comp is Scriptable script)
                        {
                            s_scriptableRemoveQueue.Enqueue(script);
                        }

                        if (comp is IDestroy dest)
                        {
                            if (!dest.IsDisposed)
                            {
                                dest.Dispose();
                            }
                        }
                        else if (comp is IDisposable disp)
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