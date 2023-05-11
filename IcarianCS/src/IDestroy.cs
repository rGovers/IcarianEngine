using System;

namespace IcarianEngine
{
    public interface IDestroy : IDisposable
    {
        bool IsDisposed
        {
            get;
        }
    }
}