using System;
using System.Collections.Generic;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
    public class RemoteFolderChildEventArgs: EventArgs
    {
        public RemoteObject Target { get; }

        public RemoteFolderChildEventArgs(RemoteObject target)
        {
            Target = target ?? throw new ArgumentNullException(nameof(target));
        }
    }

    public delegate void RemoteObjectStateChanged(RemoteObject sender, EventArgs args);
    public delegate void RemoteFolderChildEvent(RemoteObject sender, RemoteFolderChildEventArgs args);

    public class RemoteObject: NotifyPropertyBase
    {        
        public string Name { get; }
        public string ClassName { get; }
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
            ClassName = className;
            Path = path;

            InternalId = internalId;
            ParentInternalId = parentInternalId;
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

        public virtual Control CreateControl()
        {
            return null;
        }
    }

    public class RemoteFolder : RemoteObject
    {        
        private Dictionary<string, RemoteObject> mChildren;

        public event RemoteFolderChildEvent ChildAdded;
        public event RemoteFolderChildEvent ChildRemoved;

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
            
            if(ChildRemoved != null)
            {
                var args = new RemoteFolderChildEventArgs(child);
                ChildRemoved(this, args);
            }
        }

        internal void OnChildCreated(JsonValue data)
        {
            //Are we tracking children?
            if (mChildren == null)
            {
                //No, so just ignore
                return;
            }

            var obj = RemoteObjectManager.LoadObject(data);
            mChildren.Add(obj.Name, obj);

            if (ChildAdded != null)
            {
                var args = new RemoteFolderChildEventArgs(obj);
                ChildAdded(this, args);
            }
        }
    }

    public class RemoteShortcut: RemoteObject
    {
        public string Target { get; }

        public RemoteShortcut(string name, string className, string path, ulong internalId, ulong parentInternalId, string target):
            base(name, className, path, internalId, parentInternalId)
        {
            if (string.IsNullOrWhiteSpace(target))
                throw new ArgumentNullException(nameof(target));

            Target = target;
        }
        public override Control CreateControl()
        {
            return new RemoteShortcutUserControl(this);
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

        private static void HandleRpcItemCreated(JsonValue parameters)
        {
            ulong parentId = parameters["parentInternalId"];

            if (!gObjects.TryGetValue(parentId, out RemoteObject remoteParent))
                return;

            var folder = (RemoteFolder)remoteParent;

            folder.OnChildCreated(parameters);
        }

        private static void MRequestManager_RpcNotificationArrived(RequestManager sender, RpcNotificationEventArgs args)
        {
            var json = args.Notification;

            var parameters = json["params"];            

            switch ((string)json["method"])
            {
                case "On-ItemCreated":
                    HandleRpcItemCreated(parameters);
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

                case "Decoder":
                case "OutputDecoder":
                case "SimpleOutputDecoder":
                case "SensorDecoder":
                case "ServoTurnoutDecoder":
                case "TurnoutDecoder":
                    obj = new RemoteDecoder(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "SignalDecoder":
                    obj = new RemoteSignalDecoder(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "NetworkDevice":                
                    obj = new RemoteDevice(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "Location":
                    obj = new RemoteLocation(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "LocationManager":
                    obj = new RemoteLocationManager(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "LoconetService":
                    obj = new RemoteLoconetService(name, className, path, id, parentInternalId, objectDef);
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

            if (gObjects.TryGetValue(id, out RemoteObject obj))
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
