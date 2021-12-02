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
    partial class ServerSelectionForm
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

                if (mClient != null)
                    mClient.Dispose();
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
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.GroupBox groupBox1;
            this.mServicesGrid = new System.Windows.Forms.DataGridView();
            this.Server = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Address = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.m_btnCancel = new System.Windows.Forms.Button();
            this.m_btnOK = new System.Windows.Forms.Button();
            this.mBackgroundWorker = new System.ComponentModel.BackgroundWorker();
            this.mTimer = new System.Windows.Forms.Timer(this.components);
            this.m_lblCountdown = new System.Windows.Forms.Label();
            this.mCountdownTimer = new System.Windows.Forms.Timer(this.components);
            groupBox1 = new System.Windows.Forms.GroupBox();
            groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.mServicesGrid)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            groupBox1.Controls.Add(this.mServicesGrid);
            groupBox1.Location = new System.Drawing.Point(12, 12);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new System.Drawing.Size(390, 268);
            groupBox1.TabIndex = 0;
            groupBox1.TabStop = false;
            groupBox1.Text = "Available servers:";
            // 
            // mServicesGrid
            // 
            this.mServicesGrid.AllowUserToAddRows = false;
            this.mServicesGrid.AllowUserToDeleteRows = false;
            this.mServicesGrid.AllowUserToResizeRows = false;
            this.mServicesGrid.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.mServicesGrid.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Server,
            this.Address});
            this.mServicesGrid.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mServicesGrid.EditMode = System.Windows.Forms.DataGridViewEditMode.EditProgrammatically;
            this.mServicesGrid.Location = new System.Drawing.Point(3, 16);
            this.mServicesGrid.MultiSelect = false;
            this.mServicesGrid.Name = "mServicesGrid";
            this.mServicesGrid.ReadOnly = true;
            this.mServicesGrid.RowHeadersVisible = false;
            this.mServicesGrid.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.mServicesGrid.Size = new System.Drawing.Size(384, 249);
            this.mServicesGrid.TabIndex = 0;
            this.mServicesGrid.SelectionChanged += new System.EventHandler(this.mServicesGrid_SelectionChanged);
            // 
            // Server
            // 
            this.Server.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
            this.Server.HeaderText = "Server";
            this.Server.Name = "Server";
            this.Server.ReadOnly = true;
            this.Server.Width = 63;
            // 
            // Address
            // 
            this.Address.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.Address.HeaderText = "Address";
            this.Address.Name = "Address";
            this.Address.ReadOnly = true;
            // 
            // m_btnCancel
            // 
            this.m_btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.m_btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_btnCancel.Location = new System.Drawing.Point(327, 286);
            this.m_btnCancel.Name = "m_btnCancel";
            this.m_btnCancel.Size = new System.Drawing.Size(75, 23);
            this.m_btnCancel.TabIndex = 1;
            this.m_btnCancel.Text = "&Cancel";
            this.m_btnCancel.UseVisualStyleBackColor = true;
            // 
            // m_btnOK
            // 
            this.m_btnOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.m_btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.m_btnOK.Enabled = false;
            this.m_btnOK.Location = new System.Drawing.Point(246, 286);
            this.m_btnOK.Name = "m_btnOK";
            this.m_btnOK.Size = new System.Drawing.Size(75, 23);
            this.m_btnOK.TabIndex = 2;
            this.m_btnOK.Text = "&OK";
            this.m_btnOK.UseVisualStyleBackColor = true;
            // 
            // mBackgroundWorker
            // 
            this.mBackgroundWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.mBackgroundWorker_DoWork);
            this.mBackgroundWorker.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.mBackgroundWorker_ProgressChanged);
            // 
            // mTimer
            // 
            this.mTimer.Interval = 1000;
            this.mTimer.Tick += new System.EventHandler(this.mTimer_Tick);
            // 
            // m_lblCountdown
            // 
            this.m_lblCountdown.AutoSize = true;
            this.m_lblCountdown.Location = new System.Drawing.Point(12, 291);
            this.m_lblCountdown.Name = "m_lblCountdown";
            this.m_lblCountdown.Size = new System.Drawing.Size(94, 13);
            this.m_lblCountdown.TabIndex = 3;
            this.m_lblCountdown.Text = "Auto connect in ...";
            this.m_lblCountdown.Visible = false;
            // 
            // mCountdownTimer
            // 
            this.mCountdownTimer.Interval = 1000;
            this.mCountdownTimer.Tick += new System.EventHandler(this.mCountdownTimer_Tick);
            // 
            // ServerSelectionForm
            // 
            this.AcceptButton = this.m_btnOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.m_btnCancel;
            this.ClientSize = new System.Drawing.Size(414, 321);
            this.Controls.Add(this.m_lblCountdown);
            this.Controls.Add(this.m_btnOK);
            this.Controls.Add(this.m_btnCancel);
            this.Controls.Add(groupBox1);
            this.KeyPreview = true;
            this.Name = "ServerSelectionForm";
            this.Text = "Select server";
            this.Click += new System.EventHandler(this.ServerSelectionForm_Click);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.ServerSelectionForm_KeyDown);
            groupBox1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.mServicesGrid)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.DataGridView mServicesGrid;
        private System.Windows.Forms.DataGridViewTextBoxColumn Server;
        private System.Windows.Forms.DataGridViewTextBoxColumn Address;
        private System.Windows.Forms.Button m_btnCancel;
        private System.Windows.Forms.Button m_btnOK;
        private System.ComponentModel.BackgroundWorker mBackgroundWorker;
        private System.Windows.Forms.Timer mTimer;
        private System.Windows.Forms.Label m_lblCountdown;
        private System.Windows.Forms.Timer mCountdownTimer;
    }
}

