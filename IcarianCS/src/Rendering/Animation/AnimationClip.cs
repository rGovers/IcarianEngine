using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Rendering.Animation
{
    // Could do bezier curves down the line
    public struct AnimationKey
    {
        public float Time;
        public object Value;
    }
    public struct AnimationField
    {
        public string Object;
        public string Component;
        public string Field;
        public List<AnimationKey> Keys;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    struct AnimationFrame
    {
        public float Time;
        public float[] Transform;
    }
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    struct AnimationData
    {
        public string Name;
        public AnimationFrame[] Frames;
    };

    public class AnimationClip
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static AnimationData[] LoadColladaAnimation(string a_path);

        string               m_name;
        float                m_duration;

        List<AnimationField> m_fields;

        public string Name
        {
            get
            {
                return m_name;
            }
        }

        public float Duration
        {
            get
            {
                return m_duration;
            }
            set
            {
                m_duration = value;
            }
        }

        public IEnumerable<AnimationField> Fields
        {
            get
            {
                return m_fields;
            }
        }

        AnimationClip()
        {
            m_name = "";
            m_duration = 0.0f;

            m_fields = new List<AnimationField>();
        }
        public AnimationClip(string a_name, float a_duration)
        {
            m_name = a_name;
            m_duration = a_duration;

            m_fields = new List<AnimationField>();
        }

        public static AnimationClip LoadAnimationClip(string a_path)
        {
            string ext = Path.GetExtension(a_path);

            switch (ext)
            {
            case ".dae":
            {
                // Collada is a transform based animation format so we need to convert it to a native field based format
                AnimationData[] data = LoadColladaAnimation(a_path);
                if (data != null)
                {
                    AnimationClip clip = new AnimationClip();
                    clip.m_name = Path.GetFileNameWithoutExtension(a_path);

                    foreach (AnimationData animObj in data)
                    {
                        string name = animObj.Name;

                        AnimationField translationField = new AnimationField()
                        {
                            Object = name,
                            Component = "Transform",
                            Field = "Translation",
                            Keys = new List<AnimationKey>()
                        };

                        AnimationField rotationField = new AnimationField()
                        {
                            Object = name,
                            Component = "Transform",
                            Field = "Rotation",
                            Keys = new List<AnimationKey>()
                        };

                        AnimationField scaleField = new AnimationField()
                        {
                            Object = name,
                            Component = "Transform",
                            Field = "Scale",
                            Keys = new List<AnimationKey>()
                        };

                        foreach (AnimationFrame frame in animObj.Frames)
                        {
                            float time = frame.Time;
                            float[] transform = frame.Transform;

                            Matrix4 mat = new Matrix4(transform[0],  transform[1],  transform[2],  transform[3],
                                                      transform[4],  transform[5],  transform[6],  transform[7],
                                                      transform[8],  transform[9],  transform[10], transform[11],
                                                      transform[12], transform[13], transform[14], transform[15]);

                            Vector3 translation;
                            Quaternion rotation;
                            Vector3 scale;
                            Matrix4.Decompose(mat, out translation, out rotation, out scale);

                            translationField.Keys.Add(new AnimationKey()
                            {
                                Time = time,
                                Value = translation
                            });

                            rotationField.Keys.Add(new AnimationKey()
                            {
                                Time = time,
                                Value = rotation
                            });

                            scaleField.Keys.Add(new AnimationKey()
                            {
                                Time = time,
                                Value = scale
                            });

                            if (time > clip.m_duration)
                            {
                                clip.m_duration = time;
                            }
                        }

                        clip.m_fields.Add(translationField);
                        clip.m_fields.Add(rotationField);
                        clip.m_fields.Add(scaleField);
                    }

                    return clip;
                }

                Logger.IcarianError($"Failed to load animation clip: {a_path}");

                break;
            }
            case ".ianim":
            {
                // TODO: Need to implement this
                Logger.IcarianError($"Not implemented yet");

                break;
            }
            default:
            {
                Logger.IcarianError($"Invalid file extension for animation clip: {a_path}");

                break;
            }
            }

            return null;
        }

        public Vector3 GetTranslation(string a_object, float a_time)
        {
            Vector3 val;
            if (GetVector3Lerp(a_object, "Transform", "Translation", a_time, out val))
            {
                return val;
            }

            return Vector3.Zero;
        }
        public Quaternion GetRotation(string a_object, float a_time)
        {
            Quaternion val;
            if (GetQuaternionLerp(a_object, "Transform", "Rotation", a_time, out val))
            {
                return val;
            }

            return Quaternion.Identity;
        }
        public Vector3 GetScale(string a_object, float a_time)
        {
            Vector3 val;
            if (GetVector3Lerp(a_object, "Transform", "Scale", a_time, out val))
            {
                return val;
            }

            return Vector3.One;
        }

        public Matrix4 GetTransform(string a_object, float a_time)
        {
            Vector3 pos = GetTranslation(a_object, a_time);
            Quaternion rot = GetRotation(a_object, a_time);
            Vector3 scale = GetScale(a_object, a_time);

            return Matrix4.FromTransform(pos, rot, scale);
        }

        public bool GetAnimationKey(string a_object, string a_component, string a_field, float a_time, out AnimationKey a_key)
        {
            a_key = new AnimationKey();

            foreach (AnimationField field in m_fields)
            {
                if (field.Object == a_object && field.Component == a_component && field.Field == a_field)
                {
                    int count = field.Keys.Count;

                    for (int i = 1; i < count; ++i)
                    {
                        AnimationKey key = field.Keys[i];
                        if (key.Time > a_time)
                        {
                            a_key = key;

                            return true;
                        }
                    }
                }
            }

            return false;
        }

        public bool GetAnimationValue<T>(string a_object, string a_component, string a_field, float a_time, out T a_val)
        {
            a_val = Activator.CreateInstance<T>();

            AnimationKey key;
            if (GetAnimationKey(a_object, a_component, a_field, a_time, out key))
            {
                if (key.Value is T val)
                {
                    a_val = val;

                    return true;
                }
            }

            return false;
        }

        public bool GetVector3Lerp(string a_object, string a_component, string a_field, float a_time, out Vector3 a_val)
        {
            a_val = Vector3.Zero;

            foreach (AnimationField field in m_fields)
            {
                if (field.Object == a_object && field.Component == a_component && field.Field == a_field)
                {
                    int count = field.Keys.Count;

                    for (int i = 1; i < count; ++i)
                    {
                        AnimationKey key = field.Keys[i];
                        if (key.Time > a_time)
                        {
                            AnimationKey prevKey = field.Keys[i - 1];

                            float t = (a_time - prevKey.Time) / (key.Time - prevKey.Time);

                            Vector3 prevValue = (Vector3)prevKey.Value;
                            Vector3 nextValue = (Vector3)key.Value;

                            a_val = Vector3.Lerp(prevValue, nextValue, t);

                            return true;
                        }
                    }

                    if (count > 0)
                    {
                        a_val = (Vector3)field.Keys[count - 1].Value;

                        return true;
                    }
                    else
                    {
                        // Not sure what to do as the field exists but there is no value 
                        // I think returning false makes sense but may change down the line
                        // Could potentially switch to enum
                        return false;
                    }
                }
            }

            return false;
        }

        public bool GetQuaternionLerp(string a_object, string a_component, string a_field, float a_time, out Quaternion a_val)
        {
            a_val = Quaternion.Identity;

            foreach (AnimationField field in m_fields)
            {
                if (field.Object == a_object && field.Component == a_component && field.Field == a_field)
                {
                    int count = field.Keys.Count;

                    for (int i = 1; i < count; ++i)
                    {
                        AnimationKey key = field.Keys[i];
                        if (key.Time > a_time)
                        {
                            AnimationKey prevKey = field.Keys[i - 1];

                            float t = (a_time - prevKey.Time) / (key.Time - prevKey.Time);

                            Quaternion prevValue = (Quaternion)prevKey.Value;
                            Quaternion nextValue = (Quaternion)key.Value;

                            a_val = Quaternion.Lerp(prevValue, nextValue, t);

                            return true;
                        }
                    }

                    if (count > 0)
                    {
                        a_val = (Quaternion)field.Keys[count - 1].Value;

                        return true;
                    }
                    else
                    {
                        // Not sure what to do as the field exists but there is no value 
                        // I think returning false makes sense but may change down the line
                        // Could potentially switch to enum
                        return false;
                    }
                }
            }

            return false;
        }
        public bool GetQuaternionSlerp(string a_object, string a_component, string a_field, float a_time, out Quaternion a_val)
        {
            a_val = Quaternion.Identity;

            foreach (AnimationField field in m_fields)
            {
                if (field.Object == a_object && field.Component == a_component && field.Field == a_field)
                {
                    int count = field.Keys.Count;

                    for (int i = 1; i < count; ++i)
                    {
                        AnimationKey key = field.Keys[i];
                        if (key.Time > a_time)
                        {
                            AnimationKey prevKey = field.Keys[i - 1];

                            float t = (a_time - prevKey.Time) / (key.Time - prevKey.Time);

                            Quaternion prevValue = (Quaternion)prevKey.Value;
                            Quaternion nextValue = (Quaternion)key.Value;

                            // TODO: Fix slerp
                            a_val = Quaternion.Slerp(prevValue, nextValue, t);

                            return true;
                        }
                    }

                    if (count > 0)
                    {
                        a_val = (Quaternion)field.Keys[count - 1].Value;

                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }

            return false;
        }
    }
}