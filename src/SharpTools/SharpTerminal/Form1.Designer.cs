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
            this.m_lbStatus = new System.Windows.Forms.ToolStripStatusLabel();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.ucTreeView = new SharpTerminal.ObjectsTreeViewUserControl();
            this.mInternalPanel = new System.Windows.Forms.SplitContainer();
            this.ucConsole = new SharpTerminal.ConsoleUserControl();
            statusStrip1 = new System.Windows.Forms.StatusStrip();
            statusStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.mInternalPanel)).BeginInit();
            this.mInternalPanel.Panel2.SuspendLayout();
            this.mInternalPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // statusStrip1
            // 
            statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_lbStatus});
            statusStrip1.Location = new System.Drawing.Point(0, 666);
            statusStrip1.Name = "statusStrip1";
            statusStrip1.Size = new System.Drawing.Size(784, 22);
            statusStrip1.TabIndex = 5;
            statusStrip1.Text = "statusStrip1";
            // 
            // m_lbStatus
            // 
            this.m_lbStatus.Name = "m_lbStatus";
            this.m_lbStatus.Size = new System.Drawing.Size(118, 17);
            this.m_lbStatus.Text = "toolStripStatusLabel1";
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.Location = new System.Drawing.Point(0, 0);
            this.splitContainer2.Name = "splitContainer2";
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.ucTreeView);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.mInternalPanel);
            this.splitContainer2.Size = new System.Drawing.Size(784, 666);
            this.splitContainer2.SplitterDistance = 261;
            this.splitContainer2.TabIndex = 7;
            // 
            // ucTreeView
            // 
            this.ucTreeView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ucTreeView.Location = new System.Drawing.Point(0, 0);
            this.ucTreeView.MainDisplayPanel = this.mInternalPanel.Panel1;
            this.ucTreeView.Name = "ucTreeView";
            this.ucTreeView.Size = new System.Drawing.Size(261, 666);
            this.ucTreeView.TabIndex = 0;
            // 
            // mInternalPanel
            // 
            this.mInternalPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mInternalPanel.Location = new System.Drawing.Point(0, 0);
            this.mInternalPanel.Name = "mInternalPanel";
            this.mInternalPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // mInternalPanel.Panel2
            // 
            this.mInternalPanel.Panel2.Controls.Add(this.ucConsole);
            this.mInternalPanel.Size = new System.Drawing.Size(519, 666);
            this.mInternalPanel.SplitterDistance = 400;
            this.mInternalPanel.TabIndex = 6;
            // 
            // ucConsole
            // 
            this.ucConsole.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ucConsole.Location = new System.Drawing.Point(0, 0);
            this.ucConsole.Name = "ucConsole";
            this.ucConsole.Size = new System.Drawing.Size(519, 262);
            this.ucConsole.TabIndex = 0;
            // 
            // Console
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 688);
            this.Controls.Add(this.splitContainer2);
            this.Controls.Add(statusStrip1);
            this.Name = "Console";
            this.Text = "Form1";
            statusStrip1.ResumeLayout(false);
            statusStrip1.PerformLayout();
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
            this.splitContainer2.ResumeLayout(false);
            this.mInternalPanel.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.mInternalPanel)).EndInit();
            this.mInternalPanel.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.ToolStripStatusLabel m_lbStatus;
        private System.Windows.Forms.SplitContainer mInternalPanel;
        private ConsoleUserControl ucConsole;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private ObjectsTreeViewUserControl ucTreeView;        
    }
}

