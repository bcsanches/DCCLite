using System;
using System.Collections;
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

                    var rootFolder = (RemoteFolder) await RemoteObjectManager.GetRemoteObjectAsync("/", mRequestManager);

                    var brokerNode = mTreeView.Nodes.Add("Broker");
                    brokerNode.Name = "Broker";
                    brokerNode.Tag = rootFolder;

                    var children = await rootFolder.LoadChildrenAsync(mRequestManager);
                    if (children != null)
                        FillTree(brokerNode, children);

                    var servicesFolder = (RemoteFolder)await RemoteObjectManager.GetRemoteObjectAsync("/services", mRequestManager);
                    var services = await servicesFolder.LoadChildrenAsync(mRequestManager);
                    
                    foreach(var service in services)
                    {
                        if(service.Name == "locationManager")
                        {
                            var locationNode = mTreeView.Nodes.Add("locations");
                            locationNode.Tag = locationNode;
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

        private void FillTree(TreeNode node, System.Collections.Generic.IEnumerable<RemoteObject> objects)
        {           
            mTreeView.SuspendLayout();

            try
            {
                foreach (var remoteObject in objects)
                {                        
                    TreeNode newNode = node.Nodes.Add(remoteObject.Name);
                    newNode.Name = newNode.Text;
                    newNode.Tag = remoteObject;

                    if(remoteObject is RemoteFolder)
                    {                                                        
                        var subNode = newNode.Nodes.Add("dummy");
                        subNode.Tag = this;
                    }                        
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
    }
}
