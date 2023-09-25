using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Json;
using System.Linq;
using System.Management.Instrumentation;
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

    public interface IRemoteObjectAction
    {
        public string GetLabel();
        public string GetDescription();

        public void Execute(IConsole console, RemoteObject target);
    }

    public class RemoteObject: NotifyPropertyBase
    {
        [Category("Object")]
        public string Name { get; }

        [Category("Object")]
        public string ClassName { get; }

        [Category("Object")]
        public string Path { get; }

        [Category("Object")]
        public ulong InternalId { get; }

        [Category("Object")]
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

        protected void FireStateChangedEvent()
        {
            if (StateChanged != null)
            {
                var args = new EventArgs();
                StateChanged(this, args);
            }
        }

        public void UpdateState(JsonValue def)
        {
            this.OnUpdateState(def);

            this.FireStateChangedEvent();
        }

        protected virtual void OnUpdateState(JsonValue def)
        {
            //empty
        }

        public virtual string TryGetIconName()
        {
            return null;
        }

        public virtual Control CreateControl(IConsole console)
        {
            return new RemoteObjectUserControl(console, this);
        }

        public virtual IRemoteObjectAction[] GetActions()
        {
            return null;
        }

        public virtual string GetNameSuffix()
        {
            return "";
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
        public string TargetName { get; }

        private readonly System.Threading.SynchronizationContext mSyncContext;
        private Task<RemoteObject> mTargetLoaderTask;
        private RemoteObject mTarget;

        public RemoteShortcut(string name, string className, string path, ulong internalId, ulong parentInternalId, string target):
            base(name, className, path, internalId, parentInternalId)
        {
            if (string.IsNullOrWhiteSpace(target))
                throw new ArgumentNullException(nameof(target));

            TargetName = target;

            mSyncContext = System.Threading.SynchronizationContext.Current;

            mTargetLoaderTask = RemoteObjectManager.GetRemoteObjectAsync(target);            
            mTargetLoaderTask.ContinueWith((previous) =>
                {
                    lock (this)
                    {
                        mTarget = previous.Result;                        
                        mTargetLoaderTask = null;

                        mTarget.StateChanged += MTarget_StateChanged;
                        mTarget.PropertyChanged += MTarget_PropertyChanged;

                        if(mSyncContext != null)
                        {
                            mSyncContext.Post((object o) =>
                                {
                                    this.FireStateChangedEvent();
                                },
                                null
                            );
                        }
                        else
                        {
                            this.FireStateChangedEvent();
                        }
                    }
                }
            );
        }

        private void MTarget_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            this.FireStateChangedEvent();
        }

        private void MTarget_StateChanged(RemoteObject sender, EventArgs args)
        {
            this.FireStateChangedEvent();
        }

        public override Control CreateControl(IConsole console)
        {
            return new RemoteShortcutUserControl(console, this);
        }

        public Task<RemoteObject> GetTargetAsync()
        {
            lock(this)
            {
                if (mTargetLoaderTask != null)
                    return mTargetLoaderTask;

                return Task.FromResult(mTarget);
            }
        }

        public override string TryGetIconName()
        {            
            lock(this)
            {
                return mTarget?.TryGetIconName();                
            }
        }
    }    

    static class RemoteObjectManager
    {
        static Dictionary<ulong, RemoteObject> gObjects = new();
        static Dictionary<string, RemoteObject> gObjectsByPath = new();
        static Dictionary<string, Task<JsonValue>> gActiveTasks = new();

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
            lock(gObjects)
            {
                var remoteObject = TryGetCachedRemoteObjectFromRpc(parameters);
                if (remoteObject == null)
                    return;

                remoteObject.UpdateState(parameters);
            }
            
        }

        private static void HandleRpcItemDestroyed(JsonValue parameters)
        {
            lock (gObjects)
            {
                ulong id = parameters["internalId"];

                var remoteObject = TryGetCachedRemoteObjectFromRpc(parameters);
                if (remoteObject == null)
                    return;

                gObjects.Remove(id);
                gObjectsByPath.Remove(remoteObject.Path);

                if ((remoteObject.ParentInternalId > 0) && (gObjects.TryGetValue(remoteObject.ParentInternalId, out var parentObject)))
                {
                    var parentFolder = (RemoteFolder)parentObject;

                    parentFolder.OnChildDestroyed(remoteObject);
                }
            }

            
        }

        private static void HandleRpcItemCreated(JsonValue parameters)
        {
            ulong parentId = parameters["parentInternalId"];

            RemoteObject remoteParent;

            lock(gObjects)
            {
                if (!gObjects.TryGetValue(parentId, out remoteParent))
                    return;
            }
            
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
                case "dcclite::Broker":
                    obj = new RemoteRoot(name, className, path, id, parentInternalId);
                    break;

                case "dcclite::Shortcut":
                    obj = new RemoteShortcut(name, className, path, id, parentInternalId, objectDef["target"]);
                    break;

                case "Decoder":
                case "TurntableAutoInverterDecoder":
                    obj = new RemoteDecoder(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "Dispatcher":
                    obj = new Dispatcher.RemoteDispatcher(name, className, path, id, parentInternalId);
                    break;

                case "Dispatcher::Section":
                case "Dispatcher::TSection":
                    obj = new Dispatcher.RemoteSection(name, className, path, id, parentInternalId, objectDef);
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

                case "NetworkDevice":
                    obj = new RemoteDevice(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "SensorDecoder":
                case "VirtualSensorDecoder":
                    obj = new RemoteSensorDecoder(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "ServoTurnoutDecoder":
                    obj = new RemoteServoTurnoutDecoder(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "SignalDecoder":
                    obj = new RemoteSignalDecoder(name, className, path, id, parentInternalId, objectDef);
                    break;

                case "SimpleOutputDecoder":
                case "OutputDecoder":
                case "QuadInverterDecoder":
                    obj = new RemoteOutputDecoder(name, className, path, id, parentInternalId, objectDef);
                    break;                

                case "TurnoutDecoder":
                case "VirtualTurnoutDecoder":
                    obj = new RemoteTurnoutDecoder(name, className, path, id, parentInternalId, objectDef);
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
            Task<JsonValue> currentTask;

            lock (gObjects)
            {
                if (gObjectsByPath.TryGetValue(path, out RemoteObject obj))
                {
                    return obj;
                }

                if(!gActiveTasks.TryGetValue(path, out currentTask))
                {
                    currentTask = mRequestManager.RequestAsync(new string[] { "Get-Item", path });

                    gActiveTasks.Add(path, currentTask);
                }
            }

            var response = await currentTask;

            var responseObj = response["item"];

            lock(gObjects)
            {
                gActiveTasks.Remove(path);

                //Perhaps other thread registered it?
                if (gObjectsByPath.TryGetValue(path, out RemoteObject obj))
                {
                    obj.UpdateState(responseObj);

                    return obj;
                }                

                return RegisterObject(responseObj);
            }            
        }

        internal static RemoteObject LoadObject(JsonValue objectDef)
        {
            ulong id = objectDef["internalId"];

            lock(gObjects)
            {
                if (gObjects.TryGetValue(id, out RemoteObject obj))
                {
                    obj.UpdateState(objectDef);
                }
                else
                {
                    //we may have a Task in progress loading the same object, but we ignore this case
                    //The task data will be handled later or discarded
                    obj = RegisterObject(objectDef);
                }

                return obj;
            }            
        }
    }
}
