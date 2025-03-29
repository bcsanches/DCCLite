namespace SharpEEPromViewer
{
    partial class MainForm
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

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			components = new System.ComponentModel.Container();
			menuStrip1 = new System.Windows.Forms.MenuStrip();
			fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			mSplitContainer = new System.Windows.Forms.SplitContainer();
			mTreeView = new System.Windows.Forms.TreeView();
			mImageList = new System.Windows.Forms.ImageList(components);
			menuStrip1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)mSplitContainer).BeginInit();
			mSplitContainer.Panel1.SuspendLayout();
			mSplitContainer.SuspendLayout();
			SuspendLayout();
			// 
			// menuStrip1
			// 
			menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] { fileToolStripMenuItem, helpToolStripMenuItem });
			menuStrip1.Location = new System.Drawing.Point(0, 0);
			menuStrip1.Name = "menuStrip1";
			menuStrip1.Padding = new System.Windows.Forms.Padding(7, 2, 0, 2);
			menuStrip1.Size = new System.Drawing.Size(728, 24);
			menuStrip1.TabIndex = 4;
			menuStrip1.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] { openToolStripMenuItem, toolStripSeparator1, exitToolStripMenuItem });
			fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
			fileToolStripMenuItem.Text = "File";
			// 
			// openToolStripMenuItem
			// 
			openToolStripMenuItem.Name = "openToolStripMenuItem";
			openToolStripMenuItem.ShortcutKeyDisplayString = "Ctrl + O";
			openToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
			openToolStripMenuItem.Text = "&Open";
			openToolStripMenuItem.Click += OpenToolStripMenuItem_Click;
			// 
			// toolStripSeparator1
			// 
			toolStripSeparator1.Name = "toolStripSeparator1";
			toolStripSeparator1.Size = new System.Drawing.Size(149, 6);
			// 
			// exitToolStripMenuItem
			// 
			exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			exitToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
			exitToolStripMenuItem.Text = "E&xit";
			exitToolStripMenuItem.Click += btnExit_Click;
			// 
			// helpToolStripMenuItem
			// 
			helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] { aboutToolStripMenuItem });
			helpToolStripMenuItem.Name = "helpToolStripMenuItem";
			helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
			helpToolStripMenuItem.Text = "Help";
			// 
			// aboutToolStripMenuItem
			// 
			aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
			aboutToolStripMenuItem.Size = new System.Drawing.Size(107, 22);
			aboutToolStripMenuItem.Text = "About";
			// 
			// mSplitContainer
			// 
			mSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
			mSplitContainer.Location = new System.Drawing.Point(0, 24);
			mSplitContainer.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			mSplitContainer.Name = "mSplitContainer";
			// 
			// mSplitContainer.Panel1
			// 
			mSplitContainer.Panel1.Controls.Add(mTreeView);
			mSplitContainer.Size = new System.Drawing.Size(728, 485);
			mSplitContainer.SplitterDistance = 241;
			mSplitContainer.SplitterWidth = 5;
			mSplitContainer.TabIndex = 5;
			// 
			// mTreeView
			// 
			mTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
			mTreeView.Location = new System.Drawing.Point(0, 0);
			mTreeView.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			mTreeView.Name = "mTreeView";
			mTreeView.Size = new System.Drawing.Size(241, 485);
			mTreeView.TabIndex = 0;
			mTreeView.AfterSelect += mTreeView_AfterSelect;
			// 
			// mImageList
			// 
			mImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth32Bit;
			mImageList.ImageSize = new System.Drawing.Size(16, 16);
			mImageList.TransparentColor = System.Drawing.Color.Transparent;
			// 
			// MainForm
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			ClientSize = new System.Drawing.Size(728, 509);
			Controls.Add(mSplitContainer);
			Controls.Add(menuStrip1);
			MainMenuStrip = menuStrip1;
			Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			MinimumSize = new System.Drawing.Size(744, 548);
			Name = "MainForm";
			Text = "EEProm Viewer";
			KeyUp += MainForm_KeyUp;
			menuStrip1.ResumeLayout(false);
			menuStrip1.PerformLayout();
			mSplitContainer.Panel1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)mSplitContainer).EndInit();
			mSplitContainer.ResumeLayout(false);
			ResumeLayout(false);
			PerformLayout();

		}

		#endregion

		private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.SplitContainer mSplitContainer;
        private System.Windows.Forms.TreeView mTreeView;
        private System.Windows.Forms.ImageList mImageList;
    }
}