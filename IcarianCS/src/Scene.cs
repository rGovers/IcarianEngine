using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Mod;
using IcarianEngine.Physics;
using System;
using System.Collections.Generic;
using System.Xml;

namespace IcarianEngine
{
    public class SceneObject
    {
        /// <summary>
        /// The position of the SceneObject
        /// </summary>
        public Vector3 Translation;
        /// <summary>
        /// The rotation of the SceneObject
        /// </summary>
        public Quaternion Rotation;
        /// <summary>
        /// The scale of the SceneObject
        /// </summary>
        public Vector3 Scale;
        /// <summary>
        /// The <see cref="IcarianEngine.Definitions.GameObjectDef" /> the SceneObject uses
        /// </summary> 
        public string DefName;
    }
    public class SceneObjectArray
    {
        /// <summary>
        /// The starting position of the SceneObjectArray
        /// </summary>
        public Vector3 Translation;
        /// <summary>
        /// The rotation of the SceneObjectArray
        /// </summary>
        public Quaternion Rotation;
        /// <summary>
        /// The number of <see cref="IcarianEngine.GameObject" />(s) to create along each axis
        /// </summary>
        public IVector3 Count;
        /// <summary>
        /// The spacing on each axis between <see cref="IcarianEngine.GameObject" />(s)
        /// </summary>
        public Vector3 Spacing;
        /// <summary>
        /// The <see cref="IcarianEngine.Definitions.GameObjectDef" />s the SceneObjectArray uses
        /// </summary> 
        public string DefName;
    }

    /// @cond INTERNAL

    class LoadSceneThreadJob : IThreadJob
    {
        string                  m_path;
        Scene.LoadSceneCallback m_callback;

        public LoadSceneThreadJob(string a_path, Scene.LoadSceneCallback a_callback)
        {
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            Scene scene = Scene.LoadScene(m_path, false);

            if (m_callback != null)
            {
                if (scene != null)
                {
                    m_callback(scene, LoadStatus.Loaded);
                }
                else
                {
                    m_callback(null, LoadStatus.Failed);
                }
            }
        }
    }

    /// @endcond

    public class Scene : IDestroy
    {
        /// <summary>
        /// Delegate for async Scene loading
        /// </summary>
        public delegate void LoadSceneCallback(Scene a_scene, LoadStatus a_status);

        bool                   m_disposed = false;
    
        List<Def>              m_defs;

        List<SceneObject>      m_sceneObjects;
        List<SceneObjectArray> m_sceneObjectArrays;
        List<GameObject>       m_objects;

        /// <summary>
        /// Whether or not the Scene has been Disposed
        /// </summary>
        public bool IsDisposed 
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Definitions.Def" />(s) in the Scene
        /// </summary>
        public IEnumerable<Def> Defs
        {
            get
            {
                return m_defs;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.GameObject" />(s) in the Scene
        /// </summary>
        public IEnumerable<GameObject> GameObjects
        {
            get
            {
                return m_objects;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.SceneObject" />(s) in the Scene
        /// </summary>
        public IEnumerable<SceneObject> SceneObjects
        {
            get
            {
                return m_sceneObjects;
            }
        }
        /// <summary>
        /// The <see cref="IcarianEngine.SceneObjectArray" />(s) in the Scene
        /// </summary>
        public IEnumerable<SceneObjectArray> SceneObjectArrays
        {
            get
            {
                return m_sceneObjectArrays;
            }
        }

        void LoadSceneObject(XmlElement a_element)
        {
            SceneObject obj = new SceneObject()
            {
                Translation = Vector3.Zero,
                Rotation = Quaternion.Identity,
                Scale = Vector3.One
            };

            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    switch (element.Name)
                    {
                    case "Translation":
                    {
                        obj.Translation = element.ToVector3();

                        break;
                    }
                    case "Rotation":
                    {
                        obj.Rotation = Quaternion.Normalized(element.ToQuaternion());

                        break;
                    }
                    case "AxisAngle":
                    {
                        Vector4 rot = element.ToVector4(Vector4.Zero);

                        obj.Rotation = Quaternion.FromAxisAngle(Vector3.Normalized(rot.XYZ), rot.W);

                        break;
                    }
                    case "Scale":
                    {
                        obj.Scale = element.ToVector3(Vector3.One);

                        break;
                    }
                    case "DefName":
                    {
                        obj.DefName = element.InnerText;

                        break;
                    }
                    default:
                    {
                        Logger.IcarianError($"Invalid Scene element: {element.Name}");

                        return;
                    }
                    }
                }
            }

            if (!string.IsNullOrWhiteSpace(obj.DefName))
            {
                m_sceneObjects.Add(obj);
            }
            else
            {
                Logger.IcarianWarning($"Invalid Scene Object");
            }
        }

        void LoadSceneObjectArray(XmlElement a_element)
        {
            SceneObjectArray arr = new SceneObjectArray()
            {
                Translation = Vector3.Zero,
                Rotation = Quaternion.Identity,
                Count = IVector3.One,
                Spacing = Vector3.One
            };

            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    switch (element.Name)
                    {
                    case "Translation":
                    {
                        arr.Translation = element.ToVector3();

                        break;
                    }
                    case "Rotation":
                    {
                        arr.Rotation = Quaternion.Normalized(element.ToQuaternion());

                        break;
                    }
                    case "AxisAngle":
                    {
                        Vector4 rot = element.ToVector4(Vector4.Zero);

                        arr.Rotation = Quaternion.FromAxisAngle(Vector3.Normalized(rot.XYZ), rot.W);

                        break;
                    }
                    case "Count":
                    {
                        arr.Count = element.ToIVector3(IVector3.One);

                        break;
                    }
                    case "Spacing":
                    {
                        arr.Spacing = element.ToVector3(Vector3.One);

                        break;
                    }
                    case "DefName":
                    {
                        arr.DefName = element.InnerText;

                        break;
                    }
                    default:
                    {
                        Logger.IcarianError($"Invalid Scene element: {element.Name}");

                        return;
                    }
                    }
                }
            }

            if (!string.IsNullOrWhiteSpace(arr.DefName))
            {
                m_sceneObjectArrays.Add(arr);
            }
            else
            {
                Logger.IcarianWarning($"Invalid Scene Object Array");
            }
        }

        void LoadObjects(XmlElement a_element)
        {
            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    switch (element.Name)
                    {
                    case "GameObject":
                    {
                        LoadSceneObject(element);

                        break;
                    }
                    case "GameObjectArray":
                    {
                        LoadSceneObjectArray(element);

                        break;
                    }
                    default:
                    {
                        Logger.IcarianError($"Invalid Scene element: {element.Name}");

                        return;
                    }
                    }
                }            
            }
        }
        void LoadDefs(XmlElement a_element)
        {
            List<DefData> data = new List<DefData>();

            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    data.Add(DefLibrary.GetDefData(Def.SceneDefPath, element));
                }
            }

            DefLibrary.LoadSceneDefs(data);

            DefLibrary.ResolveSceneDefs();

            foreach (DefData dat in data)
            {
                Def def = DefLibrary.GetDef(dat.Name);

                if (def != null)
                {
                    m_defs.Add(def);
                }
            }
        }

        internal Scene(XmlDocument a_doc)
        {
            m_defs = new List<Def>();
            m_sceneObjects = new List<SceneObject>();
            m_sceneObjectArrays = new List<SceneObjectArray>();
            m_objects = new List<GameObject>();

            if (a_doc.DocumentElement is XmlElement root)
            {
                foreach (XmlNode node in root.ChildNodes)
                {
                    if (node is XmlElement element)
                    {
                        switch (element.Name)
                        {
                        case "Objects":
                        {
                            LoadObjects(element);

                            break;
                        }
                        case "Defs":
                        {
                            LoadDefs(element);

                            break;
                        }
                        default:
                        {
                            Logger.IcarianError($"Invalid scene element {element.Name}");

                            break;
                        }
                        }
                    }                    
                }
            }
        }

        /// <summary>
        /// Loads a Scene from file
        /// </summary>
        /// <param name="a_path">The path to the Scene relative to a <see cref="IcarianEngine.Mod.IcarianAssembly" /> Scenes folder</param>
        /// <param name="a_generate">Should generate the Scene immediately after loading</param>
        /// <returns>The scene. Null on failure</returns>
        public static Scene LoadScene(string a_path, bool a_generate = true)
        {
            string path = ModControl.GetScenePath(a_path);
            if (string.IsNullOrEmpty(path))
            {
                Logger.IcarianWarning("Scene not found");

                return null;
            }

            XmlDocument doc = new XmlDocument();
            doc.Load(path);

            Scene scene = new Scene(doc);
            if (a_generate)
            {
                scene.GenerateScene(Matrix4.Identity);
            }

            return scene;
        }
        /// <summary>
        /// Loads a Scene from file
        /// </summary>
        /// <param name="a_path"> The path to the Scene relative to a <see cref="IcarianEngine.Mod.IcarianAssembly" /> Scenes folder</param>
        /// <param name="a_callback">The callback to call after loading</param>
        /// <param name="a_priority">The priority to execute the loading</param>
        /// This does not generate the Scene automatically after loading
        public static void LoadSceneAsync(string a_path, LoadSceneCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            LoadSceneThreadJob job = new LoadSceneThreadJob(a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }

        GameObject GenerateGameObject(Matrix4 a_transform, GameObjectDef a_def)
        {
            Matrix4 t = Matrix4.FromTransform(a_def.Translation, a_def.Rotation, a_def.Scale) * a_transform;

            GameObject obj = GameObject.Instantiate();
            obj.Transform.SetMatrix(t);

            if (a_def.Children != null)
            {
                foreach (GameObjectDef def in a_def.Children)
                {
                    if (def == null)
                    {
                        continue;
                    }

                    GameObject child = GenerateGameObject(Matrix4.Identity, def);
                    child.Transform.Parent = obj.Transform;
                }
            }

            if (a_def.Components != null)
            {
                List<Component> comps = new List<Component>();

                foreach (ComponentDef def in a_def.Components)
                {
                    if (def == null)
                    {
                        continue;
                    }

                    comps.Add(obj.AddComponentN(def));
                }

                foreach (Component comp in comps)
                {
                    comp.Init();

                    if (comp is Scriptable script)
                    {
                        GameObject.AddScriptable(script);
                    }
                }
            }

            m_objects.Add(obj);

            return obj;
        }

        /// <summary>
        /// Generates the <see cref="IcarianEngine.GameObject" />(s) in the Scene from <see cref="IcarianEngine.SceneObject" />(s)/<see cref="IcarianEngine.SceneObjectArray" />(s)
        /// </summary>
        /// <param name="a_transform">Transformation matrix to apply to the Scene</param>
        public void GenerateScene(Matrix4 a_transform)
        {
            // TODO: Naive implementation of this function should be doing it in batches
            foreach (SceneObject obj in m_sceneObjects)
            {
                GameObjectDef def = DefLibrary.GetDef<GameObjectDef>(obj.DefName);
                if (def != null)
                {
                    Matrix4 t = Matrix4.FromTransform(obj.Translation, obj.Rotation, obj.Scale) * a_transform;

                    GenerateGameObject(t, def);
                }
                else
                {
                    Logger.IcarianWarning($"SceneObject invalid Def: {obj.DefName}");
                }
            }

            foreach (SceneObjectArray arr in m_sceneObjectArrays)
            {
                GameObjectDef def = DefLibrary.GetDef<GameObjectDef>(arr.DefName);
                if (def != null)
                {
                    Matrix4 rotMat = arr.Rotation.ToMatrix();

                    for (int x = 0; x < arr.Count.X; ++x)
                    {
                        for (int y = 0; y < arr.Count.Y; ++y)
                        {
                            for (int z = 0; z < arr.Count.Z; ++z)
                            {
                                Vector3 pos = new Vector3((float)x, (float)y, (float)z) * arr.Spacing;
                                Matrix4 transMat = new Matrix4
                                (
                                    Vector4.UnitX,
                                    Vector4.UnitY,
                                    Vector4.UnitZ,
                                    new Vector4(arr.Translation + arr.Rotation * pos, 1.0f)
                                );

                                Matrix4 matrix = rotMat * transMat;

                                GenerateGameObject(matrix * a_transform, def);
                            }
                        }
                    }
                }
                else
                {
                    Logger.IcarianWarning($"SceneObjectArray invalid Def: {arr.DefName}");
                }
            }
        }

        /// <summary>
        /// Clears all <see cref="IcarianEngine.GameObject" />(s) in the Scene
        /// </summary>
        public void FlushScene()
        {
            foreach (GameObject obj in m_objects)
            {
                if (obj != null && !obj.IsDisposed)
                {
                    obj.Dispose();
                }
            }

            m_objects.Clear();
        }

        /// <summary>
        /// Disposes of the Scene
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the Scene is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether or not the Scene is being Disposed</param>
        protected void Dispose(bool a_disposing)
        {
            if (!m_disposed)
            {
                if (a_disposing)
                {
                    FlushScene();
                }
                else
                {
                    Logger.IcarianWarning("Scene Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.IcarianError("Scene Multiple Dispose");
            }
        }
        ~Scene()
        {
            Dispose(false);
        }
    }
}