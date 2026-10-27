namespace SharpTerminal
{
    partial class DashboardUserControl
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
			components = new System.ComponentModel.Container();
			System.Windows.Forms.ColumnHeader columnHeader1;
			System.Windows.Forms.ColumnHeader columnHeader2;
			m_lbTitle = new System.Windows.Forms.Label();
			mLoadTimer = new System.Windows.Forms.Timer(components);
			m_tabControl = new System.Windows.Forms.TabControl();
			m_pgLog = new System.Windows.Forms.TabPage();
			m_lvLog = new System.Windows.Forms.ListView();
			m_pgNames = new System.Windows.Forms.TabPage();
			m_gridMain = new System.Windows.Forms.DataGridView();
			Index = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			Column4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			columnHeader1 = new System.Windows.Forms.ColumnHeader();
			columnHeader2 = new System.Windows.Forms.ColumnHeader();
			m_tabControl.SuspendLayout();
			m_pgLog.SuspendLayout();
			m_pgNames.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)m_gridMain).BeginInit();
			SuspendLayout();
			// 
			// columnHeader1
			// 
			columnHeader1.Text = "Time";
			// 
			// columnHeader2
			// 
			columnHeader2.Text = "Message";
			// 
			// m_lbTitle
			// 
			m_lbTitle.AutoSize = true;
			m_lbTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, 0);
			m_lbTitle.Location = new System.Drawing.Point(4, 10);
			m_lbTitle.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			m_lbTitle.Name = "m_lbTitle";
			m_lbTitle.Size = new System.Drawing.Size(111, 24);
			m_lbTitle.TabIndex = 1;
			m_lbTitle.Text = "Dashboard";
			// 
			// mLoadTimer
			// 
			mLoadTimer.Interval = 50;
			mLoadTimer.Tick += mLoadTimer_Tick;
			// 
			// m_tabControl
			// 
			m_tabControl.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_tabControl.Controls.Add(m_pgLog);
			m_tabControl.Controls.Add(m_pgNames);
			m_tabControl.Location = new System.Drawing.Point(4, 37);
			m_tabControl.Name = "m_tabControl";
			m_tabControl.SelectedIndex = 0;
			m_tabControl.Size = new System.Drawing.Size(381, 239);
			m_tabControl.TabIndex = 2;
			// 
			// m_pgLog
			// 
			m_pgLog.Controls.Add(m_lvLog);
			m_pgLog.Location = new System.Drawing.Point(4, 24);
			m_pgLog.Name = "m_pgLog";
			m_pgLog.Padding = new System.Windows.Forms.Padding(3);
			m_pgLog.Size = new System.Drawing.Size(373, 211);
			m_pgLog.TabIndex = 0;
			m_pgLog.Text = "Log";
			m_pgLog.UseVisualStyleBackColor = true;
			// 
			// m_lvLog
			// 
			m_lvLog.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { columnHeader1, columnHeader2 });
			m_lvLog.Dock = System.Windows.Forms.DockStyle.Fill;
			m_lvLog.FullRowSelect = true;
			m_lvLog.GridLines = true;
			m_lvLog.Location = new System.Drawing.Point(3, 3);
			m_lvLog.MultiSelect = false;
			m_lvLog.Name = "m_lvLog";
			m_lvLog.Size = new System.Drawing.Size(367, 205);
			m_lvLog.TabIndex = 0;
			m_lvLog.UseCompatibleStateImageBehavior = false;
			m_lvLog.View = System.Windows.Forms.View.Details;
			// 
			// m_pgNames
			// 
			m_pgNames.Controls.Add(m_gridMain);
			m_pgNames.Location = new System.Drawing.Point(4, 24);
			m_pgNames.Name = "m_pgNames";
			m_pgNames.Padding = new System.Windows.Forms.Padding(3);
			m_pgNames.Size = new System.Drawing.Size(373, 211);
			m_pgNames.TabIndex = 1;
			m_pgNames.Text = "Names";
			m_pgNames.UseVisualStyleBackColor = true;
			m_pgNames.Enter += m_pgNames_Enter;
			// 
			// m_gridMain
			// 
			m_gridMain.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			m_gridMain.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] { Index, Column1, Column2, Column5, Column4 });
			m_gridMain.Dock = System.Windows.Forms.DockStyle.Fill;
			m_gridMain.Location = new System.Drawing.Point(3, 3);
			m_gridMain.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			m_gridMain.MultiSelect = false;
			m_gridMain.Name = "m_gridMain";
			m_gridMain.ReadOnly = true;
			m_gridMain.RowHeadersVisible = false;
			m_gridMain.Size = new System.Drawing.Size(367, 205);
			m_gridMain.TabIndex = 1;
			// 
			// Index
			// 
			Index.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
			Index.FillWeight = 20F;
			Index.HeaderText = "Index";
			Index.MinimumWidth = 20;
			Index.Name = "Index";
			Index.ReadOnly = true;
			Index.Width = 61;
			// 
			// Column1
			// 
			Column1.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
			Column1.FillWeight = 20F;
			Column1.HeaderText = "Pos";
			Column1.Name = "Column1";
			Column1.ReadOnly = true;
			Column1.Width = 51;
			// 
			// Column2
			// 
			Column2.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
			Column2.FillWeight = 20F;
			Column2.HeaderText = "Cluster";
			Column2.Name = "Column2";
			Column2.ReadOnly = true;
			Column2.Width = 69;
			// 
			// Column5
			// 
			Column5.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.AllCells;
			Column5.FillWeight = 20F;
			Column5.HeaderText = "Length";
			Column5.Name = "Column5";
			Column5.ReadOnly = true;
			Column5.Width = 69;
			// 
			// Column4
			// 
			Column4.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
			Column4.HeaderText = "Name";
			Column4.Name = "Column4";
			Column4.ReadOnly = true;
			// 
			// DashboardUserControl
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
			Controls.Add(m_tabControl);
			Controls.Add(m_lbTitle);
			Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			Name = "DashboardUserControl";
			Size = new System.Drawing.Size(388, 279);
			m_tabControl.ResumeLayout(false);
			m_pgLog.ResumeLayout(false);
			m_pgNames.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)m_gridMain).EndInit();
			ResumeLayout(false);
			PerformLayout();

		}

		#endregion
		private System.Windows.Forms.Label m_lbTitle;
        private System.Windows.Forms.Timer mLoadTimer;
		private System.Windows.Forms.TabControl m_tabControl;
		private System.Windows.Forms.TabPage m_pgLog;
		private System.Windows.Forms.TabPage m_pgNames;
		private System.Windows.Forms.DataGridView m_gridMain;
		private System.Windows.Forms.DataGridViewTextBoxColumn Index;
		private System.Windows.Forms.DataGridViewTextBoxColumn Column1;
		private System.Windows.Forms.DataGridViewTextBoxColumn Column2;
		private System.Windows.Forms.DataGridViewTextBoxColumn Column5;
		private System.Windows.Forms.DataGridViewTextBoxColumn Column4;
		private System.Windows.Forms.ListView m_lvLog;
	}
}
