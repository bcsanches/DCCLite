// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

namespace SharpTerminal
{
    partial class Console
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
            if (disposing)
            {
                if(components != null)
                    components.Dispose();

                if (mRequestManager != null)
                    mRequestManager.Dispose();
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
			System.Windows.Forms.StatusStrip statusStrip1;
			m_lbStatus = new System.Windows.Forms.ToolStripStatusLabel();
			splitContainer2 = new System.Windows.Forms.SplitContainer();
			ucTreeView = new ObjectsTreeViewUserControl();
			mInternalPanel = new System.Windows.Forms.SplitContainer();
			ucConsole = new ConsoleUserControl();
			statusStrip1 = new System.Windows.Forms.StatusStrip();
			statusStrip1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
			splitContainer2.Panel1.SuspendLayout();
			splitContainer2.Panel2.SuspendLayout();
			splitContainer2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)mInternalPanel).BeginInit();
			mInternalPanel.Panel2.SuspendLayout();
			mInternalPanel.SuspendLayout();
			SuspendLayout();
			// 
			// statusStrip1
			// 
			statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] { m_lbStatus });
			statusStrip1.Location = new System.Drawing.Point(0, 772);
			statusStrip1.Name = "statusStrip1";
			statusStrip1.Padding = new System.Windows.Forms.Padding(1, 0, 16, 0);
			statusStrip1.Size = new System.Drawing.Size(915, 22);
			statusStrip1.TabIndex = 5;
			statusStrip1.Text = "statusStrip1";
			// 
			// m_lbStatus
			// 
			m_lbStatus.Name = "m_lbStatus";
			m_lbStatus.Size = new System.Drawing.Size(118, 17);
			m_lbStatus.Text = "toolStripStatusLabel1";
			// 
			// splitContainer2
			// 
			splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
			splitContainer2.Location = new System.Drawing.Point(0, 0);
			splitContainer2.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			splitContainer2.Name = "splitContainer2";
			// 
			// splitContainer2.Panel1
			// 
			splitContainer2.Panel1.Controls.Add(ucTreeView);
			// 
			// splitContainer2.Panel2
			// 
			splitContainer2.Panel2.Controls.Add(mInternalPanel);
			splitContainer2.Size = new System.Drawing.Size(915, 772);
			splitContainer2.SplitterDistance = 304;
			splitContainer2.SplitterWidth = 5;
			splitContainer2.TabIndex = 7;
			// 
			// ucTreeView
			// 
			ucTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
			ucTreeView.Location = new System.Drawing.Point(0, 0);
			ucTreeView.MainDisplayPanel = mInternalPanel.Panel1;
			ucTreeView.Margin = new System.Windows.Forms.Padding(5, 3, 5, 3);
			ucTreeView.Name = "ucTreeView";
			ucTreeView.Size = new System.Drawing.Size(304, 772);
			ucTreeView.TabIndex = 0;
			// 
			// mInternalPanel
			// 
			mInternalPanel.Dock = System.Windows.Forms.DockStyle.Fill;
			mInternalPanel.Location = new System.Drawing.Point(0, 0);
			mInternalPanel.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			mInternalPanel.Name = "mInternalPanel";
			mInternalPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// mInternalPanel.Panel2
			// 
			mInternalPanel.Panel2.Controls.Add(ucConsole);
			mInternalPanel.Size = new System.Drawing.Size(606, 772);
			mInternalPanel.SplitterDistance = 463;
			mInternalPanel.SplitterWidth = 5;
			mInternalPanel.TabIndex = 6;
			// 
			// ucConsole
			// 
			ucConsole.Dock = System.Windows.Forms.DockStyle.Fill;
			ucConsole.Location = new System.Drawing.Point(0, 0);
			ucConsole.Margin = new System.Windows.Forms.Padding(5, 3, 5, 3);
			ucConsole.Name = "ucConsole";
			ucConsole.Size = new System.Drawing.Size(606, 304);
			ucConsole.TabIndex = 0;
			// 
			// Console
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			ClientSize = new System.Drawing.Size(915, 794);
			Controls.Add(splitContainer2);
			Controls.Add(statusStrip1);
			Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			Name = "Console";
			Text = "Form1";
			statusStrip1.ResumeLayout(false);
			statusStrip1.PerformLayout();
			splitContainer2.Panel1.ResumeLayout(false);
			splitContainer2.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer2).EndInit();
			splitContainer2.ResumeLayout(false);
			mInternalPanel.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)mInternalPanel).EndInit();
			mInternalPanel.ResumeLayout(false);
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion
		private System.Windows.Forms.ToolStripStatusLabel m_lbStatus;
        private System.Windows.Forms.SplitContainer mInternalPanel;
        private ConsoleUserControl ucConsole;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private ObjectsTreeViewUserControl ucTreeView;        
    }
}

