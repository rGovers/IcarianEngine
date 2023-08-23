using System;
using System.Collections.Generic;
using IcarianEngine.Maths;

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

    public class AnimationClip
    {
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

        public AnimationClip(string a_name, float a_duration)
        {
            m_name = a_name;
            m_duration = a_duration;

            m_fields = new List<AnimationField>();
        }

        public static AnimationClip LoadAnimationClip(string a_path)
        {
            // TODO: Need to implement this

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
            if (GetQuaternionSlerp(a_object, "Transform", "Rotation", a_time, out val))
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
                            AnimationKey prevKey = field.Keys[i];

                            float t = (a_time - prevKey.Time) / (key.Time - prevKey.Time);

                            Quaternion prevValue = (Quaternion)prevKey.Value;
                            Quaternion nextValue = (Quaternion)key.Value;

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