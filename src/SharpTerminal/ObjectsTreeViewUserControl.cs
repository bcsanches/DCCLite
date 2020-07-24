using System;
using System.Collections.Generic;
using System.ComponentModel.Design.Serialization;
using System.Json;
using System.Runtime.Serialization;
using System.Text;
using System.Windows.Forms;

namespace SharpTerminal
{
    
    public partial class ObjectsTreeViewUserControl : UserControl
    {
        RequestManager mRequestManager;
        readonly Dictionary<ulong, List<TreeNode>> mObjectsNodes = new Dictionary<ulong, List<TreeNode>>();

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

            var remoteObject = (RemoteObject)node.Tag;

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

        private void UpdateNodesIcon(RemoteObject remoteObject)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.UpdateNodesIcon(remoteObject); }));                
            }
            else
            {
                if (!mObjectsNodes.TryGetValue(remoteObject.InternalId, out var nodes))
                    return;

                var customIcon = remoteObject.TryGetIconName();

                foreach (var node in nodes)
                {
                    node.ImageKey = customIcon;
                    node.SelectedImageKey = customIcon;
                }
            }            
        }

        private void RemoteObject_StateChanged(RemoteObject sender, EventArgs args)
        {            
            if (sender.TryGetIconName() == null)
                return;                        

            UpdateNodesIcon(sender);                        
        }

        private async void mRequestManager_ConnectionStateChanged(RequestManager sender, ConnectionStateEventArgs args)
        {
            if(mTreeView.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.mRequestManager_ConnectionStateChanged(sender, args); }));
            }
            else
            {
                if (args.State == ConnectionState.OK)
                {
                    mTreeView.Nodes.Clear();

                    var rootFolder = (RemoteFolder) await RemoteObjectManager.GetRemoteObjectAsync("/");

                    var brokerNode = mTreeView.Nodes.Add("Broker");
                    brokerNode.Name = "Broker";
                    brokerNode.Tag = rootFolder;

                    var children = await rootFolder.LoadChildrenAsync(mRequestManager);
                    if (children != null)
                        FillTree(brokerNode, children);

                    var servicesFolder = (RemoteFolder)await RemoteObjectManager.GetRemoteObjectAsync("/services");
                    var services = await servicesFolder.LoadChildrenAsync(mRequestManager);
                    
                    foreach(var service in services)
                    {
                        if(service.Name == "locationManager")
                        {
                            var locationNode = mTreeView.Nodes.Add("locations");
                            locationNode.Tag = service;

                            RegisterNode(service, locationNode);
                        }
                    }
                }
                else if (args.State == ConnectionState.DISCONNECTED)
                {

                }
            }            
        }

        private void FillServices(JsonArray objects)
        {
            if (mTreeView.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.FillServices(objects); }));
            }
            else
            {
                mTreeView.SuspendLayout();
                try
                {
                    foreach (var item in objects)
                    {
                        var remoteObject = RemoteObjectManager.LoadObject(item);
                    }
                }
                finally
                {
                    mTreeView.ResumeLayout();
                }
            }
        }

        private void AddNewNode(RemoteObject remoteObject, TreeNode parent)
        {
            TreeNode newNode = parent.Nodes.Add(remoteObject.Name);
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

        private async void mTreeView_BeforeExpand(object sender, TreeViewCancelEventArgs e)
        {
            if((e.Node.Nodes.Count > 0) && (e.Node.Nodes[0].Tag == this))
            {
                var remoteFolder = (RemoteFolder)e.Node.Tag;                

                e.Node.Nodes.Clear();

                var children = await remoteFolder.LoadChildrenAsync(mRequestManager);
                if(children != null)
                    FillTree(e.Node, children);
                
                e.Cancel = true;
            }            
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

            if (!(e.Node.Tag is RemoteObject remoteObject))
                return;

            var newControl = remoteObject.CreateControl();
            if (newControl == null)
                return;

            MainDisplayPanel.Controls.Add(newControl);
            newControl.Dock = DockStyle.Fill;
        }
    }
}
