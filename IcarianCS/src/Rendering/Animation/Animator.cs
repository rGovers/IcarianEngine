using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using IcarianEngine.Definitions;

namespace IcarianEngine.Rendering.Animation
{
    public enum AnimationUpdateMode : ushort
    {
        None = 0,
        // Main and physics thread
        Update = 0b1 << 0,
        // Frame thread
        FrameUpdate = 0b1 << 1,
        // Thread pool
        PooledUpdateLow = 0b1 << 2,
        PooledUpdateMedium = 0b1 << 3,
        PooledUpdateHigh = 0b1 << 4
    }

    public abstract class Animator : Component, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetUpdateMode(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetUpdateMode(uint a_addr, uint a_mode);

        static ConcurrentDictionary<uint, Animator> s_animators = new ConcurrentDictionary<uint, Animator>();

        uint                m_bufferAddr = uint.MaxValue;

        AnimationController m_controller;

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        public AnimatorDef AnimatorDef
        {
            get
            {
                return Def as AnimatorDef;
            }
        }

        public AnimationController AnimationController
        {
            get
            {
                return m_controller;
            }
        }

        public AnimationUpdateMode UpdateMode
        {
            get
            {
                return (AnimationUpdateMode)GetUpdateMode(m_bufferAddr);
            }
            set
            {
                SetUpdateMode(m_bufferAddr, (uint)value);
            }
        }

        public override void Init()
        {
            base.Init();

            m_bufferAddr = GenerateBuffer();

            AnimatorDef def = AnimatorDef;
            if (def != null)
            {
                if (def.ControllerDef != null && def.ControllerDef.ControllerType != null)
                {
                    m_controller = Activator.CreateInstance(def.ControllerDef.ControllerType) as AnimationController;
                }
            }

            s_animators.TryAdd(m_bufferAddr, this);
        }

        public abstract void Update(float a_deltaTime);

        // Yes I am passing as double and down casting to float 
        // for some reason if I have a float parameter it will fail to find the function
        // once again here I am questioning C# and it's weirdness
        // probably float is called something weird and not actually float is normally the way it goes
        static void UpdateAnimatorS(uint a_index, double a_deltaTime)
        {
            Animator animator;

            if (s_animators.TryGetValue(a_index, out animator))
            {
                if (animator != null && !animator.IsDisposed)
                {
                    animator.Update((float)a_deltaTime);
                }
            }
        }
        static void UpdateAnimatorsS(uint[] a_indices, double a_deltaTime)
        {
            Animator animator;

            foreach (uint index in a_indices)
            {
                if (s_animators.TryGetValue(index, out animator))
                {
                    if (animator != null && !animator.IsDisposed)
                    {
                        animator.Update((float)a_deltaTime);
                    }
                }
            }
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    s_animators.TryRemove(m_bufferAddr, out Animator _);

                    DestroyBuffer(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Animator Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Animator Dispose");
            }
        }
        ~Animator()
        {
            Dispose(false);
        }
    }
}