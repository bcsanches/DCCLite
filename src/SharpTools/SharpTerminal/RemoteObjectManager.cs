using System;
using System.Collections.Generic;
using System.Json;
using System.Runtime.Versioning;
using System.Threading.Tasks;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	static class RemoteObjectManager
	{
		static Dictionary<ulong, RemoteObject> gObjects = new();
		static Dictionary<string, RemoteObject> gObjectsByPath = new();
		static Dictionary<string, Task<JsonValue>> gActiveTasks = new();

		private static RequestManager mRequestManager;

		public static void Clear()
		{
			gObjects.Clear();
			gObjectsByPath.Clear();
			gActiveTasks.Clear();
		}

		public static void SetRequestManager(RequestManager requestManager)
		{
			if (mRequestManager != null)
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
			lock (gObjects)
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

			lock (gObjects)
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

			if (json.ContainsKey("error"))
			{
				//something failed, ignore
				return;
			}

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

		private static ulong GetParentInternalId(JsonValue objectDef)
		{
			return objectDef.ContainsKey("parentInternalId") ? (ulong)objectDef["parentInternalId"] : 0;
		}

		private static RemoteObject CreateObject(JsonValue objectDef, RemoteFolder parent)
		{
			ulong id = objectDef["internalId"];
			string className = objectDef["className"];
			string name = objectDef["name"];
			string path = objectDef["path"];
			ulong parentInternalId = GetParentInternalId(objectDef);

			if ((className != "dcclite::Broker") && (parentInternalId != parent.InternalId))
				throw new ArgumentException("Invalid parent id for " + name + ", provided parent: " + parent.Name);

			RemoteObject obj;

			switch (className)
			{
				case "dcclite::Broker":
					obj = new RemoteRoot(name, className, path, id);
					break;

				case "dcclite::Shortcut":
					obj = new RemoteShortcut(name, className, path, id, objectDef["target"], parent);
					break;

				case "DccLiteService":
					obj = new RemoteDccLiteService(name, className, path, id, parent);
					break;

				case "Decoder":
				case "TurntableAutoInverterDecoder":
					obj = new RemoteDecoder(name, className, path, id, objectDef, parent);
					break;

				case "Dispatcher":
					obj = new Dispatcher.RemoteDispatcher(name, className, path, id, parent);
					break;

				case "Dispatcher::Section":
				case "Dispatcher::TSection":
					obj = new Dispatcher.RemoteSection(name, className, path, id, objectDef, parent);
					break;

				case "Location":
					obj = new RemoteLocation(name, className, path, id, objectDef, parent);
					break;

				case "LocationManager":
					obj = new RemoteLocationManager(name, className, path, id, objectDef, parent);
					break;

				case "LoconetService":
					obj = new RemoteLoconetService(name, className, path, id, objectDef, parent);
					break;

				case "NetworkDevice":
					obj = new RemoteNetworkDevice(name, className, path, id, objectDef, parent);
					break;

				case "SensorDecoder":
					obj = new RemoteSensorDecoder(name, className, path, id, objectDef, parent);
					break;

				case "VirtualSensorDecoder":
					obj = new VirtualSensorDecoder(name, className, path, id, objectDef, parent);
					break;

				case "ServoTurnoutDecoder":
					obj = new RemoteServoTurnoutDecoder(name, className, path, id, objectDef, parent);
					break;

				case "SignalDecoder":
					obj = new RemoteSignalDecoder(name, className, path, id, objectDef, parent);
					break;

				case "BlinkOutputDecoder":
				case "BlinkOutputMasterDecoder":
				case "SimpleOutputDecoder":
				case "OutputDecoder":
				case "QuadInverterDecoder":
					obj = new RemoteOutputDecoder(name, className, path, id, objectDef, parent);
					break;

				case "TycoonService":
					obj = new Tycoon.RemoteTycoonService(name, className, path, id, objectDef, parent);
					break;

				case "dcclite::broker::tycoon::Industry":
					obj = new Tycoon.RemoteIndustry(name, className, path, id, objectDef, parent);
					break;

				case "dcclite::broker::tycoon::Location":
					obj = new Tycoon.RemoteLocation(name, className, path, id, objectDef, parent);
					break;

				case "TurnoutDecoder":
				case "VirtualTurnoutDecoder":
					obj = new RemoteTurnoutDecoder(name, className, path, id, objectDef, parent);
					break;

				default:
					obj = objectDef["isFolder"] ? new RemoteFolder(name, className, path, id, parent) : new RemoteObject(name, className, path, id, parent);
					break;
			}

			gObjects.Add(id, obj);
			gObjectsByPath.Add(obj.Path, obj);

			return obj;
		}

		public static async Task<RemoteRoot> LoadRoot()
		{
			lock (gObjects)
			{
				if (gObjects.Count > 0)
					throw new InvalidOperationException("[RemoteObjectManager::LoadRoot] Root already loaded");
			}

			var currentTask = mRequestManager.RequestAsync(new string[] { "Get-Item", "/" });
			var response = await currentTask;
			var responseObj = response["item"];

			lock (gObjects)
			{
				var root = (RemoteRoot)CreateObject(responseObj, null);

				return root;
			}
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

				if (!gActiveTasks.TryGetValue(path, out currentTask))
				{
					currentTask = mRequestManager.RequestAsync(new string[] { "Get-Item", path });

					gActiveTasks.Add(path, currentTask);
				}
			}

			var response = await currentTask;

			var responseObj = response["item"];

			var parentId = GetParentInternalId(responseObj);
			RemoteFolder parentFolder = null;
			lock (gObjects)
			{
				if (gObjects.TryGetValue(parentId, out RemoteObject parent))
					parentFolder = (RemoteFolder)parent;
			}

			//recursive... weeeeee
			if (parentFolder == null)
			{
				path = path.Trim();

				//got load parent
				var index = path.LastIndexOf("/");
				if (index < 0)
					throw new ArgumentException("[GetRemoteObjectAsync] Cannot find parent on suplied path: " + path + "\nAre you trying to load root using this???");

				//remove current object name
				var parentPath = path.Substring(0, index);
				if (parentPath == "/")
					throw new ArgumentException("[GetRemoteObjectAsync] Parent load resulted on root, did you forget to call LoadRoot? Path: " + path);

				parentFolder = (RemoteFolder)await GetRemoteObjectAsync(parentPath);
			}

			lock (gObjects)
			{
				gActiveTasks.Remove(path);

				//Perhaps other thread registered it?
				if (gObjectsByPath.TryGetValue(path, out RemoteObject obj))
				{
					obj.UpdateState(responseObj);

					return obj;
				}

				return CreateObject(responseObj, parentFolder);
			}
		}

		internal static RemoteObject LoadObject(JsonValue objectDef, RemoteFolder parent)
		{
			ulong id = objectDef["internalId"];

			lock (gObjects)
			{
				if (gObjects.TryGetValue(id, out RemoteObject obj))
				{
					obj.UpdateState(objectDef);
				}
				else
				{
					//we may have a Task in progress loading the same object, but we ignore this case
					//The task data will be handled later or discarded
					obj = CreateObject(objectDef, parent);
				}

				return obj;
			}
		}
	}
}
