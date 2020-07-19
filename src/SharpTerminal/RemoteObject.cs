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
        public string Name { get; }
        readonly string mClassName;
        public string Path { get; }             

        readonly int mInternalId;        

        public RemoteObject(string name, string className, string path, int internalId)
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
        }

        public virtual void UpdateState(JsonValue def)
        {
            //nothing to do
        }

        public override int GetHashCode()
        {
            return mInternalId;
        }        
    }

    public class RemoteFolder : RemoteObject
    {        
        private Dictionary<string, RemoteObject> mChildren;

        public RemoteFolder(string name, string className, string path, int internalId) :
            base(name, className, path, internalId)
        {
            //empty
        }

        public async Task<IEnumerable<RemoteObject>> LoadChildrenAsync(RequestManager requestManager)
        {            
            if (mChildren == null)
            {
                var response = await requestManager.RequestAsync(new string[] { "Get-ChildItem", this.Path });
                var children = (JsonArray)response["children"];

                if (children.Count == 0)
                    return null;

                mChildren = new Dictionary<string, RemoteObject>();

                foreach (var item in children)
                {                    
                    var obj = RemoteObjectManager.LoadObject(item);
                    mChildren.Add(obj.Name, obj);
                }
            }

            return mChildren.Select(x => x.Value);
        }        
    }

    public class RemoteShortcut: RemoteObject
    {
        readonly string mTarget;

        public RemoteShortcut(string name, string className, string path, int internalId, string target):
            base(name, className, path, internalId)
        {
            if (string.IsNullOrWhiteSpace(target))
                throw new ArgumentNullException(nameof(target));

            mTarget = target;
        }
    }    

    static class RemoteObjectManager
    {
        static Dictionary<int, RemoteObject> gObjects = new Dictionary<int, RemoteObject>();
        static Dictionary<string, RemoteObject> gObjectsByPath = new Dictionary<string, RemoteObject>();

        private static RemoteObject RegisterObject(JsonValue objectDef)
        {
            int id = objectDef["internalId"];
            string className = objectDef["className"];
            string name = objectDef["name"];
            string path = objectDef["path"];            

            RemoteObject obj;
            switch (className)
            {
                case "dcclite::Shortcut":
                    obj = new RemoteShortcut(name, className, path, id, objectDef["target"]);
                    break;

                case "Device":
                    obj = new RemoteDevice(name, className, path, id);
                    break;

                default:
                    obj = objectDef["isFolder"] ? new RemoteFolder(name, className, path, id) : new RemoteObject(name, className, path, id);
                    break;
            }

            gObjects.Add(id, obj);
            gObjectsByPath.Add(obj.Path, obj);            

            return obj;
        }

        internal static async Task<RemoteObject> GetRemoteObjectAsync(string path, RequestManager requestManager)
        {
            if(gObjectsByPath.TryGetValue(path, out RemoteObject obj))
            {
                return obj;
            }

            var response = await requestManager.RequestAsync(new string[] { "Get-Item", path });

            var responseObj = response["item"];

            return RegisterObject(responseObj);
        }

        internal static RemoteObject LoadObject(JsonValue objectDef)
        {
            int id = (int)objectDef["internalId"];

            RemoteObject obj;
            if (gObjects.TryGetValue(id, out obj))
            {
                obj.UpdateState(objectDef);
            }
            else
            {
                obj = RegisterObject(objectDef);
            }

            return obj;
        }
    }
}
