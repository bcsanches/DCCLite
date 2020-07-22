using System;
using System.Collections.Generic;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
    public delegate void RemoteObjectStateChanged(RemoteObject sender, EventArgs args);    

    public class RemoteObject
    {        
        public string Name { get; }
        readonly string mClassName;
        public string Path { get; }

        public ulong InternalId { get; }

        public ulong ParentInternalId { get; }

        public event RemoteObjectStateChanged StateChanged;

        public RemoteObject(string name, string className, string path, ulong internalId, ulong parentInternalId)
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

            InternalId = internalId;
        }

        public void UpdateState(JsonValue def)
        {
            this.OnUpdateState(def);

            if (StateChanged != null)
            {
                var args = new EventArgs();
                StateChanged(this, args);
            }
        }

        protected virtual void OnUpdateState(JsonValue def)
        {
            //empty
        }

        public virtual string TryGetIconName()
        {
            return null;
        }
    }

    public class RemoteFolder : RemoteObject
    {        
        private Dictionary<string, RemoteObject> mChildren;

        public RemoteFolder(string name, string className, string path, ulong internalId, ulong parentInternalId) :
            base(name, className, path, internalId, parentInternalId)
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
        
        internal void OnChildDestroyed(RemoteObject child)
        {
            //if children not cached, nothing to do...
            if (mChildren == null)
                return;

            mChildren.Remove(child.Name);
            if(mChildren.Count() == 0)
            {
                mChildren = null;
            }
        }
    }

    public class RemoteShortcut: RemoteObject
    {
        readonly string mTarget;

        public RemoteShortcut(string name, string className, string path, ulong internalId, ulong parentInternalId, string target):
            base(name, className, path, internalId, parentInternalId)
        {
            if (string.IsNullOrWhiteSpace(target))
                throw new ArgumentNullException(nameof(target));

            mTarget = target;
        }
    }    

    static class RemoteObjectManager
    {
        static Dictionary<ulong, RemoteObject> gObjects = new Dictionary<ulong, RemoteObject>();
        static Dictionary<string, RemoteObject> gObjectsByPath = new Dictionary<string, RemoteObject>();

        private static RequestManager mRequestManager;        

        public static void SetRequestManager(RequestManager requestManager)
        {
            if(mRequestManager != null)
            {
                mRequestManager.RpcNotificationArrived -= MRequestManager_RpcNotificationArrived;
            }

            mRequestManager = requestManager;

            if (mRequestManager == null)
                throw new ArgumentNullException(nameof(requestManager));

            mRequestManager.RpcNotificationArrived += MRequestManager_RpcNotificationArrived;
        }

        private static RemoteObject TryGetCachedRemoteObjectFromRpc(JsonValue parameters)
        {
            ulong id = parameters["internalId"];

            //object not cached?
            gObjects.TryGetValue(id, out RemoteObject remoteObject);                

            return remoteObject;
        }

        private static void HandleRpcItemPropertyValueChanged(JsonValue parameters)
        {
            var remoteObject = TryGetCachedRemoteObjectFromRpc(parameters);
            if (remoteObject == null)
                return;

            remoteObject.UpdateState(parameters);
        }

        private static void HandleRpcItemDestroyed(JsonValue parameters)
        {
            ulong id = parameters["internalId"];

            var remoteObject = TryGetCachedRemoteObjectFromRpc(parameters);
            if (remoteObject == null)
                return;

            gObjects.Remove(id);
            gObjectsByPath.Remove(remoteObject.Path);

            if((remoteObject.ParentInternalId > 0) && (gObjects.TryGetValue(remoteObject.ParentInternalId, out var parentObject)))
            {
                var parentFolder = (RemoteFolder)parentObject;

                parentFolder.OnChildDestroyed(remoteObject);
            }
        }

        private static void MRequestManager_RpcNotificationArrived(RequestManager sender, RpcNotificationEventArgs args)
        {
            var json = args.Notification;

            var parameters = json["params"];            

            switch ((string)json["method"])
            {
                case "On-ItemCreated":
                    break;

                case "On-ItemDestroyed":
                    HandleRpcItemDestroyed(parameters);
                    break;

                case "On-ItemPropertyValueChanged":
                    HandleRpcItemPropertyValueChanged(parameters);
                    break;
            }                       
        }

        private static RemoteObject RegisterObject(JsonValue objectDef)
        {
            ulong id = objectDef["internalId"];
            string className = objectDef["className"];
            string name = objectDef["name"];
            string path = objectDef["path"];
            ulong parentInternalId = objectDef.ContainsKey("parentInternalId") ? (ulong)objectDef["parentInternalId"] : 0;

            RemoteObject obj;
            switch (className)
            {
                case "dcclite::Shortcut":
                    obj = new RemoteShortcut(name, className, path, id, parentInternalId, objectDef["target"]);
                    break;

                case "Device":
                    obj = new RemoteDevice(name, className, path, id, parentInternalId, objectDef);
                    break;

                default:
                    obj = objectDef["isFolder"] ? new RemoteFolder(name, className, path, id, parentInternalId) : new RemoteObject(name, className, path, id, parentInternalId);
                    break;
            }

            gObjects.Add(id, obj);
            gObjectsByPath.Add(obj.Path, obj);            

            return obj;
        }

        internal static async Task<RemoteObject> GetRemoteObjectAsync(string path)
        {
            if(gObjectsByPath.TryGetValue(path, out RemoteObject obj))
            {
                return obj;
            }

            var response = await mRequestManager.RequestAsync(new string[] { "Get-Item", path });

            var responseObj = response["item"];

            return RegisterObject(responseObj);
        }

        internal static RemoteObject LoadObject(JsonValue objectDef)
        {
            ulong id = objectDef["internalId"];

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
