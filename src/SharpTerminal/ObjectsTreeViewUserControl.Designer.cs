namespace SharpTerminal
{
    partial class ObjectsTreeViewUserControl
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.mTreeView = new System.Windows.Forms.TreeView();
            this.mImageList = new System.Windows.Forms.ImageList(this.components);
            this.SuspendLayout();
            // 
            // mTreeView
            // 
            this.mTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mTreeView.FullRowSelect = true;
            this.mTreeView.HideSelection = false;
            this.mTreeView.Location = new System.Drawing.Point(0, 0);
            this.mTreeView.Name = "mTreeView";
            this.mTreeView.PathSeparator = "/";
            this.mTreeView.Size = new System.Drawing.Size(150, 359);
            this.mTreeView.TabIndex = 0;
            this.mTreeView.BeforeExpand += new System.Windows.Forms.TreeViewCancelEventHandler(this.mTreeView_BeforeExpand);
            // 
            // mImageList
            // 
            this.mImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth8Bit;
            this.mImageList.ImageSize = new System.Drawing.Size(16, 16);
            this.mImageList.TransparentColor = System.Drawing.Color.Transparent;
            // 
            // ObjectsTreeViewUserControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.mTreeView);
            this.Name = "ObjectsTreeViewUserControl";
            this.Size = new System.Drawing.Size(150, 359);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TreeView mTreeView;
        private System.Windows.Forms.ImageList mImageList;
    }
}
