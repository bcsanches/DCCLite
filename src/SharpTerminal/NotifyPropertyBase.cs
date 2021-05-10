using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading;

//based on https://github.com/Microsoft/dotnet-apiport/blob/dev/src/ApiPort/ApiPort.VisualStudio.Common/NotifyPropertyBase.cs
namespace SharpTerminal
{
    public class NotifyPropertyBase : INotifyPropertyChanged
    {
        public virtual event PropertyChangedEventHandler PropertyChanged;
        protected SynchronizationContext Context { get; }

        public NotifyPropertyBase()
        {
            // SynchronizationContext.Current will be null if we're not on the UI thread
            Context = SynchronizationContext.Current;// ?? throw new ArgumentNullException("SynchronizationContext.Current");
        }

        protected void UpdateNonNullableProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = "")
        {
            if(value == null)
            {
                throw new ArgumentNullException(propertyName);
            }

            UpdateProperty(ref field, value, propertyName);            
        }

        protected void ForcePropertyUpdate<T>(ref T field, T value, [CallerMemberName] string propertyName = "")
        {
            field = value;

            OnPropertyUpdated(propertyName);
        }

        protected void UpdateProperty<T>(ref T field, T value, [CallerMemberName] string propertyName = "")
        {
            if (!EqualityComparer<T>.Default.Equals(field, value))
            {
                ForcePropertyUpdate(ref field, value, propertyName);
            }
        }

        protected void OnPropertyUpdated([CallerMemberName]string propertyName = "")
        {
            var propertyChanged = PropertyChanged;
            if (propertyChanged == null)
            {
                return;
            }

            if (Context == SynchronizationContext.Current)
            {
                propertyChanged.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }
            else
            {
                Context.Post(_ => propertyChanged.Invoke(this, new PropertyChangedEventArgs(propertyName)), null);
            }
        }
    }
}
