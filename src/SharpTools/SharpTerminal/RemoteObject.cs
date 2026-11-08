// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Json;
using System.Linq;
using System.Runtime.Versioning;
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

	[SupportedOSPlatform("windows")]
	public class RemoteObject: NotifyPropertyBase, IControlProvider
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

		public RemoteFolder Parent { get; }

		private RemoteObject(string name, string className, string path, ulong internalId, RemoteFolder parent, bool allowNullParent)
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
			ParentInternalId = allowNullParent ? 0 : parent.InternalId;
			Parent = parent;
		}

		public RemoteObject(string name, string className, string path, ulong internalId, RemoteFolder parent):
			this(name, className, path, internalId, parent, false)
		{
			//empty
		}

		/// <summary>
		/// To be used only by roots... HACK
		/// </summary>
		/// <param name="name"></param>
		/// <param name="className"></param>
		/// <param name="path"></param>
		/// <param name="internalId"></param>
		protected RemoteObject(string name, string className, string path, ulong internalId):
			this(name, className, path, internalId, null, true)
		{
			//empty
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

	[SupportedOSPlatform("windows")]
	public class RemoteFolder : RemoteObject
	{        
		private Dictionary<string, RemoteObject> mChildren;

		public event RemoteFolderChildEvent ChildAdded;
		public event RemoteFolderChildEvent ChildRemoved;

		private bool mChildLoaded = false;

		public RemoteFolder(string name, string className, string path, ulong internalId, RemoteFolder parent) :
			base(name, className, path, internalId, parent)
		{
			//empty
		}

		/// <summary>
		/// To be used only by roots... HACK
		/// </summary>
		/// <param name="name"></param>
		/// <param name="className"></param>
		/// <param name="path"></param>
		/// <param name="internalId"></param>
		protected RemoteFolder(string name, string className, string path, ulong internalId) :
			base(name, className, path, internalId)
		{
			//empty
		}

		protected RemoteObject LoadChild(JsonValue def)
		{
			var obj = RemoteObjectManager.LoadObject(def, this);
			mChildren.Add(obj.Name, obj);

			return obj;
		}

		public RemoteObject TryFindChildByName(string name)
		{			
			mChildren.TryGetValue(name, out RemoteObject remoteObject);

			return remoteObject;
		}

		public async Task<IEnumerable<RemoteObject>> LoadChildrenAsync(RequestManager requestManager)
		{            
			if (!mChildLoaded)
			{
				var response = await requestManager.RequestAsync(new string[] { "Get-ChildItem", this.Path });
				var children = (JsonArray)response["children"];

				mChildLoaded = true;

				if (children.Count == 0)
					return null;

				mChildren = [];

				foreach (var item in children)
				{
					this.LoadChild(item);
				}
			}

			return mChildren.Select(x => x.Value);
		}

		public RemoteObject[] GetChildren()
		{
			if (!mChildLoaded)
				throw new InvalidOperationException("Load children first");

			return mChildren != null ? mChildren.Values.ToArray() : null;
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

			var obj = RemoteObjectManager.LoadObject(data, this);
			mChildren.Add(obj.Name, obj);

			if (ChildAdded != null)
			{
				var args = new RemoteFolderChildEventArgs(obj);
				ChildAdded(this, args);
			}
		}
	}

	[SupportedOSPlatform("windows")]
	public class RemoteShortcut: RemoteObject
	{
		public string TargetName { get; }

		private readonly System.Threading.SynchronizationContext mSyncContext;
		private Task<RemoteObject> mTargetLoaderTask;
		private RemoteObject mTarget;

		public RemoteShortcut(string name, string className, string path, ulong internalId, string target, RemoteFolder parent):
			base(name, className, path, internalId, parent)
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
}
