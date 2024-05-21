using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

#include "InteropTypes.h"

// Quick 20min adventure I thought as the legwork was done in C++
// 1 week of debugging later...
// I now know more about UNIX signals and harware interrupts than I ever wanted to know
// Fuck you C# hiding implementation details is not fun
// I am once again reminded why I had the rule that if there are issues just go lower level
// I now know how to do it properly now but it has sucked everything out of me debugging so
// this is the code now
// Now to scream in a pillow for a few hours and forget everything I learned
// C# is a "safe" and "portable" language btw
//
// Atleast there is a silver lining in that some other bugs got caught in the crossfire
// 
// DO NOT TOUCH UNLESS YOU KNOW WHAT YOU ARE DOING
// Will get crashes on random lines all over the application for example
// int brk = 3;
// TLDR: Do not put blind trust in C# threading types they are only reliable in pure C#

namespace IcarianEngine
{
    /// <summary>
    /// JobPriority enumeration
    /// </summary>
    public enum JobPriority : uint
    {
        High = 4,
        Medium = 2,
        Low = 0
    }

    public interface IThreadJob
    {
        /// <summary>
        /// Called when the job is being executed
        /// </summary>
        void Execute();
    }

    public static class ThreadPool
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void AddJob(uint a_job, uint a_priority);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetThreadCount();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetQueueSize();

        class ThreadJobFunc : IThreadJob
        {
            ThreadJobCallback m_callback;

            public ThreadJobFunc(ThreadJobCallback a_callback)
            {
                m_callback = a_callback;
            }

            public void Execute()
            {
                m_callback();
            }
        }

        static List<IThreadJob> s_jobs;
        static NativeLock s_lock;

        /// <summary>
        /// The number of threads in the ThreadPool
        /// </summary>
        public static uint ThreadCount
        {
            get
            {
                return GetThreadCount();
            }
        }
        /// <summary>
        /// The number of queued jobs in the ThreadPool
        /// </summary>
        public static uint QueuedJobCount
        {
            get
            {
                return GetQueueSize();
            }
        }

        internal static void Init()
        {
            s_jobs = new List<IThreadJob>();
            s_lock = new NativeLock();
        }
        internal static void Destroy()
        {
            s_jobs.Clear();
            s_lock.Dispose();
        }

        /// <summary>
        /// Delegate for ThreadPool jobs
        /// </summary>
        public delegate void ThreadJobCallback();

        static void Dispatch(uint a_addr)
        {
            IThreadJob job = null;

            s_lock.ReadLock();

            try
            {
                job = s_jobs[(int)a_addr];
                s_jobs[(int)a_addr] = null;
            }
            finally
            {
                s_lock.ReadUnlock();
            }
            
            if (job != null)
            {
                job.Execute();

                if (job is IDisposable disp)
                {
                    disp.Dispose();
                }   
            }
        }

        static uint PushJobList(IThreadJob a_job)
        {
            s_lock.WriteLock();

            try
            {
                int count = s_jobs.Count;

                for (int i = 0; i < count; ++i)
                {
                    if (s_jobs[i] == null)
                    {
                        s_jobs[i] = a_job;

                        return (uint)i;
                    }
                }

                s_jobs.Add(a_job);

                return (uint)count;
            }
            finally
            {
                s_lock.WriteUnlock();
            }

            // Warnings where pissing me off as it is harmless 
            // The warning itself is harmful as if the block above was wrapped in a conditional block can throw a compiler error
            // Need to use macro to pass the macro through to the C# preprocessor as currently in the C preprocessor
IOP_CSMACRO(pragma warning disable CS0162)

            return 0;

IOP_CSMACRO(pragma warning restore CS0162)
        }

        /// <summary>
        /// Pushes a job to the ThreadPool
        /// </summary>
        /// <param name="a_job">The job to execute</param>
        /// <param name="a_priority">The priority in which to execute the job</param>
        public static void PushJob(IThreadJob a_job, JobPriority a_priority = JobPriority.Medium)
        {
            uint index = PushJobList(a_job);

            AddJob(index, (uint)a_priority);
        }
        /// <summary>
        /// Pushes a job to the ThreadPool
        /// </summary>
        /// <param name="a_callback">Job to execute as a delegate</param>
        /// <param name="a_priority">The priority in which to execute the job</param>
        public static void PushJob(ThreadJobCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            PushJob(new ThreadJobFunc(a_callback), a_priority);
        }
    }
}