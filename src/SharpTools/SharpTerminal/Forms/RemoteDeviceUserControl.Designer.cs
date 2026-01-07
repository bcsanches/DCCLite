

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
			System.Windows.Forms.TabPage tabPage1;
			System.Windows.Forms.ColumnHeader Time;
			System.Windows.Forms.ColumnHeader Type;
			System.Windows.Forms.ColumnHeader Info;
			System.Windows.Forms.GroupBox groupBox1;
			System.Windows.Forms.FlowLayoutPanel flowLayoutPanel2;
			m_gridMain = new System.Windows.Forms.DataGridView();
			Column1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			m_tabControl = new System.Windows.Forms.TabControl();
			tabPage2 = new System.Windows.Forms.TabPage();
			m_lvEventsLog = new System.Windows.Forms.ListView();
			m_lbTitle = new System.Windows.Forms.Label();
			m_btnRename = new System.Windows.Forms.Button();
			m_btnClear = new System.Windows.Forms.Button();
			m_btnEmulate = new System.Windows.Forms.Button();
			m_btnBlock = new System.Windows.Forms.Button();
			m_btnReboot = new System.Windows.Forms.Button();
			m_btnNetworkTest = new System.Windows.Forms.Button();
			m_btnReadEEPROM = new System.Windows.Forms.Button();
			flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
			m_btnDisconnect = new System.Windows.Forms.Button();
			m_lblSentBytes = new System.Windows.Forms.Label();
			m_lblReceivedBytes = new System.Windows.Forms.Label();
			tabPage1 = new System.Windows.Forms.TabPage();
			Time = new System.Windows.Forms.ColumnHeader();
			Type = new System.Windows.Forms.ColumnHeader();
			Info = new System.Windows.Forms.ColumnHeader();
			groupBox1 = new System.Windows.Forms.GroupBox();
			flowLayoutPanel2 = new System.Windows.Forms.FlowLayoutPanel();
			tabPage1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)m_gridMain).BeginInit();
			m_tabControl.SuspendLayout();
			tabPage2.SuspendLayout();
			flowLayoutPanel1.SuspendLayout();
			groupBox1.SuspendLayout();
			flowLayoutPanel2.SuspendLayout();
			SuspendLayout();
			// 
			// tabPage1
			// 
			tabPage1.Controls.Add(m_gridMain);
			tabPage1.Location = new System.Drawing.Point(4, 24);
			tabPage1.Name = "tabPage1";
			tabPage1.Padding = new System.Windows.Forms.Padding(3);
			tabPage1.Size = new System.Drawing.Size(503, 324);
			tabPage1.TabIndex = 0;
			tabPage1.Text = "Pins";
			tabPage1.UseVisualStyleBackColor = true;
			// 
			// m_gridMain
			// 
			m_gridMain.AllowUserToAddRows = false;
			m_gridMain.AllowUserToDeleteRows = false;
			m_gridMain.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			m_gridMain.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] { Column1, Column2, Column4, Column3, Column5 });
			m_gridMain.Dock = System.Windows.Forms.DockStyle.Fill;
			m_gridMain.Location = new System.Drawing.Point(3, 3);
			m_gridMain.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			m_gridMain.MultiSelect = false;
			m_gridMain.Name = "m_gridMain";
			m_gridMain.ReadOnly = true;
			m_gridMain.RowHeadersVisible = false;
			m_gridMain.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
			m_gridMain.Size = new System.Drawing.Size(497, 318);
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
			// Time
			// 
			Time.Text = "Time";
			Time.Width = 120;
			// 
			// Type
			// 
			Type.Text = "Type";
			Type.Width = 100;
			// 
			// Info
			// 
			Info.Text = "Information";
			Info.Width = 200;
			// 
			// m_tabControl
			// 
			m_tabControl.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_tabControl.Controls.Add(tabPage1);
			m_tabControl.Controls.Add(tabPage2);
			m_tabControl.Location = new System.Drawing.Point(115, 42);
			m_tabControl.Name = "m_tabControl";
			m_tabControl.SelectedIndex = 0;
			m_tabControl.Size = new System.Drawing.Size(511, 352);
			m_tabControl.TabIndex = 11;
			m_tabControl.SelectedIndexChanged += tabControl_SelectedIndexChanged;
			// 
			// tabPage2
			// 
			tabPage2.Controls.Add(m_lvEventsLog);
			tabPage2.Location = new System.Drawing.Point(4, 24);
			tabPage2.Name = "tabPage2";
			tabPage2.Padding = new System.Windows.Forms.Padding(3);
			tabPage2.Size = new System.Drawing.Size(503, 324);
			tabPage2.TabIndex = 1;
			tabPage2.Text = "Events";
			tabPage2.UseVisualStyleBackColor = true;
			// 
			// m_lvEventsLog
			// 
			m_lvEventsLog.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { Time, Type, Info });
			m_lvEventsLog.Dock = System.Windows.Forms.DockStyle.Fill;
			m_lvEventsLog.FullRowSelect = true;
			m_lvEventsLog.GridLines = true;
			m_lvEventsLog.Location = new System.Drawing.Point(3, 3);
			m_lvEventsLog.MultiSelect = false;
			m_lvEventsLog.Name = "m_lvEventsLog";
			m_lvEventsLog.Size = new System.Drawing.Size(497, 318);
			m_lvEventsLog.TabIndex = 0;
			m_lvEventsLog.UseCompatibleStateImageBehavior = false;
			m_lvEventsLog.View = System.Windows.Forms.View.Details;
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
			m_btnRename.Location = new System.Drawing.Point(3, 206);
			m_btnRename.Name = "m_btnRename";
			m_btnRename.Size = new System.Drawing.Size(97, 23);
			m_btnRename.TabIndex = 3;
			m_btnRename.Text = "Rename...";
			m_btnRename.UseVisualStyleBackColor = true;
			m_btnRename.Click += m_btnRename_Click;
			// 
			// m_btnClear
			// 
			m_btnClear.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnClear.Location = new System.Drawing.Point(3, 32);
			m_btnClear.Name = "m_btnClear";
			m_btnClear.Size = new System.Drawing.Size(97, 23);
			m_btnClear.TabIndex = 4;
			m_btnClear.Text = "Clear EEPROM";
			m_btnClear.UseVisualStyleBackColor = true;
			m_btnClear.Click += m_btnClear_Click;
			// 
			// m_btnEmulate
			// 
			m_btnEmulate.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnEmulate.Enabled = false;
			m_btnEmulate.Location = new System.Drawing.Point(3, 90);
			m_btnEmulate.Name = "m_btnEmulate";
			m_btnEmulate.Size = new System.Drawing.Size(97, 23);
			m_btnEmulate.TabIndex = 5;
			m_btnEmulate.Text = "Emulate";
			m_btnEmulate.UseVisualStyleBackColor = true;
			m_btnEmulate.Click += m_btnEmulate_Click;
			// 
			// m_btnBlock
			// 
			m_btnBlock.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnBlock.Enabled = false;
			m_btnBlock.Location = new System.Drawing.Point(3, 3);
			m_btnBlock.Name = "m_btnBlock";
			m_btnBlock.Size = new System.Drawing.Size(97, 23);
			m_btnBlock.TabIndex = 6;
			m_btnBlock.Text = "Block";
			m_btnBlock.UseVisualStyleBackColor = true;
			m_btnBlock.Click += m_btnBlock_Click;
			// 
			// m_btnReboot
			// 
			m_btnReboot.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnReboot.Enabled = false;
			m_btnReboot.Location = new System.Drawing.Point(3, 177);
			m_btnReboot.Name = "m_btnReboot";
			m_btnReboot.Size = new System.Drawing.Size(97, 23);
			m_btnReboot.TabIndex = 7;
			m_btnReboot.Text = "Reboot";
			m_btnReboot.UseVisualStyleBackColor = true;
			m_btnReboot.Click += m_btnReboot_Click;
			// 
			// m_btnNetworkTest
			// 
			m_btnNetworkTest.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnNetworkTest.Location = new System.Drawing.Point(3, 119);
			m_btnNetworkTest.Name = "m_btnNetworkTest";
			m_btnNetworkTest.Size = new System.Drawing.Size(97, 23);
			m_btnNetworkTest.TabIndex = 8;
			m_btnNetworkTest.Text = "Network...";
			m_btnNetworkTest.UseVisualStyleBackColor = true;
			m_btnNetworkTest.Click += m_btnNetworkTest_Click;
			// 
			// m_btnReadEEPROM
			// 
			m_btnReadEEPROM.Location = new System.Drawing.Point(3, 148);
			m_btnReadEEPROM.Name = "m_btnReadEEPROM";
			m_btnReadEEPROM.Size = new System.Drawing.Size(97, 23);
			m_btnReadEEPROM.TabIndex = 9;
			m_btnReadEEPROM.Text = "Read EEPROM";
			m_btnReadEEPROM.UseVisualStyleBackColor = true;
			m_btnReadEEPROM.Click += m_btnReadEEPROM_Click;
			// 
			// flowLayoutPanel1
			// 
			flowLayoutPanel1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			flowLayoutPanel1.Controls.Add(m_btnBlock);
			flowLayoutPanel1.Controls.Add(m_btnClear);
			flowLayoutPanel1.Controls.Add(m_btnDisconnect);
			flowLayoutPanel1.Controls.Add(m_btnEmulate);
			flowLayoutPanel1.Controls.Add(m_btnNetworkTest);
			flowLayoutPanel1.Controls.Add(m_btnReadEEPROM);
			flowLayoutPanel1.Controls.Add(m_btnReboot);
			flowLayoutPanel1.Controls.Add(m_btnRename);
			flowLayoutPanel1.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
			flowLayoutPanel1.Location = new System.Drawing.Point(4, 42);
			flowLayoutPanel1.Name = "flowLayoutPanel1";
			flowLayoutPanel1.Size = new System.Drawing.Size(105, 284);
			flowLayoutPanel1.TabIndex = 10;
			// 
			// m_btnDisconnect
			// 
			m_btnDisconnect.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnDisconnect.Location = new System.Drawing.Point(3, 61);
			m_btnDisconnect.Name = "m_btnDisconnect";
			m_btnDisconnect.Size = new System.Drawing.Size(97, 23);
			m_btnDisconnect.TabIndex = 10;
			m_btnDisconnect.Text = "Disconnect";
			m_btnDisconnect.UseVisualStyleBackColor = true;
			m_btnDisconnect.Click += m_btnDisconnect_Click;
			// 
			// groupBox1
			// 
			groupBox1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			groupBox1.Controls.Add(flowLayoutPanel2);
			groupBox1.Location = new System.Drawing.Point(4, 332);
			groupBox1.Name = "groupBox1";
			groupBox1.Size = new System.Drawing.Size(105, 53);
			groupBox1.TabIndex = 12;
			groupBox1.TabStop = false;
			groupBox1.Text = "Data Flow";
			// 
			// flowLayoutPanel2
			// 
			flowLayoutPanel2.Controls.Add(m_lblSentBytes);
			flowLayoutPanel2.Controls.Add(m_lblReceivedBytes);
			flowLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			flowLayoutPanel2.FlowDirection = System.Windows.Forms.FlowDirection.TopDown;
			flowLayoutPanel2.Location = new System.Drawing.Point(3, 19);
			flowLayoutPanel2.Name = "flowLayoutPanel2";
			flowLayoutPanel2.Size = new System.Drawing.Size(99, 31);
			flowLayoutPanel2.TabIndex = 0;
			// 
			// m_lblSentBytes
			// 
			m_lblSentBytes.AutoSize = true;
			m_lblSentBytes.Location = new System.Drawing.Point(3, 0);
			m_lblSentBytes.Name = "m_lblSentBytes";
			m_lblSentBytes.Size = new System.Drawing.Size(73, 15);
			m_lblSentBytes.TabIndex = 0;
			m_lblSentBytes.Text = "Sent: 0 bytes";
			// 
			// m_lblReceivedBytes
			// 
			m_lblReceivedBytes.AutoSize = true;
			m_lblReceivedBytes.Location = new System.Drawing.Point(3, 15);
			m_lblReceivedBytes.Name = "m_lblReceivedBytes";
			m_lblReceivedBytes.Size = new System.Drawing.Size(69, 15);
			m_lblReceivedBytes.TabIndex = 1;
			m_lblReceivedBytes.Text = "Rcv: 0 bytes";
			// 
			// RemoteDeviceUserControl
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			Controls.Add(groupBox1);
			Controls.Add(m_tabControl);
			Controls.Add(flowLayoutPanel1);
			Controls.Add(m_lbTitle);
			Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			Name = "RemoteDeviceUserControl";
			Size = new System.Drawing.Size(629, 397);
			tabPage1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)m_gridMain).EndInit();
			m_tabControl.ResumeLayout(false);
			tabPage2.ResumeLayout(false);
			flowLayoutPanel1.ResumeLayout(false);
			groupBox1.ResumeLayout(false);
			flowLayoutPanel2.ResumeLayout(false);
			flowLayoutPanel2.PerformLayout();
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
		private System.Windows.Forms.Button m_btnEmulate;
		private System.Windows.Forms.Button m_btnBlock;
		private System.Windows.Forms.Button m_btnReboot;
		private System.Windows.Forms.Button m_btnNetworkTest;
		private System.Windows.Forms.Button m_btnReadEEPROM;
		private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
		private System.Windows.Forms.Button m_btnDisconnect;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.ListView m_lvEventsLog;
		private System.Windows.Forms.TabControl m_tabControl;
		private System.Windows.Forms.Label m_lblSentBytes;
		private System.Windows.Forms.Label m_lblReceivedBytes;
	}
}
