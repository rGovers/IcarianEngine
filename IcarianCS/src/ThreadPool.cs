using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Threading;

namespace IcarianEngine
{
    public enum JobPriority : uint
    {
        High = 4,
        Medium = 2,
        Low = 0
    }

    public static class ThreadPool
    {
        // Naive approach but it works
        // Some concurrent types seem to cause crashes in C# for some reason often with sigbus errors
        // But C# is supposedly a "safe" language
        static List<ThreadJob> s_jobs = new List<ThreadJob>();
        static ReaderWriterLock s_lock = new ReaderWriterLock();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void AddJob(uint a_job, uint a_priority);

        static void Dispatch(uint a_addr)
        {
            ThreadJob job = null;

            s_lock.AcquireReaderLock(-1);

            job = s_jobs[(int)a_addr];
            s_jobs[(int)a_addr] = null;

            s_lock.ReleaseReaderLock();

            job.Execute();
        }

        static uint PushJobList(ThreadJob a_job)
        {
            s_lock.AcquireWriterLock(-1);

            int count = s_jobs.Count;
            for (int i = 0; i < count; ++i)
            {
                if (s_jobs[i] == null)
                {
                    s_jobs[i] = a_job;

                    s_lock.ReleaseWriterLock();

                    return (uint)i;
                }
            }

            s_jobs.Add(a_job);

            s_lock.ReleaseWriterLock();

            return (uint)count;
        }

        public static void PushJob(ThreadJob a_job, JobPriority a_priority)
        {
            uint index = PushJobList(a_job);

            AddJob(index, (uint)a_priority);
        }
    }
}