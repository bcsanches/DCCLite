namespace SharpDude
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
            if (disposing)
            {
                components?.Dispose();

                mArrivalWatcher.Dispose();
                mRemovalWatcher.Dispose();

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
			GroupBox groupBox1;
			Label label1;
			GroupBox groupBox2;
			Label label5;
			Label label4;
			Label label3;
			Label label2;
			StatusStrip mStatusBar;
			tableLayoutPanel1 = new TableLayoutPanel();
			tbAvrDude = new TextBox();
			tableLayoutPanel2 = new TableLayoutPanel();
			tbVersion = new TextBox();
			cbComPorts = new ComboBox();
			cbArduinoTypes = new ComboBox();
			arduinoBindingSource = new BindingSource(components);
			tbImageName = new TextBox();
			btnBurn = new Button();
			btnExit = new Button();
			tbOutput = new TextBox();
			m_lblStatus = new ToolStripStatusLabel();
			groupBox1 = new GroupBox();
			label1 = new Label();
			groupBox2 = new GroupBox();
			label5 = new Label();
			label4 = new Label();
			label3 = new Label();
			label2 = new Label();
			mStatusBar = new StatusStrip();
			groupBox1.SuspendLayout();
			tableLayoutPanel1.SuspendLayout();
			groupBox2.SuspendLayout();
			tableLayoutPanel2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)arduinoBindingSource).BeginInit();
			mStatusBar.SuspendLayout();
			SuspendLayout();
			// 
			// groupBox1
			// 
			groupBox1.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
			groupBox1.Controls.Add(tableLayoutPanel1);
			groupBox1.Location = new Point(14, 14);
			groupBox1.Margin = new Padding(4, 3, 4, 3);
			groupBox1.Name = "groupBox1";
			groupBox1.Padding = new Padding(4, 3, 4, 3);
			groupBox1.Size = new Size(677, 75);
			groupBox1.TabIndex = 0;
			groupBox1.TabStop = false;
			groupBox1.Text = "AVRDude";
			// 
			// tableLayoutPanel1
			// 
			tableLayoutPanel1.ColumnCount = 2;
			tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 8.710801F));
			tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 91.2892F));
			tableLayoutPanel1.Controls.Add(tbAvrDude, 1, 0);
			tableLayoutPanel1.Controls.Add(label1, 0, 0);
			tableLayoutPanel1.Dock = DockStyle.Fill;
			tableLayoutPanel1.Location = new Point(4, 19);
			tableLayoutPanel1.Margin = new Padding(4, 3, 4, 3);
			tableLayoutPanel1.Name = "tableLayoutPanel1";
			tableLayoutPanel1.RowCount = 1;
			tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 50F));
			tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Absolute, 50F));
			tableLayoutPanel1.Size = new Size(669, 53);
			tableLayoutPanel1.TabIndex = 2;
			// 
			// tbAvrDude
			// 
			tbAvrDude.Anchor = AnchorStyles.Left | AnchorStyles.Right;
			tbAvrDude.Location = new Point(62, 15);
			tbAvrDude.Margin = new Padding(4, 3, 4, 3);
			tbAvrDude.Name = "tbAvrDude";
			tbAvrDude.ReadOnly = true;
			tbAvrDude.Size = new Size(603, 23);
			tbAvrDude.TabIndex = 1;
			// 
			// label1
			// 
			label1.Anchor = AnchorStyles.Right;
			label1.AutoSize = true;
			label1.Location = new Point(20, 19);
			label1.Margin = new Padding(4, 0, 4, 0);
			label1.Name = "label1";
			label1.Size = new Size(34, 15);
			label1.TabIndex = 0;
			label1.Text = "Type:";
			// 
			// groupBox2
			// 
			groupBox2.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
			groupBox2.Controls.Add(tableLayoutPanel2);
			groupBox2.Location = new Point(14, 96);
			groupBox2.Margin = new Padding(4, 3, 4, 3);
			groupBox2.Name = "groupBox2";
			groupBox2.Padding = new Padding(4, 3, 4, 3);
			groupBox2.Size = new Size(677, 117);
			groupBox2.TabIndex = 1;
			groupBox2.TabStop = false;
			groupBox2.Text = "Arduino";
			// 
			// tableLayoutPanel2
			// 
			tableLayoutPanel2.ColumnCount = 4;
			tableLayoutPanel2.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 12.77641F));
			tableLayoutPanel2.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 87.22359F));
			tableLayoutPanel2.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 75F));
			tableLayoutPanel2.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 148F));
			tableLayoutPanel2.Controls.Add(tbVersion, 3, 1);
			tableLayoutPanel2.Controls.Add(label5, 2, 1);
			tableLayoutPanel2.Controls.Add(label4, 0, 1);
			tableLayoutPanel2.Controls.Add(cbComPorts, 3, 0);
			tableLayoutPanel2.Controls.Add(label3, 2, 0);
			tableLayoutPanel2.Controls.Add(label2, 0, 0);
			tableLayoutPanel2.Controls.Add(cbArduinoTypes, 1, 0);
			tableLayoutPanel2.Controls.Add(tbImageName, 1, 1);
			tableLayoutPanel2.Dock = DockStyle.Fill;
			tableLayoutPanel2.Location = new Point(4, 19);
			tableLayoutPanel2.Margin = new Padding(4, 3, 4, 3);
			tableLayoutPanel2.Name = "tableLayoutPanel2";
			tableLayoutPanel2.RowCount = 2;
			tableLayoutPanel2.RowStyles.Add(new RowStyle(SizeType.Percent, 50F));
			tableLayoutPanel2.RowStyles.Add(new RowStyle(SizeType.Percent, 50F));
			tableLayoutPanel2.Size = new Size(669, 95);
			tableLayoutPanel2.TabIndex = 0;
			// 
			// tbVersion
			// 
			tbVersion.Anchor = AnchorStyles.Left | AnchorStyles.Right;
			tbVersion.Location = new Point(524, 59);
			tbVersion.Margin = new Padding(4, 3, 4, 3);
			tbVersion.Name = "tbVersion";
			tbVersion.ReadOnly = true;
			tbVersion.Size = new Size(141, 23);
			tbVersion.TabIndex = 9;
			// 
			// label5
			// 
			label5.Anchor = AnchorStyles.Right;
			label5.AutoSize = true;
			label5.Location = new Point(468, 63);
			label5.Margin = new Padding(4, 0, 4, 0);
			label5.Name = "label5";
			label5.Size = new Size(48, 15);
			label5.TabIndex = 8;
			label5.Text = "Version:";
			// 
			// label4
			// 
			label4.Anchor = AnchorStyles.Right;
			label4.AutoSize = true;
			label4.Location = new Point(9, 63);
			label4.Margin = new Padding(4, 0, 4, 0);
			label4.Name = "label4";
			label4.Size = new Size(43, 15);
			label4.TabIndex = 6;
			label4.Text = "Image:";
			// 
			// cbComPorts
			// 
			cbComPorts.Anchor = AnchorStyles.Left | AnchorStyles.Right;
			cbComPorts.FormattingEnabled = true;
			cbComPorts.Location = new Point(524, 12);
			cbComPorts.Margin = new Padding(4, 3, 4, 3);
			cbComPorts.Name = "cbComPorts";
			cbComPorts.Size = new Size(141, 23);
			cbComPorts.TabIndex = 5;
			// 
			// label3
			// 
			label3.Anchor = AnchorStyles.Right;
			label3.AutoSize = true;
			label3.Location = new Point(484, 16);
			label3.Margin = new Padding(4, 0, 4, 0);
			label3.Name = "label3";
			label3.Size = new Size(32, 15);
			label3.TabIndex = 4;
			label3.Text = "Port:";
			// 
			// label2
			// 
			label2.Anchor = AnchorStyles.Right;
			label2.AutoSize = true;
			label2.Location = new Point(8, 16);
			label2.Margin = new Padding(4, 0, 4, 0);
			label2.Name = "label2";
			label2.Size = new Size(44, 15);
			label2.TabIndex = 2;
			label2.Text = "Model:";
			// 
			// cbArduinoTypes
			// 
			cbArduinoTypes.Anchor = AnchorStyles.Left | AnchorStyles.Right;
			cbArduinoTypes.DataBindings.Add(new Binding("Text", arduinoBindingSource, "BoardName", true));
			cbArduinoTypes.FormattingEnabled = true;
			cbArduinoTypes.Location = new Point(60, 12);
			cbArduinoTypes.Margin = new Padding(4, 3, 4, 3);
			cbArduinoTypes.Name = "cbArduinoTypes";
			cbArduinoTypes.Size = new Size(381, 23);
			cbArduinoTypes.TabIndex = 3;
			cbArduinoTypes.SelectedIndexChanged += cbArduinoTypes_SelectedIndexChanged;
			// 
			// arduinoBindingSource
			// 
			arduinoBindingSource.DataSource = typeof(Arduino);
			// 
			// tbImageName
			// 
			tbImageName.Anchor = AnchorStyles.Left | AnchorStyles.Right;
			tbImageName.Location = new Point(60, 59);
			tbImageName.Margin = new Padding(4, 3, 4, 3);
			tbImageName.Name = "tbImageName";
			tbImageName.ReadOnly = true;
			tbImageName.Size = new Size(381, 23);
			tbImageName.TabIndex = 7;
			// 
			// btnBurn
			// 
			btnBurn.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
			btnBurn.Enabled = false;
			btnBurn.Location = new Point(604, 548);
			btnBurn.Margin = new Padding(4, 3, 4, 3);
			btnBurn.Name = "btnBurn";
			btnBurn.Size = new Size(88, 27);
			btnBurn.TabIndex = 2;
			btnBurn.Text = "&Burn";
			btnBurn.UseVisualStyleBackColor = true;
			btnBurn.Click += btnBurn_Click;
			// 
			// btnExit
			// 
			btnExit.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
			btnExit.DialogResult = DialogResult.Cancel;
			btnExit.Location = new Point(13, 548);
			btnExit.Margin = new Padding(4, 3, 4, 3);
			btnExit.Name = "btnExit";
			btnExit.Size = new Size(88, 27);
			btnExit.TabIndex = 3;
			btnExit.Text = "E&xit";
			btnExit.UseVisualStyleBackColor = true;
			btnExit.Click += btnExit_Click;
			// 
			// tbOutput
			// 
			tbOutput.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
			tbOutput.BackColor = Color.Blue;
			tbOutput.Font = new Font("Consolas", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
			tbOutput.ForeColor = Color.White;
			tbOutput.Location = new Point(14, 219);
			tbOutput.Margin = new Padding(4, 3, 4, 3);
			tbOutput.Multiline = true;
			tbOutput.Name = "tbOutput";
			tbOutput.ReadOnly = true;
			tbOutput.ScrollBars = ScrollBars.Vertical;
			tbOutput.Size = new Size(676, 323);
			tbOutput.TabIndex = 4;
			// 
			// mStatusBar
			// 
			mStatusBar.Items.AddRange(new ToolStripItem[] { m_lblStatus });
			mStatusBar.Location = new Point(0, 578);
			mStatusBar.Name = "mStatusBar";
			mStatusBar.Size = new Size(705, 22);
			mStatusBar.TabIndex = 5;
			mStatusBar.Text = "statusStrip1";
			// 
			// m_lblStatus
			// 
			m_lblStatus.Name = "m_lblStatus";
			m_lblStatus.Size = new Size(118, 17);
			m_lblStatus.Text = "toolStripStatusLabel1";
			// 
			// MainForm
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			CancelButton = btnExit;
			ClientSize = new Size(705, 600);
			Controls.Add(mStatusBar);
			Controls.Add(tbOutput);
			Controls.Add(btnExit);
			Controls.Add(btnBurn);
			Controls.Add(groupBox2);
			Controls.Add(groupBox1);
			FormBorderStyle = FormBorderStyle.FixedToolWindow;
			Margin = new Padding(4, 3, 4, 3);
			MaximizeBox = false;
			Name = "MainForm";
			Text = "SharpDude";
			groupBox1.ResumeLayout(false);
			tableLayoutPanel1.ResumeLayout(false);
			tableLayoutPanel1.PerformLayout();
			groupBox2.ResumeLayout(false);
			tableLayoutPanel2.ResumeLayout(false);
			tableLayoutPanel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)arduinoBindingSource).EndInit();
			mStatusBar.ResumeLayout(false);
			mStatusBar.PerformLayout();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.TextBox tbAvrDude;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
        private System.Windows.Forms.TextBox tbVersion;
        private System.Windows.Forms.ComboBox cbComPorts;
        private System.Windows.Forms.ComboBox cbArduinoTypes;
        private System.Windows.Forms.TextBox tbImageName;
        private System.Windows.Forms.Button btnBurn;
        private System.Windows.Forms.Button btnExit;
        private System.Windows.Forms.BindingSource arduinoBindingSource;
        private System.Windows.Forms.TextBox tbOutput;
		private StatusStrip mStatusBar;
		private ToolStripStatusLabel m_lblStatus;
	}
}