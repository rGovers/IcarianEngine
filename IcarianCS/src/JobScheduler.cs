using System.Collections.Generic;

namespace IcarianEngine
{
    public abstract class SchedulerJob
    {
        double m_time;

        /// <summary>
        /// The time to execute the job
        /// </summary>
        public double Time
        {
            get
            {
                return m_time;
            }
        }

        protected SchedulerJob(double a_time) 
        {
            m_time = a_time;
        }

        /// <summary>
        /// Called when the job is being executed
        /// </summary>
        public abstract void Execute();
    }

    public static class JobScheduler
    {
        /// <summary>
        /// Delegate for JobScheduler jobs
        /// </summary>
        public delegate void JobSchedulerCallback();

        /// @cond INTERNAL

        class SchedulerJobFunc : SchedulerJob
        {
            JobSchedulerCallback m_callback;

            public SchedulerJobFunc(JobSchedulerCallback a_callback, double a_time) : base(a_time)
            {
                m_callback = a_callback;
            }

            public override void Execute()
            {
                m_callback();
            }
        }

        /// @endcond

        static List<SchedulerJob> s_jobs;

        internal static void Init()
        {
            s_jobs = new List<SchedulerJob>();
        }

        internal static void Update()
        {
            double time = Time.TimePassed;

            lock (s_jobs)
            {
                while (true)
                {
                    SchedulerJob job = null;

                    uint count = (uint)s_jobs.Count;
                    for (uint i = 0; i < count; ++i)
                    {
                        SchedulerJob j = s_jobs[(int)i];
                        if (j != null && time >= j.Time)
                        {
                            job = j;
                            s_jobs[(int)i] = null;

                            break;
                        }
                    }

                    if (job == null)
                    {
                        break;
                    }

                    job.Execute();
                }
            }
        }

        internal static void Destroy()
        {
            lock (s_jobs)
            {
                foreach (SchedulerJob j in s_jobs)
                {
                    if (j != null)
                    {
                        j.Execute();
                    }
                }

                s_jobs.Clear();
                s_jobs = null;
            }
        }

        /// <summary>
        /// Pushes a job to the JobScheduler
        /// </summary>
        /// <param name="a_job">The job to execute</param>
        public static void PushJob(SchedulerJob a_job)
        {
            lock (s_jobs)
            {
                uint count = (uint)s_jobs.Count;
                for (uint i = 0; i < count; ++i)
                {
                    if (s_jobs[(int)i] == null)
                    {
                        s_jobs[(int)i] = a_job;

                        return;
                    }
                }

                s_jobs.Add(a_job);
            }
        }

        /// <summary>
        /// Pushes a job to the JobScheduler
        /// </summary>
        /// <param name="a_callback">The job to execute as a delegate</param>  
        /// <param name="a_timeOffset">How long from now to execute the job</param>      
        public static void PushJob(JobSchedulerCallback a_callback, double a_timeOffset)
        {
            PushJob(new SchedulerJobFunc(a_callback, Time.TimePassed + a_timeOffset));
        }
    }
}