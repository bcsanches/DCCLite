using System;
using System.Json;
using System.Text;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class ObjectsTreeViewUserControl : UserControl
    {
        RequestManager mRequestManager;

        class ObjectListRetriever : IResponseHandler
        {
            private ObjectsTreeViewUserControl mOwner;
            private TreeNode mNode;

            public ObjectListRetriever(ObjectsTreeViewUserControl owner, TreeNode node)
            {
                mOwner = owner;
                mNode = node;
            }

            public void OnError(string msg, int id)
            {
                throw new NotImplementedException();
            }

            public void OnResponse(JsonValue response, int id)
            {
                var responseObj = (JsonObject)response;
                var items = (JsonArray)responseObj["children"];

                mOwner.FillTree(mNode, items);                
            }
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

        private void RequestTreeNodesChildren(string path, TreeNode parent)
        {
            mRequestManager.DispatchRequest(new string[] { "Get-ChildItem", path }, new ObjectListRetriever(this, parent));
        }

        private void mRequestManager_ConnectionStateChanged(RequestManager sender, ConnectionStateEventArgs args)
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

                    var root = mTreeView.Nodes.Add("root");

                    RequestTreeNodesChildren("/", root);                    
                }
                else if (args.State == ConnectionState.DISCONNECTED)
                {

                }
            }            
        }

        private void FillTree(TreeNode node, JsonArray objects)
        {
            if (mTreeView.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.FillTree(node, objects); }));
            }
            else
            {                
                mTreeView.SuspendLayout();

                try
                {
                    foreach (var item in objects)
                    {
                        TreeNode newNode = node.Nodes.Add(item["name"]);
                        newNode.Name = newNode.Text;

                        if(item["isFolder"])
                        {
                            newNode.Tag = this;
                            newNode.Nodes.Add("dummy");
                        }                        
                    }

                    node.Expand();
                }
                finally
                {
                    mTreeView.ResumeLayout();
                }                
            }
        }

        public ObjectsTreeViewUserControl()
        {
            InitializeComponent();
        }

        private void GetTreePath_r(TreeNode node, StringBuilder strBuilder)
        {
            if (node.Parent != null)
            {
                GetTreePath_r(node.Parent, strBuilder);

                strBuilder.Append("/");
                strBuilder.Append(node.Name);
            }
            else
            {
                //nothing
            }                
        }

        private string GetTreePath(TreeNode node)
        {
            var strBuilder = new StringBuilder(128);

            GetTreePath_r(node, strBuilder);

            return strBuilder.ToString();
        }

        private void mTreeView_BeforeExpand(object sender, TreeViewCancelEventArgs e)
        {
            if(e.Node.Tag == this)
            {
                var path = GetTreePath(e.Node);

                e.Node.Nodes.Clear();
                e.Node.Tag = null;
                RequestTreeNodesChildren(path, e.Node);                

                e.Cancel = true;
            }
        }
    }
}
