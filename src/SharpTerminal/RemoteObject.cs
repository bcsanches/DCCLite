using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharpTerminal
{
    class RemoteObject
    {
        [Flags]
        enum Flags
        {
            None = 0,
            Shortcut = 1,
            Folder = 2
        }

        readonly string mClassName;
        readonly string mName;

        readonly bool mIsShortcut;
        readonly bool mIsFolder;

        RemoteObject(string name, string className, Flags flags)
        {
            if (string.IsNullOrWhiteSpace(name))
                throw new ArgumentNullException(nameof(name));

            if (string.IsNullOrWhiteSpace(className))
                throw new ArgumentNullException(nameof(className));

            mName = name;
            mClassName = className;

            mIsShortcut = (flags & Flags.Shortcut) == Flags.Shortcut;
            mIsFolder = (flags & Flags.Folder) == Flags.Folder;
        }
    }
}
