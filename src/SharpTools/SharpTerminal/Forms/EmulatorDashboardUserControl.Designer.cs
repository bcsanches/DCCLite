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
			if (mLastSelected != null)
			{
				mLastSelected.AppendOutput -= OnEmulatorOutputAppended;
			}

			if (disposing)
			{				 
				components?.Dispose();
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
			System.Windows.Forms.SplitContainer splitContainer1;
			System.Windows.Forms.TabControl tabControl1;
			System.Windows.Forms.TabPage tabPage1;
			System.Windows.Forms.TabPage Output;
			m_lvEmulators = new System.Windows.Forms.ListView();
			m_tbEmulatorOutput = new System.Windows.Forms.TextBox();
			emulatorBindingSource = new System.Windows.Forms.BindingSource(components);
			m_btnKill = new System.Windows.Forms.Button();
			m_btnRestart = new System.Windows.Forms.Button();
			m_lbTitle = new System.Windows.Forms.Label();
			groupBox1 = new System.Windows.Forms.GroupBox();
			columnHeader1 = new System.Windows.Forms.ColumnHeader();
			columnHeader2 = new System.Windows.Forms.ColumnHeader();
			splitContainer1 = new System.Windows.Forms.SplitContainer();
			tabControl1 = new System.Windows.Forms.TabControl();
			tabPage1 = new System.Windows.Forms.TabPage();
			Output = new System.Windows.Forms.TabPage();
			groupBox1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
			splitContainer1.Panel1.SuspendLayout();
			splitContainer1.Panel2.SuspendLayout();
			splitContainer1.SuspendLayout();
			tabControl1.SuspendLayout();
			Output.SuspendLayout();
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
			groupBox1.Controls.Add(m_lvEmulators);
			groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			groupBox1.Location = new System.Drawing.Point(0, 0);
			groupBox1.Name = "groupBox1";
			groupBox1.Size = new System.Drawing.Size(466, 202);
			groupBox1.TabIndex = 4;
			groupBox1.TabStop = false;
			groupBox1.Text = "Running";
			// 
			// m_lvEmulators
			// 
			m_lvEmulators.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { columnHeader1, columnHeader2 });
			m_lvEmulators.Dock = System.Windows.Forms.DockStyle.Fill;
			m_lvEmulators.FullRowSelect = true;
			m_lvEmulators.GridLines = true;
			m_lvEmulators.Location = new System.Drawing.Point(3, 19);
			m_lvEmulators.MultiSelect = false;
			m_lvEmulators.Name = "m_lvEmulators";
			m_lvEmulators.Size = new System.Drawing.Size(460, 180);
			m_lvEmulators.TabIndex = 3;
			m_lvEmulators.UseCompatibleStateImageBehavior = false;
			m_lvEmulators.View = System.Windows.Forms.View.Details;
			m_lvEmulators.SelectedIndexChanged += m_lvEmulators_SelectedIndexChanged;
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
			// splitContainer1
			// 
			splitContainer1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			splitContainer1.Location = new System.Drawing.Point(4, 27);
			splitContainer1.Name = "splitContainer1";
			splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitContainer1.Panel1
			// 
			splitContainer1.Panel1.Controls.Add(groupBox1);
			// 
			// splitContainer1.Panel2
			// 
			splitContainer1.Panel2.Controls.Add(tabControl1);
			splitContainer1.Size = new System.Drawing.Size(466, 404);
			splitContainer1.SplitterDistance = 202;
			splitContainer1.TabIndex = 7;
			// 
			// tabControl1
			// 
			tabControl1.Controls.Add(tabPage1);
			tabControl1.Controls.Add(Output);
			tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
			tabControl1.Location = new System.Drawing.Point(0, 0);
			tabControl1.Name = "tabControl1";
			tabControl1.SelectedIndex = 0;
			tabControl1.Size = new System.Drawing.Size(466, 198);
			tabControl1.TabIndex = 0;
			// 
			// tabPage1
			// 
			tabPage1.Location = new System.Drawing.Point(4, 24);
			tabPage1.Name = "tabPage1";
			tabPage1.Padding = new System.Windows.Forms.Padding(3);
			tabPage1.Size = new System.Drawing.Size(458, 170);
			tabPage1.TabIndex = 0;
			tabPage1.Text = "Pins";
			tabPage1.UseVisualStyleBackColor = true;
			// 
			// Output
			// 
			Output.Controls.Add(m_tbEmulatorOutput);
			Output.Location = new System.Drawing.Point(4, 24);
			Output.Name = "Output";
			Output.Padding = new System.Windows.Forms.Padding(3);
			Output.Size = new System.Drawing.Size(458, 172);
			Output.TabIndex = 1;
			Output.Text = "Output";
			Output.UseVisualStyleBackColor = true;
			// 
			// m_tbEmulatorOutput
			// 
			m_tbEmulatorOutput.Dock = System.Windows.Forms.DockStyle.Fill;
			m_tbEmulatorOutput.Location = new System.Drawing.Point(3, 3);
			m_tbEmulatorOutput.Multiline = true;
			m_tbEmulatorOutput.Name = "m_tbEmulatorOutput";
			m_tbEmulatorOutput.ReadOnly = true;
			m_tbEmulatorOutput.Size = new System.Drawing.Size(452, 166);
			m_tbEmulatorOutput.TabIndex = 0;
			// 
			// emulatorBindingSource
			// 
			emulatorBindingSource.DataSource = typeof(Emulator);
			// 
			// m_btnKill
			// 
			m_btnKill.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnKill.Enabled = false;
			m_btnKill.Location = new System.Drawing.Point(4, 437);
			m_btnKill.Name = "m_btnKill";
			m_btnKill.RightToLeft = System.Windows.Forms.RightToLeft.No;
			m_btnKill.Size = new System.Drawing.Size(75, 27);
			m_btnKill.TabIndex = 5;
			m_btnKill.Text = "&Kill";
			m_btnKill.UseVisualStyleBackColor = true;
			m_btnKill.Click += m_btnKill_Click;
			// 
			// m_btnRestart
			// 
			m_btnRestart.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnRestart.Enabled = false;
			m_btnRestart.Location = new System.Drawing.Point(85, 437);
			m_btnRestart.Name = "m_btnRestart";
			m_btnRestart.Size = new System.Drawing.Size(75, 27);
			m_btnRestart.TabIndex = 6;
			m_btnRestart.Text = "&Restart";
			m_btnRestart.UseVisualStyleBackColor = true;
			m_btnRestart.Click += m_btnRestart_Click;
			// 
			// EmulatorDashboardUserControl
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			Controls.Add(splitContainer1);
			Controls.Add(m_btnRestart);
			Controls.Add(m_btnKill);
			Controls.Add(m_lbTitle);
			Name = "EmulatorDashboardUserControl";
			Size = new System.Drawing.Size(473, 467);
			groupBox1.ResumeLayout(false);
			splitContainer1.Panel1.ResumeLayout(false);
			splitContainer1.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
			splitContainer1.ResumeLayout(false);
			tabControl1.ResumeLayout(false);
			Output.ResumeLayout(false);
			Output.PerformLayout();
			((System.ComponentModel.ISupportInitialize)emulatorBindingSource).EndInit();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.ListView m_lvEmulators;
		private System.Windows.Forms.BindingSource emulatorBindingSource;
		private System.Windows.Forms.Button m_btnKill;
		private System.Windows.Forms.Button m_btnRestart;				
		private System.Windows.Forms.TextBox m_tbEmulatorOutput;
	}
}
