using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineAnimationClipInteropStructures.h"

namespace IcarianEngine.Rendering.Animation
{
    // Could do bezier curves down the line
    public struct AnimationKey
    {
        /// <summary>
        /// The time of the animation key.
        /// </summary>
        public float Time;
        /// <summary>
        /// The value of the animation key.
        /// </summary>
        public object Value;
    }
    public struct AnimationField
    {
        /// <summary>
        /// The object of the animation field.
        /// </summary>
        public string Object;
        /// <summary>
        /// The component of the animation field.
        /// </summary>
        public string Component;
        /// <summary>
        /// The field of the animation field.
        /// </summary>
        public string Field;
        /// <summary>
        /// The keys of the animation field.
        /// </summary>
        public List<AnimationKey> Keys;
    }
    
    public class AnimationClip
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static AnimationDataExternal[] LoadExternalAnimationData(string a_path);

        bool                 m_icarianClip;
        float                m_duration;

        List<AnimationField> m_fields;

        /// <summary>
        /// Whether the AnimationClip is an Engine or Imported clip
        /// </summary>
        public bool IcarianClip
        {
            get
            {
                return m_icarianClip;
            }
        }

        /// <summary>
        /// The duration of the AnimationClip
        /// </summary>
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

        /// <summary>
        /// The fields in the AnimationClip
        /// </summary>
        public IEnumerable<AnimationField> Fields
        {
            get
            {
                return m_fields;
            }
        }

        AnimationClip()
        {
            m_icarianClip = false;
            m_duration = 0.0f;

            m_fields = new List<AnimationField>();
        }
        AnimationClip(float a_duration)
        {
            m_icarianClip = true;
            m_duration = a_duration;

            m_fields = new List<AnimationField>();
        }

        /// <summary>
        /// Loads an AnimationClip from the specified path
        /// </summary>
        /// Supported file formats:
        ///    .dae
        ///    .fbx
        ///    .glb
        ///    .gltf
        /// <param name="a_path">The path to the AnimationClip</param>
        /// <returns>The animation clip. Null on failure</returns>
        public static AnimationClip LoadAnimationClip(string a_path)
        {
            string ext = Path.GetExtension(a_path);

            switch (ext)
            {
            case ".dae":
            case ".fbx":
            case ".glb":
            case ".gltf":
            {
                AnimationDataExternal[] data = LoadExternalAnimationData(a_path);
                if (data != null)
                {
                    AnimationClip clip = new AnimationClip();

                    foreach (AnimationDataExternal d in data)
                    {
                        AnimationField field = new AnimationField();
                        field.Object = d.Name;
                        field.Component = "Transform";
                        field.Field = d.Target;
                        field.Keys = new List<AnimationKey>();

                        foreach (AnimationFrameExternal f in d.Frames)
                        {
                            clip.m_duration = Mathf.Max(clip.m_duration, f.Time);

                            switch (d.Target)
                            {
                            case "Translation":
                            case "Scale":
                            {
                                field.Keys.Add(new AnimationKey()
                                {
                                    Time = f.Time,
                                    Value = new Vector3(f.Data.XYZ)
                                });

                                break;
                            }
                            case "Rotation":
                            {
                                field.Keys.Add(new AnimationKey()
                                {
                                    Time = f.Time,
                                    Value = f.Data.ToQuaternion()
                                });

                                break;
                            }
                            }
                        }

                        clip.m_fields.Add(field);
                    }

                    return clip;
                }

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

        /// <summary>
        /// Gets the translation of the specified object at the specified time.
        /// </summary>
        /// <param name="a_skeleton">The skeleton to get the translation from.</param>
        /// <param name="a_object">The object to get the translation from.</param>
        /// <param name="a_time">The time to get the translation at.</param>
        /// <returns>The translation of the specified object at the specified time.</returns>
        public Vector3 GetTranslation(Skeleton a_skeleton, string a_object, float a_time)
        {
            Vector3 val;
            if (GetVector3Lerp(a_object, "Transform", "Translation", a_time, out val))
            {
                return val;
            }

            return a_skeleton.GetLocalTranslation(a_object);
        }
        /// <summary>
        /// Gets the rotation of the specified object at the specified time.
        /// </summary>
        /// <param name="a_skeleton">The skeleton to get the rotation from.</param>
        /// <param name="a_object">The object to get the rotation from.</param>
        /// <param name="a_time">The time to get the rotation at.</param>
        /// <returns>The rotation of the specified object at the specified time.</returns>
        public Quaternion GetRotation(Skeleton a_skeleton, string a_object, float a_time)
        {
            Quaternion val;
            if (GetQuaternionLerp(a_object, "Transform", "Rotation", a_time, out val))
            {
                return val;
            }

            return a_skeleton.GetLocalRotation(a_object);
        }
        /// <summary>
        /// Gets the scale of the specified object at the specified time.
        /// </summary>
        /// <param name="a_object">The object to get the scale from.</param>
        /// <param name="a_time">The time to get the scale at.</param>
        /// <returns>The scale of the specified object at the specified time.</returns>
        public Vector3 GetScale(string a_object, float a_time)
        {
            Vector3 val;
            if (GetVector3Lerp(a_object, "Transform", "Scale", a_time, out val))
            {
                return val;
            }

            return Vector3.One;
        }

        /// <summary>
        /// Gets the transform of the specified object at the specified time.
        /// </summary>
        /// <param name="a_skeleton">The skeleton to get the transform from.</param>
        /// <param name="a_object">The object to get the transform from.</param>
        /// <param name="a_time">The time to get the transform at.</param>
        /// <returns>The transform of the specified object at the specified time.</returns>
        public Matrix4 GetTransform(Skeleton a_skeleton, string a_object, float a_time)
        {
            Vector3 pos = GetTranslation(a_skeleton, a_object, a_time);
            Quaternion rot = GetRotation(a_skeleton, a_object, a_time);
            Vector3 scale = GetScale(a_object, a_time);

            return Matrix4.FromTransform(pos, rot, scale);
        }

        /// <summary>
        /// Gets the animation key of the specified object, component and field at the specified time.
        /// </summary>
        /// <param name="a_object">The object to get the animation key from.</param>
        /// <param name="a_component">The component to get the animation key from.</param>
        /// <param name="a_field">The field to get the animation key from.</param>
        /// <param name="a_time">The time to get the animation key at.</param>
        /// <param name="a_key">The animation key.</param>
        /// <returns>Whether the animation key was found.</returns>
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

        /// <summary>
        /// Gets the animation value of the specified object, component and field at the specified time.
        /// </summary>
        /// <typeparam name="T">The type of the animation value.</typeparam>
        /// <param name="a_object">The object to get the animation value from.</param>
        /// <param name="a_component">The component to get the animation value from.</param>
        /// <param name="a_field">The field to get the animation value from.</param>
        /// <param name="a_time">The time to get the animation value at.</param>
        /// <param name="a_val">The animation value.</param>
        /// <returns>Whether the animation value was found.</returns>
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

        /// <summary>
        /// Gets the animation value of the specified object, component and field at the specified time.
        /// </summary>
        /// <param name="a_object">The object to get the animation value from.</param>
        /// <param name="a_component">The component to get the animation value from.</param>
        /// <param name="a_field">The field to get the animation value from.</param>
        /// <param name="a_time">The time to get the animation value at.</param>
        /// <param name="a_val">The animation value.</param>
        /// <returns>Whether the animation value was found.</returns>
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

        /// <summary>
        /// Gets the animation value of the specified object, component and field at the specified time.
        /// </summary>
        /// <param name="a_object">The object to get the animation value from.</param>
        /// <param name="a_component">The component to get the animation value from.</param>
        /// <param name="a_field">The field to get the animation value from.</param>
        /// <param name="a_time">The time to get the animation value at.</param>
        /// <param name="a_val">The animation value.</param>
        /// <returns>Whether the animation value was found.</returns>
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
        /// <summary>
        /// Gets the animation value of the specified object, component and field at the specified time.
        /// </summary>
        /// <param name="a_object">The object to get the animation value from.</param>
        /// <param name="a_component">The component to get the animation value from.</param>
        /// <param name="a_field">The field to get the animation value from.</param>
        /// <param name="a_time">The time to get the animation value at.</param>
        /// <param name="a_val">The animation value.</param>
        /// <returns>Whether the animation value was found.</returns>
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