

namespace SharpTerminal
{
    partial class RemoteDeviceUserControl
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
                components?.Dispose();

                if(mRemoteDevice != null)
                {
                    mRemoteDevice.PropertyChanged -= RemoteDevice_PropertyChanged;                    
                }                    
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
			System.Windows.Forms.GroupBox groupBox1;
			m_gridMain = new System.Windows.Forms.DataGridView();
			Column1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			m_lbTitle = new System.Windows.Forms.Label();
			m_btnRename = new System.Windows.Forms.Button();
			m_btnClear = new System.Windows.Forms.Button();
			groupBox1 = new System.Windows.Forms.GroupBox();
			groupBox1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)m_gridMain).BeginInit();
			SuspendLayout();
			// 
			// groupBox1
			// 
			groupBox1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			groupBox1.Controls.Add(m_gridMain);
			groupBox1.Location = new System.Drawing.Point(4, 42);
			groupBox1.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			groupBox1.Name = "groupBox1";
			groupBox1.Padding = new System.Windows.Forms.Padding(4, 3, 4, 3);
			groupBox1.Size = new System.Drawing.Size(594, 275);
			groupBox1.TabIndex = 2;
			groupBox1.TabStop = false;
			groupBox1.Text = "Pins";
			// 
			// m_gridMain
			// 
			m_gridMain.AllowUserToAddRows = false;
			m_gridMain.AllowUserToDeleteRows = false;
			m_gridMain.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			m_gridMain.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] { Column1, Column2, Column4, Column3, Column5 });
			m_gridMain.Dock = System.Windows.Forms.DockStyle.Fill;
			m_gridMain.Location = new System.Drawing.Point(4, 19);
			m_gridMain.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			m_gridMain.MultiSelect = false;
			m_gridMain.Name = "m_gridMain";
			m_gridMain.ReadOnly = true;
			m_gridMain.RowHeadersVisible = false;
			m_gridMain.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
			m_gridMain.Size = new System.Drawing.Size(586, 253);
			m_gridMain.TabIndex = 0;
			// 
			// Column1
			// 
			Column1.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
			Column1.HeaderText = "Pins";
			Column1.Name = "Column1";
			Column1.ReadOnly = true;
			Column1.Width = 54;
			// 
			// Column2
			// 
			Column2.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
			Column2.HeaderText = "Name";
			Column2.Name = "Column2";
			Column2.ReadOnly = true;
			Column2.Width = 64;
			// 
			// Column4
			// 
			Column4.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
			Column4.HeaderText = "Decoder";
			Column4.Name = "Column4";
			Column4.ReadOnly = true;
			// 
			// Column3
			// 
			Column3.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
			Column3.HeaderText = "Address";
			Column3.Name = "Column3";
			Column3.ReadOnly = true;
			Column3.Width = 74;
			// 
			// Column5
			// 
			Column5.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
			Column5.HeaderText = "Usage";
			Column5.Name = "Column5";
			Column5.ReadOnly = true;
			Column5.Width = 64;
			// 
			// m_lbTitle
			// 
			m_lbTitle.AutoSize = true;
			m_lbTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, 0);
			m_lbTitle.Location = new System.Drawing.Point(4, 10);
			m_lbTitle.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			m_lbTitle.Name = "m_lbTitle";
			m_lbTitle.Size = new System.Drawing.Size(171, 24);
			m_lbTitle.TabIndex = 1;
			m_lbTitle.Text = "Remote Device - ";
			// 
			// m_btnRename
			// 
			m_btnRename.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnRename.Location = new System.Drawing.Point(8, 323);
			m_btnRename.Name = "m_btnRename";
			m_btnRename.Size = new System.Drawing.Size(75, 23);
			m_btnRename.TabIndex = 3;
			m_btnRename.Text = "Rename...";
			m_btnRename.UseVisualStyleBackColor = true;
			m_btnRename.Click += m_btnRename_Click;
			// 
			// m_btnClear
			// 
			m_btnClear.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnClear.Location = new System.Drawing.Point(89, 323);
			m_btnClear.Name = "m_btnClear";
			m_btnClear.Size = new System.Drawing.Size(97, 23);
			m_btnClear.TabIndex = 4;
			m_btnClear.Text = "Clear EEPROM";
			m_btnClear.UseVisualStyleBackColor = true;
			m_btnClear.Click += m_btnClear_Click;
			// 
			// RemoteDeviceUserControl
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			Controls.Add(m_btnClear);
			Controls.Add(m_btnRename);
			Controls.Add(groupBox1);
			Controls.Add(m_lbTitle);
			Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			Name = "RemoteDeviceUserControl";
			Size = new System.Drawing.Size(601, 349);
			groupBox1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)m_gridMain).EndInit();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.DataGridView m_gridMain;
        private System.Windows.Forms.Label m_lbTitle;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column1;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column2;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column4;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column3;
        private System.Windows.Forms.DataGridViewTextBoxColumn Column5;
		private System.Windows.Forms.Button m_btnRename;
		private System.Windows.Forms.Button m_btnClear;
	}
}
