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
			components = new System.ComponentModel.Container();
			System.Windows.Forms.GroupBox groupBox1;
			mServicesGrid = new System.Windows.Forms.DataGridView();
			Server = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Address = new System.Windows.Forms.DataGridViewTextBoxColumn();
			m_btnCancel = new System.Windows.Forms.Button();
			m_btnOK = new System.Windows.Forms.Button();
			mBackgroundWorker = new System.ComponentModel.BackgroundWorker();
			mTimer = new System.Windows.Forms.Timer(components);
			m_lblCountdown = new System.Windows.Forms.Label();
			mCountdownTimer = new System.Windows.Forms.Timer(components);
			groupBox1 = new System.Windows.Forms.GroupBox();
			groupBox1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)mServicesGrid).BeginInit();
			SuspendLayout();
			// 
			// groupBox1
			// 
			groupBox1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			groupBox1.Controls.Add(mServicesGrid);
			groupBox1.Location = new System.Drawing.Point(14, 14);
			groupBox1.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			groupBox1.Name = "groupBox1";
			groupBox1.Padding = new System.Windows.Forms.Padding(4, 3, 4, 3);
			groupBox1.Size = new System.Drawing.Size(455, 309);
			groupBox1.TabIndex = 0;
			groupBox1.TabStop = false;
			groupBox1.Text = "Available servers:";
			// 
			// mServicesGrid
			// 
			mServicesGrid.AllowUserToAddRows = false;
			mServicesGrid.AllowUserToDeleteRows = false;
			mServicesGrid.AllowUserToResizeRows = false;
			mServicesGrid.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			mServicesGrid.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] { Server, Address });
			mServicesGrid.Dock = System.Windows.Forms.DockStyle.Fill;
			mServicesGrid.EditMode = System.Windows.Forms.DataGridViewEditMode.EditProgrammatically;
			mServicesGrid.Location = new System.Drawing.Point(4, 19);
			mServicesGrid.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			mServicesGrid.MultiSelect = false;
			mServicesGrid.Name = "mServicesGrid";
			mServicesGrid.ReadOnly = true;
			mServicesGrid.RowHeadersVisible = false;
			mServicesGrid.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
			mServicesGrid.Size = new System.Drawing.Size(447, 287);
			mServicesGrid.TabIndex = 0;
			mServicesGrid.SelectionChanged += mServicesGrid_SelectionChanged;
			// 
			// Server
			// 
			Server.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
			Server.HeaderText = "Server";
			Server.Name = "Server";
			Server.ReadOnly = true;
			Server.Width = 64;
			// 
			// Address
			// 
			Address.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
			Address.HeaderText = "Address";
			Address.Name = "Address";
			Address.ReadOnly = true;
			// 
			// m_btnCancel
			// 
			m_btnCancel.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
			m_btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			m_btnCancel.Location = new System.Drawing.Point(382, 330);
			m_btnCancel.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			m_btnCancel.Name = "m_btnCancel";
			m_btnCancel.Size = new System.Drawing.Size(88, 27);
			m_btnCancel.TabIndex = 1;
			m_btnCancel.Text = "&Cancel";
			m_btnCancel.UseVisualStyleBackColor = true;
			// 
			// m_btnOK
			// 
			m_btnOK.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
			m_btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
			m_btnOK.Enabled = false;
			m_btnOK.Location = new System.Drawing.Point(287, 330);
			m_btnOK.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			m_btnOK.Name = "m_btnOK";
			m_btnOK.Size = new System.Drawing.Size(88, 27);
			m_btnOK.TabIndex = 2;
			m_btnOK.Text = "&OK";
			m_btnOK.UseVisualStyleBackColor = true;
			// 
			// mBackgroundWorker
			// 
			mBackgroundWorker.DoWork += mBackgroundWorker_DoWork;
			mBackgroundWorker.ProgressChanged += mBackgroundWorker_ProgressChanged;
			// 
			// mTimer
			// 
			mTimer.Interval = 1000;
			mTimer.Tick += mTimer_Tick;
			// 
			// m_lblCountdown
			// 
			m_lblCountdown.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_lblCountdown.AutoSize = true;
			m_lblCountdown.Location = new System.Drawing.Point(14, 336);
			m_lblCountdown.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			m_lblCountdown.Name = "m_lblCountdown";
			m_lblCountdown.Size = new System.Drawing.Size(104, 15);
			m_lblCountdown.TabIndex = 3;
			m_lblCountdown.Text = "Auto connect in ...";
			m_lblCountdown.Visible = false;
			// 
			// mCountdownTimer
			// 
			mCountdownTimer.Interval = 1000;
			mCountdownTimer.Tick += mCountdownTimer_Tick;
			// 
			// ServerSelectionForm
			// 
			AcceptButton = m_btnOK;
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			CancelButton = m_btnCancel;
			ClientSize = new System.Drawing.Size(483, 370);
			Controls.Add(m_lblCountdown);
			Controls.Add(m_btnOK);
			Controls.Add(m_btnCancel);
			Controls.Add(groupBox1);
			KeyPreview = true;
			Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			MinimumSize = new System.Drawing.Size(380, 240);
			Name = "ServerSelectionForm";
			Text = "Select server";
			Click += ServerSelectionForm_Click;
			KeyDown += ServerSelectionForm_KeyDown;
			groupBox1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)mServicesGrid).EndInit();
			ResumeLayout(false);
			PerformLayout();
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

