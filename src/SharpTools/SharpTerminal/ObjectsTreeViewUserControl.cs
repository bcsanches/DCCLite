using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Versioning;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public partial class ObjectsTreeViewUserControl : UserControl
    {
        RequestManager mRequestManager;
        IConsole mConsole;
        readonly Dictionary<ulong, List<TreeNode>> mObjectsNodes = new Dictionary<ulong, List<TreeNode>>();

        RemoteObject mPreviousSelectedObject;

        public Panel MainDisplayPanel
        {
            get;
            set;
        }
                
        internal RequestManager RequestManager
        {
            set
            {
                if (value == mRequestManager)
                    return;

                if(mRequestManager != null)
                {
                    mRequestManager.ConnectionStateChanged -= mRequestManager_ConnectionStateChanged;
                }

                mRequestManager = value;

                if(mRequestManager != null)
                {
                    mRequestManager.ConnectionStateChanged += mRequestManager_ConnectionStateChanged;
                }
            }
        }

        internal IConsole Console
        {
            set
            {
                mConsole = value;
            }
        }

        private void RegisterNode(RemoteObject obj, TreeNode node)
        {
            if (this.InvokeRequired)
                throw new InvalidOperationException("InvokeRequired");

            if (!mObjectsNodes.TryGetValue(obj.InternalId, out var nodes))
            {
                nodes = new List<TreeNode>();
                mObjectsNodes.Add(obj.InternalId, nodes);

                obj.StateChanged += RemoteObject_StateChanged;

                if (obj is RemoteFolder folder)
                {
                    folder.ChildAdded += RemoteFolder_ChildAdded;
                    folder.ChildRemoved += RemoteFolder_ChildRemoved;
                }
            }

            nodes.Add(node);            
        }

        private void DeleteTreeNodes_r(TreeNode node)
        {
            foreach(TreeNode subNode in node.Nodes)
            {
                DeleteTreeNodes_r(subNode);
            }

			if (node.Tag is not RemoteObject remoteObject)
				return;

			var nodes = mObjectsNodes[remoteObject.InternalId];

            nodes.Remove(node);

            if(nodes.Count == 0)
            {
                remoteObject.StateChanged -= RemoteObject_StateChanged;
                if (remoteObject is RemoteFolder folder)
                {
                    folder.ChildAdded -= RemoteFolder_ChildAdded;
                    folder.ChildRemoved -= RemoteFolder_ChildRemoved;
                }

                mObjectsNodes.Remove(remoteObject.InternalId);
            }
            
            node.Parent.Nodes.Remove(node);
        }

        private void RemoteFolder_ChildRemoved(RemoteObject sender, RemoteFolderChildEventArgs args)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.RemoteFolder_ChildRemoved(sender, args); }));

                return;
            }


            var removedItem = args.Target;

            if (!mObjectsNodes.TryGetValue(removedItem.InternalId, out var nodes))
            {
                return;
            }

            //copy it, as the DeleteTreeNodes_r will change it
            var nodesArray = nodes.ToArray();

            foreach (var node in nodesArray)
                DeleteTreeNodes_r(node);
        }

        private void RemoteFolder_ChildAdded(RemoteObject sender, RemoteFolderChildEventArgs args)
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.RemoteFolder_ChildAdded(sender, args); }));
            }
            else
            {
                var addedItem = args.Target;

                if (!mObjectsNodes.TryGetValue(addedItem.ParentInternalId, out var nodes))
                    return;

                foreach(var node in nodes)
                {
                    AddNewNode(addedItem, node);
                }
            }
        }

        private void UpdateNodesData(RemoteObject remoteObject)
        {
            if (!mObjectsNodes.TryGetValue(remoteObject.InternalId, out var nodes))
                return;

            var customIcon = remoteObject.TryGetIconName();

            var name = GenerateObjectName(remoteObject);
            foreach (var node in nodes)
            {
                node.ImageKey = customIcon;
                node.SelectedImageKey = customIcon;

                node.Text = name;
            }            
        }

        private string GenerateObjectName(RemoteObject remoteObject)
        {
            return remoteObject.Name + remoteObject.GetNameSuffix();
        }

        private void RemoteObject_StateChanged(RemoteObject sender, EventArgs args)
        {            
            if (sender.TryGetIconName() == null)
                return;

            UpdateNodesData(sender);                        
        }

        private async void mRequestManager_ConnectionStateChanged(RequestManager sender, ConnectionStateEventArgs args)
        {            
            if (args.State == ConnectionState.OK)
            {
                mTreeView.Nodes.Clear();
                RemoteObjectManager.Clear();

				var rootFolder = (RemoteFolder) await RemoteObjectManager.GetRemoteObjectAsync("/");

                var brokerNode = mTreeView.Nodes.Add("Broker");
                brokerNode.Name = "Broker";
                brokerNode.Tag = rootFolder;

                var children = await rootFolder.LoadChildrenAsync(mRequestManager);
                if (children != null)
                    FillTree(brokerNode, children);

                var servicesFolder = (RemoteFolder)await RemoteObjectManager.GetRemoteObjectAsync("/services");
                var services = await servicesFolder.LoadChildrenAsync(mRequestManager);

                var locationService = services.Where(x => x.Name == "locationManager").FirstOrDefault();
                if (locationService != null)
                {                                        
                    var locationNode = mTreeView.Nodes.Add("locations");
                    locationNode.Tag = locationService;

                    RegisterNode(locationService, locationNode);                        
                }

                var emulatorNode = mTreeView.Nodes.Add("Emulator");
                emulatorNode.Name = "Emulator";

                if(mPreviousSelectedObject != null)
                {
                    //try to reload previous object
                    var path = mPreviousSelectedObject.Path;
                    var pathNodes = path.Split("/");
                    
                    TreeNode node = brokerNode;

                    //index 0 is a empty string (root)
                    for(int i = 1; i < pathNodes.Length; i++)
                    {
                        await TryToLoadNodeChildrenAsync(node);                        

						var result = node.Nodes.Find(pathNodes[i], false);
                        if ((result == null) || (result.Length == 0))
                            break;

						node = result[0];                        
					}

                    mTreeView.SelectedNode = node;
                }
            }
            else if ((args.State == ConnectionState.DISCONNECTED) || (args.State == ConnectionState.ERROR))
            {
                var node = mTreeView.SelectedNode;
                if (node == null)
                    return;

                var remoteObj = (RemoteObject)node.Tag;
                if (remoteObj == null)                
                    return;

                mPreviousSelectedObject = remoteObj;
			}                        
        }        

        private void AddNewNode(RemoteObject remoteObject, TreeNode parent)
        {
            TreeNode newNode = parent.Nodes.Add(GenerateObjectName(remoteObject));
            newNode.Name = newNode.Text;
            newNode.Tag = remoteObject;

            var iconKey = remoteObject.TryGetIconName();

            if (iconKey != null)
            {
                newNode.ImageKey = iconKey;
                newNode.SelectedImageKey = iconKey;
            }

            if (remoteObject is RemoteFolder)
            {
                var subNode = newNode.Nodes.Add("dummy");
                subNode.Tag = this;
            }
            else if (iconKey == null)
            {
                newNode.ImageKey = DefaultIcons.FILE_GEAR_ICON;
                newNode.SelectedImageKey = DefaultIcons.FILE_GEAR_ICON;
            }

            RegisterNode(remoteObject, newNode);
        }

        private void FillTree(TreeNode node, System.Collections.Generic.IEnumerable<RemoteObject> objects)
        {           
            mTreeView.SuspendLayout();

            try
            {
                foreach (var remoteObject in objects)
                {
                    AddNewNode(remoteObject, node);                    
                }

                node.Expand();
            }
            finally
            {
                mTreeView.ResumeLayout();
            }                            
        }

        public ObjectsTreeViewUserControl()
        {
            InitializeComponent();

            DefaultIcons.LoadIcons(mImageList);
            mTreeView.ImageList = mImageList;            
        }

        private async Task<bool> TryToLoadNodeChildrenAsync(TreeNode node)
        {
			if ((node.Nodes.Count > 0) && (node.Nodes[0].Tag == this))
			{
				var remoteFolder = (RemoteFolder)node.Tag;

				node.Nodes.Clear();

				var children = await remoteFolder.LoadChildrenAsync(mRequestManager);
				if (children != null)
					FillTree(node, children);

                return true;
			}

            return false;
		}

        private async void mTreeView_BeforeExpand(object sender, TreeViewCancelEventArgs e)
        {
            e.Cancel = await TryToLoadNodeChildrenAsync(e.Node);			
        }

        private void mTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (MainDisplayPanel == null)
                return;

            if (MainDisplayPanel.Controls.Count > 0)
            {
                var control = MainDisplayPanel.Controls[0];
                MainDisplayPanel.Controls.Remove(control);

                control.Dispose();
            }

            if (e.Node.Tag is not RemoteObject remoteObject)
                return;

            var newControl = remoteObject.CreateControl(mConsole);            

            MainDisplayPanel.Controls.Add(newControl);
            newControl.Dock = DockStyle.Fill;
        }
    }
}
