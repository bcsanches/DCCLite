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
			System.Windows.Forms.GroupBox groupBox2;
			System.Windows.Forms.Label label5;
			System.Windows.Forms.Label label6;
			System.Windows.Forms.Label label7;
			System.Windows.Forms.Label label8;
			System.Windows.Forms.GroupBox Production;
			tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			m_tbStatus = new System.Windows.Forms.TextBox();
			m_tbQuantity = new System.Windows.Forms.TextBox();
			m_tbDailyRate = new System.Windows.Forms.TextBox();
			tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			m_lnkSpotActionAux = new System.Windows.Forms.LinkLabel();
			m_tbSpotInfo = new System.Windows.Forms.TextBox();
			m_tbSpotState = new System.Windows.Forms.TextBox();
			m_cbSpot = new System.Windows.Forms.ComboBox();
			m_lnkSpotAction = new System.Windows.Forms.LinkLabel();
			m_lbTitle = new System.Windows.Forms.Label();
			m_lvProduction = new System.Windows.Forms.ListView();
			columnHeader1 = new System.Windows.Forms.ColumnHeader();
			columnHeader2 = new System.Windows.Forms.ColumnHeader();
			columnHeader3 = new System.Windows.Forms.ColumnHeader();
			columnHeader4 = new System.Windows.Forms.ColumnHeader();
			groupBox1 = new System.Windows.Forms.GroupBox();
			label4 = new System.Windows.Forms.Label();
			label3 = new System.Windows.Forms.Label();
			label2 = new System.Windows.Forms.Label();
			groupBox2 = new System.Windows.Forms.GroupBox();
			label5 = new System.Windows.Forms.Label();
			label6 = new System.Windows.Forms.Label();
			label7 = new System.Windows.Forms.Label();
			label8 = new System.Windows.Forms.Label();
			Production = new System.Windows.Forms.GroupBox();
			groupBox1.SuspendLayout();
			tableLayoutPanel1.SuspendLayout();
			groupBox2.SuspendLayout();
			tableLayoutPanel2.SuspendLayout();
			Production.SuspendLayout();
			SuspendLayout();
			// 
			// groupBox1
			// 
			groupBox1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			groupBox1.Controls.Add(tableLayoutPanel1);
			groupBox1.Location = new System.Drawing.Point(4, 27);
			groupBox1.Name = "groupBox1";
			groupBox1.Size = new System.Drawing.Size(296, 109);
			groupBox1.TabIndex = 3;
			groupBox1.TabStop = false;
			groupBox1.Text = "Status";
			// 
			// tableLayoutPanel1
			// 
			tableLayoutPanel1.ColumnCount = 2;
			tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 80F));
			tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
			tableLayoutPanel1.Controls.Add(m_tbStatus, 1, 2);
			tableLayoutPanel1.Controls.Add(label4, 0, 2);
			tableLayoutPanel1.Controls.Add(m_tbQuantity, 1, 1);
			tableLayoutPanel1.Controls.Add(label3, 0, 1);
			tableLayoutPanel1.Controls.Add(m_tbDailyRate, 1, 0);
			tableLayoutPanel1.Controls.Add(label2, 0, 0);
			tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			tableLayoutPanel1.Location = new System.Drawing.Point(3, 19);
			tableLayoutPanel1.Name = "tableLayoutPanel1";
			tableLayoutPanel1.RowCount = 3;
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
			tableLayoutPanel1.Size = new System.Drawing.Size(290, 87);
			tableLayoutPanel1.TabIndex = 0;
			// 
			// m_tbStatus
			// 
			m_tbStatus.Dock = System.Windows.Forms.DockStyle.Fill;
			m_tbStatus.Location = new System.Drawing.Point(83, 61);
			m_tbStatus.Multiline = true;
			m_tbStatus.Name = "m_tbStatus";
			m_tbStatus.ReadOnly = true;
			m_tbStatus.Size = new System.Drawing.Size(204, 23);
			m_tbStatus.TabIndex = 7;
			// 
			// label4
			// 
			label4.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label4.AutoSize = true;
			label4.Location = new System.Drawing.Point(35, 65);
			label4.Name = "label4";
			label4.Size = new System.Drawing.Size(42, 15);
			label4.TabIndex = 6;
			label4.Text = "Status:";
			// 
			// m_tbQuantity
			// 
			m_tbQuantity.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_tbQuantity.Location = new System.Drawing.Point(83, 32);
			m_tbQuantity.Name = "m_tbQuantity";
			m_tbQuantity.ReadOnly = true;
			m_tbQuantity.Size = new System.Drawing.Size(204, 23);
			m_tbQuantity.TabIndex = 5;
			// 
			// label3
			// 
			label3.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label3.AutoSize = true;
			label3.Location = new System.Drawing.Point(21, 36);
			label3.Name = "label3";
			label3.Size = new System.Drawing.Size(56, 15);
			label3.TabIndex = 4;
			label3.Text = "Quantity:";
			// 
			// m_tbDailyRate
			// 
			m_tbDailyRate.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_tbDailyRate.Location = new System.Drawing.Point(83, 3);
			m_tbDailyRate.Name = "m_tbDailyRate";
			m_tbDailyRate.ReadOnly = true;
			m_tbDailyRate.Size = new System.Drawing.Size(204, 23);
			m_tbDailyRate.TabIndex = 3;
			// 
			// label2
			// 
			label2.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label2.AutoSize = true;
			label2.Location = new System.Drawing.Point(15, 7);
			label2.Name = "label2";
			label2.Size = new System.Drawing.Size(62, 15);
			label2.TabIndex = 2;
			label2.Text = "Daily Rate:";
			// 
			// groupBox2
			// 
			groupBox2.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			groupBox2.Controls.Add(tableLayoutPanel2);
			groupBox2.Location = new System.Drawing.Point(3, 239);
			groupBox2.Name = "groupBox2";
			groupBox2.Size = new System.Drawing.Size(297, 150);
			groupBox2.TabIndex = 4;
			groupBox2.TabStop = false;
			groupBox2.Text = "Spots";
			// 
			// tableLayoutPanel2
			// 
			tableLayoutPanel2.ColumnCount = 3;
			tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 80F));
			tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			tableLayoutPanel2.Controls.Add(m_lnkSpotActionAux, 2, 3);
			tableLayoutPanel2.Controls.Add(label5, 0, 3);
			tableLayoutPanel2.Controls.Add(m_tbSpotInfo, 1, 2);
			tableLayoutPanel2.Controls.Add(label6, 0, 2);
			tableLayoutPanel2.Controls.Add(m_tbSpotState, 1, 1);
			tableLayoutPanel2.Controls.Add(label7, 0, 1);
			tableLayoutPanel2.Controls.Add(label8, 0, 0);
			tableLayoutPanel2.Controls.Add(m_cbSpot, 1, 0);
			tableLayoutPanel2.Controls.Add(m_lnkSpotAction, 1, 3);
			tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			tableLayoutPanel2.Location = new System.Drawing.Point(3, 19);
			tableLayoutPanel2.Name = "tableLayoutPanel2";
			tableLayoutPanel2.RowCount = 4;
			tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			tableLayoutPanel2.Size = new System.Drawing.Size(291, 128);
			tableLayoutPanel2.TabIndex = 1;
			// 
			// m_lnkSpotActionAux
			// 
			m_lnkSpotActionAux.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_lnkSpotActionAux.AutoSize = true;
			m_lnkSpotActionAux.Location = new System.Drawing.Point(188, 100);
			m_lnkSpotActionAux.Name = "m_lnkSpotActionAux";
			m_lnkSpotActionAux.Size = new System.Drawing.Size(100, 15);
			m_lnkSpotActionAux.TabIndex = 10;
			m_lnkSpotActionAux.TabStop = true;
			m_lnkSpotActionAux.Text = "linkLabel1";
			m_lnkSpotActionAux.LinkClicked += m_lnkSpotActionAux_LinkClicked;
			// 
			// label5
			// 
			label5.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label5.AutoSize = true;
			label5.Location = new System.Drawing.Point(32, 100);
			label5.Name = "label5";
			label5.Size = new System.Drawing.Size(45, 15);
			label5.TabIndex = 6;
			label5.Text = "Action:";
			// 
			// m_tbSpotInfo
			// 
			m_tbSpotInfo.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			tableLayoutPanel2.SetColumnSpan(m_tbSpotInfo, 2);
			m_tbSpotInfo.Location = new System.Drawing.Point(83, 61);
			m_tbSpotInfo.Name = "m_tbSpotInfo";
			m_tbSpotInfo.ReadOnly = true;
			m_tbSpotInfo.Size = new System.Drawing.Size(205, 23);
			m_tbSpotInfo.TabIndex = 5;
			// 
			// label6
			// 
			label6.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label6.AutoSize = true;
			label6.Location = new System.Drawing.Point(46, 65);
			label6.Name = "label6";
			label6.Size = new System.Drawing.Size(31, 15);
			label6.TabIndex = 4;
			label6.Text = "Info:";
			// 
			// m_tbSpotState
			// 
			m_tbSpotState.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			tableLayoutPanel2.SetColumnSpan(m_tbSpotState, 2);
			m_tbSpotState.Location = new System.Drawing.Point(83, 32);
			m_tbSpotState.Name = "m_tbSpotState";
			m_tbSpotState.ReadOnly = true;
			m_tbSpotState.Size = new System.Drawing.Size(205, 23);
			m_tbSpotState.TabIndex = 3;
			// 
			// label7
			// 
			label7.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label7.AutoSize = true;
			label7.Location = new System.Drawing.Point(41, 36);
			label7.Name = "label7";
			label7.Size = new System.Drawing.Size(36, 15);
			label7.TabIndex = 2;
			label7.Text = "State:";
			// 
			// label8
			// 
			label8.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label8.AutoSize = true;
			label8.Location = new System.Drawing.Point(43, 7);
			label8.Name = "label8";
			label8.Size = new System.Drawing.Size(34, 15);
			label8.TabIndex = 0;
			label8.Text = "Spot:";
			// 
			// m_cbSpot
			// 
			m_cbSpot.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			tableLayoutPanel2.SetColumnSpan(m_cbSpot, 2);
			m_cbSpot.FormattingEnabled = true;
			m_cbSpot.Location = new System.Drawing.Point(83, 3);
			m_cbSpot.Name = "m_cbSpot";
			m_cbSpot.Size = new System.Drawing.Size(205, 23);
			m_cbSpot.TabIndex = 8;
			m_cbSpot.SelectedIndexChanged += m_cbSpot_SelectedIndexChanged;
			// 
			// m_lnkSpotAction
			// 
			m_lnkSpotAction.Anchor = System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_lnkSpotAction.AutoSize = true;
			m_lnkSpotAction.Location = new System.Drawing.Point(83, 100);
			m_lnkSpotAction.Name = "m_lnkSpotAction";
			m_lnkSpotAction.Size = new System.Drawing.Size(99, 15);
			m_lnkSpotAction.TabIndex = 9;
			m_lnkSpotAction.TabStop = true;
			m_lnkSpotAction.Text = "linkLabel1";
			m_lnkSpotAction.LinkClicked += m_lnkSpotAction_LinkClicked;
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
			// Production
			// 
			Production.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			Production.Controls.Add(m_lvProduction);
			Production.Location = new System.Drawing.Point(4, 142);
			Production.Name = "Production";
			Production.Size = new System.Drawing.Size(296, 91);
			Production.TabIndex = 5;
			Production.TabStop = false;
			Production.Text = "Production";
			// 
			// m_lvProduction
			// 
			m_lvProduction.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { columnHeader1, columnHeader2, columnHeader3, columnHeader4 });
			m_lvProduction.Dock = System.Windows.Forms.DockStyle.Fill;
			m_lvProduction.FullRowSelect = true;
			m_lvProduction.Location = new System.Drawing.Point(3, 19);
			m_lvProduction.MultiSelect = false;
			m_lvProduction.Name = "m_lvProduction";
			m_lvProduction.Size = new System.Drawing.Size(290, 69);
			m_lvProduction.TabIndex = 0;
			m_lvProduction.UseCompatibleStateImageBehavior = false;
			m_lvProduction.View = System.Windows.Forms.View.Details;
			// 
			// columnHeader1
			// 
			columnHeader1.Text = "Cargo";
			// 
			// columnHeader2
			// 
			columnHeader2.Text = "Quantity";
			// 
			// columnHeader3
			// 
			columnHeader3.Text = "Reserved";
			// 
			// columnHeader4
			// 
			columnHeader4.Text = "Total";
			// 
			// RemoteIndustryUserControl
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			Controls.Add(Production);
			Controls.Add(groupBox2);
			Controls.Add(groupBox1);
			Controls.Add(m_lbTitle);
			Name = "RemoteIndustryUserControl";
			Size = new System.Drawing.Size(303, 401);
			groupBox1.ResumeLayout(false);
			tableLayoutPanel1.ResumeLayout(false);
			tableLayoutPanel1.PerformLayout();
			groupBox2.ResumeLayout(false);
			tableLayoutPanel2.ResumeLayout(false);
			tableLayoutPanel2.PerformLayout();
			Production.ResumeLayout(false);
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.Label m_lbTitle;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;		
		private System.Windows.Forms.TextBox m_tbDailyRate;
		private System.Windows.Forms.TextBox m_tbQuantity;
		private System.Windows.Forms.TextBox m_tbStatus;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.TextBox m_tbSpotInfo;
		private System.Windows.Forms.TextBox m_tbSpotState;
		private System.Windows.Forms.ComboBox m_cbSpot;
		private System.Windows.Forms.LinkLabel m_lnkSpotAction;
		private System.Windows.Forms.LinkLabel m_lnkSpotActionAux;
		private System.Windows.Forms.ListView m_lvProduction;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
	}
}
