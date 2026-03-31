namespace SharpTerminal.Forms
{
	partial class RemoteIndustryUserControl
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

				if(mIndustry != null)
				{
					mIndustry.PropertyChanged -= Industry_PropertyChanged;
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
			System.Windows.Forms.Label label4;
			System.Windows.Forms.Label label3;
			System.Windows.Forms.Label label2;
			System.Windows.Forms.Label label1;
			tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			m_tbStatus = new System.Windows.Forms.TextBox();
			m_tbQuantity = new System.Windows.Forms.TextBox();
			m_tbDailyRate = new System.Windows.Forms.TextBox();
			m_tbCargo = new System.Windows.Forms.TextBox();
			m_lbTitle = new System.Windows.Forms.Label();
			groupBox1 = new System.Windows.Forms.GroupBox();
			label4 = new System.Windows.Forms.Label();
			label3 = new System.Windows.Forms.Label();
			label2 = new System.Windows.Forms.Label();
			label1 = new System.Windows.Forms.Label();
			groupBox1.SuspendLayout();
			tableLayoutPanel1.SuspendLayout();
			SuspendLayout();
			// 
			// groupBox1
			// 
			groupBox1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			groupBox1.Controls.Add(tableLayoutPanel1);
			groupBox1.Location = new System.Drawing.Point(4, 27);
			groupBox1.Name = "groupBox1";
			groupBox1.Size = new System.Drawing.Size(296, 166);
			groupBox1.TabIndex = 3;
			groupBox1.TabStop = false;
			groupBox1.Text = "Status";
			// 
			// tableLayoutPanel1
			// 
			tableLayoutPanel1.ColumnCount = 2;
			tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 80F));
			tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			tableLayoutPanel1.Controls.Add(m_tbStatus, 1, 3);
			tableLayoutPanel1.Controls.Add(label4, 0, 3);
			tableLayoutPanel1.Controls.Add(m_tbQuantity, 1, 2);
			tableLayoutPanel1.Controls.Add(label3, 0, 2);
			tableLayoutPanel1.Controls.Add(m_tbDailyRate, 1, 1);
			tableLayoutPanel1.Controls.Add(label2, 0, 1);
			tableLayoutPanel1.Controls.Add(label1, 0, 0);
			tableLayoutPanel1.Controls.Add(m_tbCargo, 1, 0);
			tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			tableLayoutPanel1.Location = new System.Drawing.Point(3, 19);
			tableLayoutPanel1.Name = "tableLayoutPanel1";
			tableLayoutPanel1.RowCount = 4;
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel1.Size = new System.Drawing.Size(290, 144);
			tableLayoutPanel1.TabIndex = 0;
			// 
			// m_tbStatus
			// 
			m_tbStatus.Dock = System.Windows.Forms.DockStyle.Fill;
			m_tbStatus.Location = new System.Drawing.Point(83, 90);
			m_tbStatus.Multiline = true;
			m_tbStatus.Name = "m_tbStatus";
			m_tbStatus.ReadOnly = true;
			m_tbStatus.Size = new System.Drawing.Size(204, 51);
			m_tbStatus.TabIndex = 7;
			// 
			// label4
			// 
			label4.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label4.AutoSize = true;
			label4.Location = new System.Drawing.Point(35, 108);
			label4.Name = "label4";
			label4.Size = new System.Drawing.Size(42, 15);
			label4.TabIndex = 6;
			label4.Text = "Status:";
			// 
			// m_tbQuantity
			// 
			m_tbQuantity.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_tbQuantity.Location = new System.Drawing.Point(83, 61);
			m_tbQuantity.Name = "m_tbQuantity";
			m_tbQuantity.ReadOnly = true;
			m_tbQuantity.Size = new System.Drawing.Size(204, 23);
			m_tbQuantity.TabIndex = 5;
			// 
			// label3
			// 
			label3.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label3.AutoSize = true;
			label3.Location = new System.Drawing.Point(21, 65);
			label3.Name = "label3";
			label3.Size = new System.Drawing.Size(56, 15);
			label3.TabIndex = 4;
			label3.Text = "Quantity:";
			// 
			// m_tbDailyRate
			// 
			m_tbDailyRate.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_tbDailyRate.Location = new System.Drawing.Point(83, 32);
			m_tbDailyRate.Name = "m_tbDailyRate";
			m_tbDailyRate.ReadOnly = true;
			m_tbDailyRate.Size = new System.Drawing.Size(204, 23);
			m_tbDailyRate.TabIndex = 3;
			// 
			// label2
			// 
			label2.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label2.AutoSize = true;
			label2.Location = new System.Drawing.Point(15, 36);
			label2.Name = "label2";
			label2.Size = new System.Drawing.Size(62, 15);
			label2.TabIndex = 2;
			label2.Text = "Daily Rate:";
			// 
			// label1
			// 
			label1.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label1.AutoSize = true;
			label1.Location = new System.Drawing.Point(35, 7);
			label1.Name = "label1";
			label1.Size = new System.Drawing.Size(42, 15);
			label1.TabIndex = 0;
			label1.Text = "Cargo:";
			// 
			// m_tbCargo
			// 
			m_tbCargo.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_tbCargo.Location = new System.Drawing.Point(83, 3);
			m_tbCargo.Name = "m_tbCargo";
			m_tbCargo.ReadOnly = true;
			m_tbCargo.Size = new System.Drawing.Size(204, 23);
			m_tbCargo.TabIndex = 1;
			// 
			// m_lbTitle
			// 
			m_lbTitle.AutoSize = true;
			m_lbTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, 0);
			m_lbTitle.Location = new System.Drawing.Point(4, 0);
			m_lbTitle.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			m_lbTitle.Name = "m_lbTitle";
			m_lbTitle.Size = new System.Drawing.Size(102, 24);
			m_lbTitle.TabIndex = 2;
			m_lbTitle.Text = "Industry - ";
			// 
			// RemoteIndustryUserControl
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			Controls.Add(groupBox1);
			Controls.Add(m_lbTitle);
			Name = "RemoteIndustryUserControl";
			Size = new System.Drawing.Size(303, 231);
			groupBox1.ResumeLayout(false);
			tableLayoutPanel1.ResumeLayout(false);
			tableLayoutPanel1.PerformLayout();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.Label m_lbTitle;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox m_tbCargo;
		private System.Windows.Forms.TextBox m_tbDailyRate;
		private System.Windows.Forms.TextBox m_tbQuantity;
		private System.Windows.Forms.TextBox m_tbStatus;
	}
}
