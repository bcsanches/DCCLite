using System;
using System.Collections.Generic;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{   
    public class RemoteObject
    {
        [Flags]
        public enum Flags
        {
            None = 0,            
            Folder = 1
        }

        public string Name { get; }
        readonly string mClassName;
        public string Path { get; }
        public bool IsFolder { get; }        

        readonly int mInternalId;        

        public RemoteObject(string name, string className, string path, int internalId, Flags flags)
        {
            if (string.IsNullOrWhiteSpace(name))
                throw new ArgumentNullException(nameof(name));

            if (string.IsNullOrWhiteSpace(className))
                throw new ArgumentNullException(nameof(className));

            if (string.IsNullOrWhiteSpace(path))
                throw new ArgumentNullException(nameof(path));

            Name = name;
            mClassName = className;
            Path = path;

            mInternalId = internalId;

            IsFolder = (flags & Flags.Folder) == Flags.Folder;
        }

        public void UpdateState(JsonValue def)
        {
            //nothing to do
        }

        public override int GetHashCode()
        {
            return mInternalId;
        }        
    }

    public class RemoteShortcut: RemoteObject
    {
        readonly string mTarget;

        public RemoteShortcut(string name, string className, string path, int internalId, RemoteObject.Flags flags, string target):
            base(name, className, path, internalId, flags)
        {
            if (string.IsNullOrWhiteSpace(target))
                throw new ArgumentNullException(nameof(target));

            mTarget = target;
        }
    }

    public static class RemoteObjectManager
    {
        static Dictionary<int, RemoteObject> gObjects = new Dictionary<int, RemoteObject>();

        public static RemoteObject TryLookup(JsonValue objectDef)
        {
            int id = (int)objectDef["internalId"];

            RemoteObject obj;
            if (gObjects.TryGetValue(id, out obj))
            {
                obj.UpdateState(objectDef);
            }
            else
            {
                string className = objectDef["className"];
                string name = objectDef["name"];
                string path = objectDef["path"];
                var flags = objectDef["isFolder"] ? RemoteObject.Flags.Folder : RemoteObject.Flags.None;

                switch (className)
                {
                    case "dcclite::Shortcut":
                        obj = new RemoteShortcut(name, className, path, id, flags, objectDef["target"]);
                        break;

                    default:
                        obj = new RemoteObject(name, className, path, id, flags);
                        break;
                }

                gObjects.Add(id, obj);
            }

            return obj;
        }
    }
}
