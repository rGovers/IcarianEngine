using IcarianEngine.Maths;
using IcarianEngine.Mod;
using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Xml;

namespace IcarianEngine.Definitions
{
    public struct DefDataObject
    {
        public string Name;
        public string Text;
        public List<DefDataObject> Children;
    }

    public struct DefData
    {
        public string Type;
        public string Path;
        public string Name;
        public string Parent;
        public bool Abstract;
        public List<DefDataObject> DefDataObjects;
    }

    public static class DefLibrary
    {
        // Yes I know I had them as ConcurrentBags.
        // Yes I know they are better then using locks in a threaded context.
        // However....
        // It seems that ConcurrentBags are causing a SIGBUS crash when the assembly size gets too large.
        // The weird part but after much hair pulling but, the crash seems to happen in the VTable and not the actual data type.
        // Not completely sure what is happening and not gonna debug it got stuff to do and locked lists are good enough.
        // And people still tell me that C and C++ are bad atleast in those I have address sanitizers and leak detection to debug this stuff.
        // 
        // ------------------------------------
        // TLDR: Concurrent types cause crashes do not change unless the root cause has been found and fixed.
        static List<Def>                         s_sceneDefs;
        static List<Def>                         s_defs;

        static ConcurrentDictionary<string, Def> s_sceneLookup;
        static ConcurrentDictionary<string, Def> s_defLookup;

        static void DefError(Type a_type, DefDataObject a_datObj, DefData a_data)
        {
            Logger.IcarianError($"Cannot Parse Def {a_type.ToString()}: {a_datObj.Name}, {a_data.Name} : {a_data.Path}");
        }

        static DefDataObject? GetData(XmlElement a_element)
        {
            DefDataObject dataObj;
            dataObj.Name = a_element.Name;
            dataObj.Text = a_element.InnerText;
            dataObj.Children = new List<DefDataObject>();

            foreach (XmlNode node in a_element.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    DefDataObject? data = GetData(element);
                    if (data != null)
                    {
                        dataObj.Children.Add(data.Value);
                    }
                    else
                    {
                        return null;
                    }
                }
            }

            if (string.IsNullOrWhiteSpace(dataObj.Text) && dataObj.Children.Count <= 0)
            {
                Logger.IcarianError($"Invalid Def DataObject: {dataObj.Name}");

                return null;
            }

            return dataObj;
        }

        static void LoadDefVariables(object a_obj, DefDataObject a_datObj, DefData a_data)
        {
            Type type = a_obj.GetType();

            FieldInfo field = type.GetField(a_datObj.Name, BindingFlags.Public | BindingFlags.Instance);
            if (field != null)
            {
                object obj = field.GetValue(a_obj);

                switch (obj)
                {
                case Type _:
                {
                    Type val = ModControl.GetTypeValue(a_datObj.Text);

                    if (val != null)
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(Type), a_datObj, a_data);
                    }

                    break;
                }
                case byte val:
                {
                    if (byte.TryParse(a_datObj.Text, out val))
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(byte), a_datObj, a_data);
                    }

                    break;
                }
                case short val:
                {
                    if (short.TryParse(a_datObj.Text, out val))
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(short), a_datObj, a_data);
                    }

                    break;
                }
                case ushort val:
                {
                    if (ushort.TryParse(a_datObj.Text, out val))
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(ushort), a_datObj, a_data);
                    }

                    break;
                }
                case uint val:
                {
                    if (uint.TryParse(a_datObj.Text, out val))
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(uint), a_datObj, a_data);
                    }

                    break;
                }
                case int val:
                {
                    if (int.TryParse(a_datObj.Text, out val))
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(int), a_datObj, a_data);
                    }

                    break;
                }
                case float val:
                {
                    if (float.TryParse(a_datObj.Text, out val))
                    {
                        field.SetValue(a_obj, val);
                    }
                    else
                    {
                        DefError(typeof(float), a_datObj, a_data);
                    }

                    break;
                }
                default:
                {
                    Type fieldType = field.FieldType;

                    if (fieldType == typeof(Def) || fieldType.IsSubclassOf(typeof(Def)))
                    {
                        Def def = Activator.CreateInstance(fieldType) as Def;
                        def.DefName = a_datObj.Text;

                        field.SetValue(a_obj, def);
                    }
                    else if (fieldType == typeof(string))
                    {
                        field.SetValue(a_obj, a_datObj.Text);
                    }
                    else if (fieldType.IsSubclassOf(typeof(Enum)))
                    {
                        int val;
                        if (int.TryParse(a_datObj.Text, out val))
                        {
                            if (Enum.IsDefined(fieldType, val))
                            {
                                field.SetValue(a_obj, val);
                            }
                            else
                            {
                                DefError(fieldType, a_datObj, a_data);
                            }
                        }   
                        else
                        {
                            object eVal = Enum.Parse(fieldType, a_datObj.Text);
                            if (eVal != null)
                            {
                                field.SetValue(a_obj, eVal);
                            }
                            else
                            {
                                DefError(fieldType, a_datObj, a_data);
                            }
                        }
                    }
                    else if (fieldType.IsArray)
                    {
                        Type elementType = fieldType.GetElementType();

                        int count = a_datObj.Children.Count;
                        Array a = Array.CreateInstance(elementType, a_datObj.Children.Count);

                        if (elementType == typeof(Def) || elementType.IsSubclassOf(typeof(Def)))
                        {
                            for (int i = 0; i < count; ++i)
                            {
                                DefDataObject dataObj = a_datObj.Children[i];

                                if (dataObj.Name == "lv")
                                {
                                    Def def = Activator.CreateInstance(elementType) as Def;
                                    def.DefName = dataObj.Text;

                                    a.SetValue(def, i);
                                }
                                else
                                {
                                    DefError(fieldType, a_datObj, a_data);
                                }
                            }
                        }
                        else
                        {
                            for (int i = 0; i < count; ++i)
                            {
                                DefDataObject dataObj = a_datObj.Children[i];
                                object arrayObj = Activator.CreateInstance(elementType);
                                if (dataObj.Name == "lv")
                                {
                                    foreach (DefDataObject objVal in dataObj.Children)
                                    {
                                        LoadDefVariables(arrayObj, objVal, a_data);
                                    }
                                }
                                else
                                {
                                    DefError(fieldType, a_datObj, a_data);
                                }

                                a.SetValue(arrayObj, i);
                            }
                        }

                        field.SetValue(a_obj, a);
                    }
                    else if (fieldType.IsGenericType && fieldType.GetGenericTypeDefinition() == typeof(List<>))
                    {
                        Type genericType = fieldType.GetGenericArguments()[0];
                        MethodInfo methodInfo = fieldType.GetMethod("Add");

                        if (obj == null)
                        {
                            obj = Activator.CreateInstance(fieldType);
                        } 

                        if (genericType == typeof(Def) || genericType.IsSubclassOf(typeof(Def)))
                        {
                            foreach (DefDataObject datObj in a_datObj.Children)
                            {
                                if (datObj.Name == "lv")
                                {
                                    Def def = Activator.CreateInstance(genericType) as Def;
                                    def.DefName = datObj.Text;

                                    methodInfo.Invoke(obj, new object[] { def });
                                }
                                else
                                {
                                    DefError(fieldType, a_datObj, a_data);
                                }
                            }
                        }
                        else
                        {
                            foreach (DefDataObject datObj in a_datObj.Children)
                            {
                                if (datObj.Name == "lv")
                                {
                                    object listObj = Activator.CreateInstance(genericType);

                                    foreach (DefDataObject objVal in datObj.Children)
                                    {
                                        LoadDefVariables(listObj, objVal, a_data);
                                    }

                                    methodInfo.Invoke(obj, new object[] { listObj });
                                }
                                else
                                {
                                    DefError(fieldType, a_datObj, a_data);
                                }
                            }
                        }

                        field.SetValue(a_obj, obj);
                    }
                    else
                    {
                        // If there is already an object do not want to overwrite it.
                        if (obj == null)
                        {
                            obj = Activator.CreateInstance(fieldType);
                        }

                        foreach (DefDataObject objVal in a_datObj.Children)
                        {
                            LoadDefVariables(obj, objVal, a_data);
                        }

                        field.SetValue(a_obj, obj);
                    }

                    break;
                }
                }
                
            }
            else
            {
                Logger.IcarianError($"Invalid Def Field: {a_datObj.Name}, {a_data.Name} : {a_data.Parent}");
            }
        }

        internal static DefData GetDefData(string a_path, XmlElement a_root)
        {
            DefData data;
            data.Type = a_root.Name;
            data.Path = a_path;
            data.Name = string.Empty;
            data.Parent = string.Empty;
            data.Abstract = false;
            data.DefDataObjects = new List<DefDataObject>();

            foreach (XmlAttribute att in a_root.Attributes)
            {
                switch (att.Name)
                {
                case "Name":
                {
                    data.Name = att.Value;

                    break;
                }
                case "Parent":
                {
                    data.Parent = att.Value;

                    break;
                }
                case "Abstract":
                {
                    bool val;
                    if (bool.TryParse(att.Value, out val))
                    {
                        data.Abstract = val;
                    }
                    else
                    {
                        Logger.IcarianError($"Error parsing Abstract value: {att.Value} : {a_path}");
                    }

                    break;
                }
                default:
                {
                    Logger.IcarianError($"Invalid Def Attribute: {att.Name} : {a_path}");

                    break;
                }
                }
            }

            foreach (XmlNode node in a_root.ChildNodes)
            {
                if (node is XmlElement element)
                {
                    DefDataObject? dataObject = GetData(element);
                    if (dataObject != null)
                    {
                        data.DefDataObjects.Add(dataObject.Value);
                    }
                }
            }

            return data;
        }

        static void LoadDefData(string a_path, ref List<DefData> a_data)
        {
            string[] files = Directory.GetFiles(a_path);

            foreach (string file in files)
            {
                FileInfo info = new FileInfo(file);

                if (info.Extension == ".def")
                {
                    XmlDocument doc = new XmlDocument();
                    doc.Load(file);

                    if (doc.DocumentElement is XmlElement root)
                    {
                        DefData data = GetDefData(file, root);

                        if (!string.IsNullOrWhiteSpace(data.Name))
                        {
                            a_data.Add(data);
                        }
                        else
                        {
                            Logger.IcarianError($"Error parsing unamed Def: {a_path}");
                        }
                    }
                }
            }

            string[] dirs = Directory.GetDirectories(a_path);
            foreach (string dir in dirs)
            {
                LoadDefData(dir, ref a_data);
            }
        }

        static bool SetDefData(Def a_def, DefData a_data, IEnumerable<DefData> a_dataList)
        {
            if (!string.IsNullOrWhiteSpace(a_data.Parent))
            {
                bool found = false;
                foreach (DefData dat in a_dataList)
                {
                    if (dat.Name == a_data.Parent)
                    {
                        if (!SetDefData(a_def, dat, a_dataList))
                        {
                            return false;
                        }

                        found = true;

                        break;
                    }
                }

                if (!found)
                {
                    Logger.IcarianError($"Cannot find def parent: {a_data.Parent}, {a_data.Name} : {a_data.Path}");

                    return false;
                }
            }

            a_def.DefName = a_data.Name;

            foreach (DefDataObject obj in a_data.DefDataObjects)
            {
                LoadDefVariables(a_def, obj, a_data);
            }
            
            return true;
        }

        static Def CreateDef(DefData a_data, IEnumerable<DefData> a_dataList)
        {
            Type type = ModControl.GetTypeValue(a_data.Type, true);

            if (type != null)
            {
                Def defObj = null;
                if (s_defLookup.ContainsKey(a_data.Name))
                {
                    defObj = s_defLookup[a_data.Name];
                }
                else
                {
                    defObj = Activator.CreateInstance(type) as Def;
                }

                if (defObj != null)
                {
                    if (!SetDefData(defObj, a_data, a_dataList))
                    {
                        return null;
                    }

                    defObj.DefPath = a_data.Path;
                    defObj.DefParentName = a_data.Parent;

                    return defObj;
                }
                else
                {   
                    Logger.IcarianError($"Error creating Def: {a_data.Type}, {a_data.Name} : {a_data.Path}");
                }
            }
            else
            {
                Logger.IcarianError($"Invalid Def Type: {a_data.Type}, {a_data.Name} : {a_data.Path}");
            }

            return null;
        }

        internal static void Init()
        {
            s_defs = new List<Def>();
            s_defLookup = new ConcurrentDictionary<string, Def>();
            s_sceneDefs = new List<Def>();
            s_sceneLookup = new ConcurrentDictionary<string, Def>();
        }

        public static void Clear()
        {
            FlushSceneDefs();

            lock (s_defs)
            {
                s_defs.Clear();
            }
            s_defLookup.Clear();
        }

        static void LoadDefs(IEnumerable<DefData> a_data)
        {
            foreach (DefData dat in a_data)
            {
                if (dat.Abstract)
                {
                    continue;
                }

                Def def = CreateDef(dat, a_data);
                if (def != null)
                {
                    lock (s_defs)
                    {
                        s_defs.Add(def);
                    }
                    s_defLookup.TryAdd(def.DefName, def);
                }
                else
                {
                    Logger.IcarianError("Invalid def");
                }
            }
        }
        internal static void LoadSceneDefs(IEnumerable<DefData> a_data)
        {
            foreach (DefData dat in a_data)
            {
                if (dat.Abstract)
                {
                    continue;
                }

                Def def = CreateDef(dat, a_data);
                if (def != null)
                {
                    lock (s_sceneDefs)
                    {
                        s_sceneDefs.Add(def);
                    }
                    s_sceneLookup.TryAdd(def.DefName, def);
                }
                else
                {
                    Logger.IcarianError("Invalid scene def");
                }
            }
        }

        public static void FlushSceneDefs()
        {
            lock (s_sceneDefs)
            {
                s_sceneDefs.Clear();
            }
            s_sceneLookup.Clear();
        }

        public static void LoadDefs(string a_path)
        {
            if (Directory.Exists(a_path))
            {
                Logger.IcarianMessage("Loading Defs");

                List<DefData> defData = new List<DefData>();
                LoadDefData(a_path, ref defData);

                Logger.IcarianMessage("Building DefTable");

                LoadDefs(defData);
            }
        }

        static void LoadDefs(byte[][] a_data, string[] a_paths)
        {
            uint defCount = (uint)a_data.LongLength;

            List<DefData> defData = new List<DefData>();

            for (uint i = 0; i < defCount; ++i)
            {
                string path = a_paths[i];

                MemoryStream stream = new MemoryStream(a_data[i]);

                XmlDocument doc = new XmlDocument();
                doc.Load(stream);

                if (doc.DocumentElement is XmlElement root)
                {
                    DefData data = GetDefData(path, root);

                    if (!string.IsNullOrWhiteSpace(data.Name))
                    {
                        defData.Add(data);
                    }
                    else
                    {
                        Logger.Error($"IcarianCS: Error parsing unamed Def: {path}");
                    }
                }
            }   

            foreach(DefData dat in defData)
            {
                if (dat.Abstract)
                {
                    continue;
                }

                Def def = CreateDef(dat, defData);
                if (def != null)
                {
                    lock (s_defs)
                    {
                        s_defs.Add(def);
                    }
                    s_defLookup.TryAdd(def.DefName, def);
                }
            }
        }   

        static void ResolveDefs(object a_obj)
        {
            if (a_obj == null)
            {
                Logger.IcarianWarning("ResolveDefs: Null Object");

                return;
            }

            Type type = a_obj.GetType();
            FieldInfo[] fieldInfo = type.GetFields(BindingFlags.Public | BindingFlags.Instance);
            foreach (FieldInfo info in fieldInfo)
            {
                Type fieldType = info.FieldType;

                if (fieldType.IsPrimitive || fieldType == typeof(string) || fieldType == typeof(decimal) || fieldType.IsSubclassOf(typeof(Enum)) || fieldType == typeof(Vector2) || fieldType == typeof(Vector3) || fieldType == typeof(Vector4))
                {
                    continue;
                }
                else if (fieldType == typeof(Def) || fieldType.IsSubclassOf(typeof(Def)))
                {
                    Def stub = (Def)info.GetValue(a_obj);

                    if (stub != null)
                    {
                        Def resDef = GetDef(stub.DefName);

                        if (resDef != null)
                        {
                            info.SetValue(a_obj, resDef);
                        }
                        else
                        {
                            Logger.IcarianError($"Error resolving Def: {stub.DefName}");
                        }
                    }
                }
                else if (fieldType.IsGenericType && fieldType.GetGenericTypeDefinition() == typeof(List<>))
                {
                    object listObj = info.GetValue(a_obj);
                    if (listObj != null)
                    {
                        IEnumerable enumer = (IEnumerable)listObj;

                        Type genericType = fieldType.GetGenericArguments()[0];
                        if (genericType.IsSubclassOf(typeof(Def)))
                        {
                            MethodInfo methodInfo = fieldType.GetMethod("Add");
                            object list = Activator.CreateInstance(fieldType);

                            foreach (object obj in enumer)
                            {
                                if (obj is Def stub)
                                {
                                    if (string.IsNullOrEmpty(stub.DefName))
                                    {
                                        Logger.IcarianError("Invalid def stub");

                                        continue;
                                    }

                                    methodInfo.Invoke(list, new object[] { GetDef(stub.DefName) });
                                }
                            }

                            info.SetValue(a_obj, list);
                        }
                        else
                        {
                            foreach (object obj in enumer)
                            {
                                ResolveDefs(obj);
                            }
                        }
                    }
                }
                else
                {
                    object val = info.GetValue(a_obj);

                    if (val != null)
                    {
                        ResolveDefs(val);
                    }
                }
            }
        }

        static void RefreshDefTables(IcarianAssembly a_asm)
        {
            foreach (Assembly asm in a_asm.Assemblies)
            {
                foreach (Type t in asm.GetTypes())
                {
                    if (t.GetCustomAttribute<DefTableAttribute>() != null)
                    {
                        FieldInfo[] fields = t.GetFields(BindingFlags.Public | BindingFlags.Static);
                        foreach (FieldInfo f in fields)
                        {
                            Def d = GetDef(f.Name);

                            if (d != null)
                            {
                                f.SetValue(null, d);
                            }
                            else
                            {
                                Logger.IcarianError("DefTable Invalid Def");
                            }
                        }
                    }
                }
            }
        }

        public static void ResolveDefs()
        {
            List<Def> defs = new List<Def>();

            lock (s_defs)
            {
                defs.AddRange(s_defs);
            }
            
            foreach (Def def in defs)
            {
                ResolveDefs(def);
            }

            foreach (Def def in defs)
            {
                def.PostResolve();
            }

            if (!Application.IsEditor)
            {
                RefreshDefTables(ModControl.CoreAssembly);

                foreach (IcarianAssembly fasm in ModControl.Assemblies)
                {
                    RefreshDefTables(fasm);
                }
            }
        }
        public static void ResolveSceneDefs()
        {
            List<Def> defs = new List<Def>();

            lock (s_sceneDefs)
            {
                defs.AddRange(s_sceneDefs);
            }   

            foreach (Def def in defs)
            {
                ResolveDefs(def);
            }

            foreach (Def def in defs)
            {
                def.PostResolve();
            }
        }

        public static IEnumerable<T> GetDefs<T>() where T : Def
        {
            List<T> defs = new List<T>();
            
            lock (s_sceneDefs)
            {
                foreach (Def sDef in s_sceneDefs)
                {
                    if (sDef is T t)
                    {
                        defs.Add(t);
                    }
                }
            }
            
            lock (s_defs)
            {
                foreach (Def def in s_defs)
                {
                    if (def is T t)
                    {
                        defs.Add(t);
                    }
                }
            }
            
            return defs;
        }
        public static IEnumerable<Def> GetDefs()
        {
            List<Def> defs = new List<Def>();
            lock (s_defs)
            {
                defs.AddRange(s_defs);
            }
            lock (s_sceneDefs)
            {
                defs.AddRange(s_sceneDefs);
            }

            return defs;
        }

        public static Def GetDef(string a_name)
        {
            if (s_sceneLookup.ContainsKey(a_name))
            {
                return s_sceneLookup[a_name];
            }

            if (s_defLookup.ContainsKey(a_name))
            {
                return s_defLookup[a_name];
            }

            Logger.IcarianWarning($"Cannot find def of name: {a_name}");

            return null;
        }
        public static T GetDef<T>(string a_name) where T : Def
        {
            if (s_sceneLookup.ContainsKey(a_name))
            {
                T def = s_sceneLookup[a_name] as T;
                if (def != null)
                {
                    return def;
                }
            }

            if (s_defLookup.ContainsKey(a_name))
            {
                T def = s_defLookup[a_name] as T;
                if (def != null)
                {
                    return def;
                }
            }

            Logger.IcarianWarning($"Cannot find def of name and type: {a_name}, {typeof(T).ToString()}");

            return null;
        }
    };
}