namespace SharpTerminal.Forms
{
	partial class EmulatorDashboardUserControl
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
			System.Windows.Forms.Label m_lbTitle;
			System.Windows.Forms.GroupBox groupBox1;
			System.Windows.Forms.ColumnHeader columnHeader1;
			System.Windows.Forms.ColumnHeader columnHeader2;
			m_lvEmulators = new System.Windows.Forms.ListView();
			emulatorBindingSource = new System.Windows.Forms.BindingSource(components);
			m_lbTitle = new System.Windows.Forms.Label();
			groupBox1 = new System.Windows.Forms.GroupBox();
			columnHeader1 = new System.Windows.Forms.ColumnHeader();
			columnHeader2 = new System.Windows.Forms.ColumnHeader();
			groupBox1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)emulatorBindingSource).BeginInit();
			SuspendLayout();
			// 
			// m_lbTitle
			// 
			m_lbTitle.AutoSize = true;
			m_lbTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, 0);
			m_lbTitle.Location = new System.Drawing.Point(4, 0);
			m_lbTitle.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			m_lbTitle.Name = "m_lbTitle";
			m_lbTitle.Size = new System.Drawing.Size(103, 24);
			m_lbTitle.TabIndex = 2;
			m_lbTitle.Text = "Emulators";
			// 
			// groupBox1
			// 
			groupBox1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			groupBox1.Controls.Add(m_lvEmulators);
			groupBox1.Location = new System.Drawing.Point(4, 27);
			groupBox1.Name = "groupBox1";
			groupBox1.Size = new System.Drawing.Size(466, 437);
			groupBox1.TabIndex = 4;
			groupBox1.TabStop = false;
			groupBox1.Text = "Running";
			// 
			// m_lvEmulators
			// 
			m_lvEmulators.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_lvEmulators.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { columnHeader1, columnHeader2 });
			m_lvEmulators.FullRowSelect = true;
			m_lvEmulators.GridLines = true;
			m_lvEmulators.Location = new System.Drawing.Point(6, 22);
			m_lvEmulators.Name = "m_lvEmulators";
			m_lvEmulators.Size = new System.Drawing.Size(454, 409);
			m_lvEmulators.TabIndex = 3;
			m_lvEmulators.UseCompatibleStateImageBehavior = false;
			m_lvEmulators.View = System.Windows.Forms.View.Details;
			// 
			// emulatorBindingSource
			// 
			emulatorBindingSource.DataSource = typeof(Emulator);
			// 
			// columnHeader1
			// 
			columnHeader1.Text = "Name";
			columnHeader1.Width = 180;
			// 
			// columnHeader2
			// 
			columnHeader2.Text = "Status";
			columnHeader2.Width = 80;
			// 
			// EmulatorDashboardUserControl
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			Controls.Add(groupBox1);
			Controls.Add(m_lbTitle);
			Name = "EmulatorDashboardUserControl";
			Size = new System.Drawing.Size(473, 467);
			groupBox1.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)emulatorBindingSource).EndInit();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.ListView m_lvEmulators;
		private System.Windows.Forms.BindingSource emulatorBindingSource;
	}
}
