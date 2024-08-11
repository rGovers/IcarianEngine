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

        /// <summary>
        /// Gets the number of GameObjects in existence
        /// </summary>
        public static uint GameObjectCount
        {
            get
            {
                return (uint)s_objs.Count;
            }
        }

        /// <summary>
        /// Whether or not the GameObject has been disposed
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Definitions.GameObjectDef" /> used to create this GameObject
        /// </summary>
        public GameObjectDef Def
        {
            get
            {
                return m_def;
            }
        }

        /// <summary>
        /// The tag of the GameObject
        /// </summary>
        public string Tag
        {
            get
            {
                return m_tag;
            }
        }

        /// <summary>
        /// The name of the GameObject
        /// </summary>
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

        /// <summary>
        /// The <see cref="IcarianEngine.Transform" /> of the GameObject
        /// </summary>
        public Transform Transform
        {
            get
            {
                return m_transform;
            }
        }

        /// <summary>
        /// The parent of the GameObject
        /// </summary>
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

        /// <summary>
        /// The <see cref="IcarianEngine.Component" />(s) of the GameObject
        /// </summary>
        public IEnumerable<Component> Components
        {
            get
            {
                return m_components;
            }
        }

        /// <summary>
        /// The children of the GameObject
        /// </summary>
        public IEnumerable<GameObject> Children
        {
            get
            {
                foreach (Transform child in m_transform.Children)
                {
                    yield return child.Object;
                }
            }
        }

        GameObject()
        {
            m_components = new List<Component>();
        }

        static void RemoveScripts()
        {
            while (!s_scriptableRemoveQueue.IsEmpty)
            {
                Scriptable script = null;

                s_scriptableRemoveQueue.TryDequeue(out script);

                if (!(script is null))
                {
                    s_scriptableComps.Remove(script);
                }
                else
                {
                    Logger.IcarianWarning($"Scriptable {script.GetType()} failed to Destroy");
                }
            }
        }

        static void RemoveObjects()
        {
            while (!s_objRemoveQueue.IsEmpty)
            {
                GameObject obj = null;

                s_objRemoveQueue.TryDequeue(out obj);

                if (!(obj is null))
                {
                    if (obj.m_components != null)
                    {
                        foreach (Component comp in obj.m_components)
                        {
                            if (comp is null)
                            {
                                continue;
                            }

                            if (comp is Scriptable script)
                            {
                                // Cannot ensure the script exists yet if destroyed while initialising
                                // Use the queue to ensure proper deletion
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

                        obj.m_components.Clear();
                    }
                    else
                    {
                        Logger.IcarianWarning("GameObject null Components");
                    }

                    s_objs.Remove(obj);

                    if (!string.IsNullOrWhiteSpace(obj.m_tag) && s_objDictionary.ContainsKey(obj.m_tag))
                    {
                        s_objDictionary.Remove(obj.m_tag);
                    }

                    if (obj.m_transform != null)
                    {
                        obj.m_transform.Dispose();
                        obj.m_transform = null;
                    }
                    else
                    {
                        // TODO: Getting a null Transform rarely on GameObjects created at runtime 
                        // Put a guard in as should not crash the app but need to figure out why it is happening in the first place
                        // The part that makes it fucking weird is I know it exists because it should crash somewhere else when setting the Transform
                        // Losing the reference somehow and need to find where
                        Logger.IcarianWarning("GameObject null Transform");
                    }
                }
                else
                {
                    Logger.IcarianWarning("GameObject failed to Destroy");
                }
            }
        }

        internal static void UpdateObjects()
        {
            Profiler.StartFrame("Object Update");

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
            
            RemoveObjects();

            Profiler.StopFrame();
        }
        internal static void UpdateScripts()
        {
            Profiler.StartFrame("Script Update");

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

            RemoveScripts();

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
        internal static void FixedUpdateScripts()
        {
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

                script.FixedUpdate();
            }
        }

        internal static void DestroyObjects() 
        {
            List<GameObject> objs = new List<GameObject>(s_objs);

            foreach (GameObject obj in objs)
            {
                if (obj == null)
                {
                    continue;
                }

                obj.Dispose();
            }

            RemoveObjects();
            
            s_objs.Clear();
            s_objDictionary.Clear();

            s_objRemoveQueue = new ConcurrentQueue<GameObject>();
            s_objAddQueue = new ConcurrentQueue<GameObject>();

            RemoveScripts();
        }

        /// <summary>
        /// Called when the GameObject is created
        /// </summary>
        public virtual void Init() { }

        internal static void AddScriptable(Scriptable a_script)
        {
            s_scriptableAddQueue.Enqueue(a_script);
        }

        internal Component AddComponentN(ComponentDef a_def)
        {
            Component comp = Component.FromDef(a_def);
            if (!(comp is null))
            {
                comp.GameObject = this;
                m_components.Add(comp);
            }
            
            return comp;
        }

        /// <summary>
        /// Adds a <see cref="IcarianEngine.Component" /> of type T to the GameObject
        /// </summary>
        /// <returns>The added <see cref="IcarianEngine.Component" /></returns>
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
        /// <summary>
        /// Adds a <see cref="IcarianEngine.Component" /> from a <see cref="IcarianEngine.Definitions.ComponentDef" /> to the GameObject
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.ComponentDef" /> to add</param>
        /// <returns>The added <see cref="IcarianEngine.Component" /></returns>
        public Component AddComponent(ComponentDef a_def)
        {
            Component comp = AddComponentN(a_def);
            
            if (comp is Scriptable script)
            {
                s_scriptableAddQueue.Enqueue(script);
            }

            comp.Init();

            return comp;
        }
        /// <summary>
        /// Adds a <see cref="IcarianEngine.Component" /> of Type T from a <see cref="IcarianEngine.Definitions.ComponentDef" /> to the GameObject
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.ComponentDef" /> to add</param>
        /// <returns>The added <see cref="IcarianEngine.Component" /></returns>
        public T AddComponent<T>(ComponentDef a_def) where T : Component
        {
            return AddComponent(a_def) as T;
        }

        /// <summary>
        /// Gets a <see cref="IcarianEngine.Component" /> of type T from the GameObject
        /// </summary>
        /// <returns>The <see cref="IcarianEngine.Component" /> of type T. Null on failure.</returns>
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
        /// <summary>
        /// Gets all <see cref="IcarianEngine.Component" /> of type T from the GameObject
        /// </summary>
        /// <returns>All <see cref="IcarianEngine.Component" /> of type T</returns>
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
        /// <summary>
        /// Gets a <see cref="IcarianEngine.Component" /> with <see cref="IcarianEngine.Definitions.ComponentDef "/> from the GameObject
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.ComponentDef" /> to get</param>
        /// <returns>The <see cref="IcarianEngine.Component" /> with <see cref="IcarianEngine.Definitions.ComponentDef" />. Null on failure.</returns>
        public Component GetComponent(ComponentDef a_def)
        {
            return GetComponent<Component>(a_def);
        }

        /// <summary>
        /// Gets a <see cref="IcarianEngine.Component" /> of type T with <see cref="IcarianEngine.Definitions.ComponentDef "/> from the GameObject
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.ComponentDef "/> to get</param>
        /// <returns>The <see cref="IcarianEngine.Component" /> of type T with <see cref="IcarianEngine.Definitions.ComponentDef" />. Null on failure.</returns>
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

        /// <summary>
        /// Gets a <see cref="IcarianEngine.Component" /> of type T from children of the GameObject
        /// </summary>
        /// <param name="a_recursive">Whether or not to search recursively</param>
        /// <returns>The <see cref="IcarianEngine.Component" /> of type T from children of the GameObject. Null on failure.</returns>
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
        /// <summary>
        /// Gets all <see cref="IcarianEngine.Component" /> of type T from children of the GameObject
        /// </summary>
        /// <param name="a_recursive">Whether or not to search recursively</param>
        /// <returns>All <see cref="IcarianEngine.Component" /> of type T from children of the GameObject</returns>
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

        /// <summary>
        /// Removes the <see cref="IcarianEngine.Component" /> from the GameObject
        /// </summary>
        /// <param name="a_component">The <see cref="IcarianEngine.Component" /> to remove</param>
        public void RemoveComponent(Component a_component)
        {
            if (a_component is Scriptable script)
            {
                s_scriptableRemoveQueue.Enqueue(script);
            }

            m_components.Remove(a_component);

            if (a_component is IDestroy dest)
            {
                if (!dest.IsDisposed)
                {
                    dest.Dispose();
                }
            }
            else if (a_component is IDisposable disp)
            {
                disp.Dispose();
            }
        }
        /// <summary>
        /// Removes the <see cref="IcarianEngine.Component" /> of the <see cref="IcarianEngine.Definitions.ComponentDef" /> from the GameObject
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.ComponentDef" /> of the <see cref="IcarianEngine.Component" /> to remove</param>
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

        /// <summary>
        /// Instantiates a GameObject
        /// </summary>
        /// <param name="a_tag">The tag of the GameObject</param>
        /// <returns>The instantiated GameObject</returns>
        public static GameObject Instantiate(string a_tag = null)
        {
            GameObject obj = new GameObject()
            {
                m_tag = a_tag
            };
            obj.m_transform = new Transform(obj);

            s_objAddQueue.Enqueue(obj);

            return obj;
        }
        /// <summary>
        /// Instantiates a GameObject of type T
        /// </summary>
        /// <param name="a_tag">The tag of the GameObject</param>
        /// <returns>The instantiated GameObject of type T</returns>
        public static T Instantiate<T>(string a_tag = null) where T : GameObject
        {
            T obj = Activator.CreateInstance(typeof(T), true) as T;
            obj.m_tag = a_tag;
            obj.m_transform = new Transform(obj);

            s_objAddQueue.Enqueue(obj);

            return obj;
        }   
        
        /// <summary>
        /// Instantiates N number of GameObjects at once
        /// </summary>
        /// <param name="a_count">The number of GameObjects to create</param>
        /// <returns>The instantiated GameObjects</returns>
        public static GameObject[] BatchInstantiate(uint a_count)
        {
            GameObject[] objects = new GameObject[a_count];
            for (uint i = 0; i < a_count; ++i)
            {
                objects[i] = new GameObject();
            }

            Transform[] transforms = Transform.BatchGenerateTransforms(objects);

            for (uint i = 0; i < a_count; ++i)
            {
                objects[i].m_transform = transforms[i];

                s_objAddQueue.Enqueue(objects[i]);
            }

            return objects;
        }

        /// <summary>
        /// Gets a child of the GameObject with the name
        /// </summary>
        /// <param name="a_name">The name of the child</param>
        /// <param name="a_recursive">Whether or not to search recursively</param>
        /// <returns>The child of the GameObject with the name. Null on failure.</returns>
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

        /// <summary>
        /// Gets all children of the GameObject with the name
        /// </summary>
        /// <param name="a_name">The name of the children</param>
        /// <param name="a_recursive">Whether or not to search recursively</param>
        /// <returns>All children of the GameObject with the name</returns>
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

        /// <summary>
        /// Finds a GameObject with the tag
        /// </summary>
        /// <param name="a_tag">The tag of the GameObject</param>
        /// <returns>The GameObject with the tag. Null on failure.</returns>
        public static GameObject FindGameObjectWithTag(string a_tag)
        {
            if (s_objDictionary.ContainsKey(a_tag))
            {
                return s_objDictionary[a_tag];
            }

            return null;
        }
        /// <summary>
        /// Finds a GameObject of type T with the tag
        /// </summary>
        /// <param name="a_tag">The tag of the GameObject</param>
        /// <returns>The GameObject of type T with the tag. Null on failure.</returns>
        public static T FindGameObjectWithTag<T>(string a_tag) where T : GameObject
        {
            if (s_objDictionary.ContainsKey(a_tag))
            {
                return s_objDictionary[a_tag] as T;
            }

            return null;
        }

        static GameObject ChildDef(GameObjectDef a_def, ref List<Component> a_comps, ref List<GameObject> a_objs)
        {
            GameObject obj = Activator.CreateInstance(a_def.ObjectType, true) as GameObject;
            if (!(obj is null))
            {
                obj.m_transform = new Transform(obj);

                obj.m_def = a_def;
                obj.m_name = a_def.Name;

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

        /// <summary>
        /// Creates a GameObject from a <see cref="IcarianEngine.Definitions.GameObjectDef"/>
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.GameObjectDef"/> to create the GameObject from</param>
        /// <param name="a_tag">The tag of the GameObject</param>
        /// <returns>The created GameObject</returns>
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
                return obj;
            }
            
            return null;
        }
        /// <summary>
        /// Creates a GameObject of type T from a <see cref="IcarianEngine.Definitions.GameObjectDef"/>
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.GameObjectDef"/> to create the GameObject from</param>
        /// <param name="a_tag">The tag of the GameObject</param>
        /// <returns>The created GameObject of type T</returns>
        public static T FromDef<T>(GameObjectDef a_def, string a_tag = null) where T : GameObject
        {
            return FromDef(a_def, a_tag) as T;
        }

        /// <summary>
        /// Disposes of the GameObject
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the GameObject is destroyed
        /// </summary>
        /// <param name="a_disposing">Whether or not the GameObject is being disposed</param>
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
                        if (obj != null)
                        {
                            obj.Dispose();
                        }
                    }

                    s_objRemoveQueue.Enqueue(this);
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

        public override bool Equals(object a_obj)
        {
            if (base.Equals(a_obj))
            {
                return true;
            }

            if (a_obj == null)
            {
                if (IsDisposed)
                {
                    return true;
                }
            }

            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public static bool operator ==(GameObject a_lhs, GameObject a_rhs)
        {
            if (a_lhs is null)
            {
                return a_rhs is null;
            }

            return a_lhs.Equals(a_rhs);
        }
        public static bool operator !=(GameObject a_lhs, GameObject a_rhs)
        {
            return !(a_lhs == a_rhs);
        }
    }
}