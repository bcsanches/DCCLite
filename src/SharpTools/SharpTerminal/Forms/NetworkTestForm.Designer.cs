namespace SharpTerminal.Forms
{
	partial class NetworkTestForm
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

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			components = new System.ComponentModel.Container();
			System.Windows.Forms.StatusStrip statusStrip1;
			System.Windows.Forms.GroupBox groupBox1;
			System.Windows.Forms.Label label5;
			System.Windows.Forms.Label label1;
			System.Windows.Forms.Label label2;
			System.Windows.Forms.Label label3;
			System.Windows.Forms.Label label4;
			m_lblStatus = new System.Windows.Forms.ToolStripStatusLabel();
			tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			m_lblLatency = new System.Windows.Forms.Label();
			m_lblOutOfSyncPacketsCount = new System.Windows.Forms.Label();
			m_lblLostPacketsCount = new System.Windows.Forms.Label();
			m_lblReceivedPacketsCount = new System.Windows.Forms.Label();
			m_lblSentPackets = new System.Windows.Forms.Label();
			button1 = new System.Windows.Forms.Button();
			m_btnStart = new System.Windows.Forms.Button();
			m_btnStop = new System.Windows.Forms.Button();
			m_tTimer = new System.Windows.Forms.Timer(components);
			statusStrip1 = new System.Windows.Forms.StatusStrip();
			groupBox1 = new System.Windows.Forms.GroupBox();
			label5 = new System.Windows.Forms.Label();
			label1 = new System.Windows.Forms.Label();
			label2 = new System.Windows.Forms.Label();
			label3 = new System.Windows.Forms.Label();
			label4 = new System.Windows.Forms.Label();
			statusStrip1.SuspendLayout();
			groupBox1.SuspendLayout();
			tableLayoutPanel1.SuspendLayout();
			SuspendLayout();
			// 
			// statusStrip1
			// 
			statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] { m_lblStatus });
			statusStrip1.Location = new System.Drawing.Point(0, 175);
			statusStrip1.Name = "statusStrip1";
			statusStrip1.Size = new System.Drawing.Size(394, 22);
			statusStrip1.TabIndex = 1;
			statusStrip1.Text = "statusStrip1";
			// 
			// m_lblStatus
			// 
			m_lblStatus.Name = "m_lblStatus";
			m_lblStatus.Size = new System.Drawing.Size(118, 17);
			m_lblStatus.Text = "toolStripStatusLabel1";
			// 
			// groupBox1
			// 
			groupBox1.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			groupBox1.Controls.Add(tableLayoutPanel1);
			groupBox1.Location = new System.Drawing.Point(12, 12);
			groupBox1.Name = "groupBox1";
			groupBox1.Size = new System.Drawing.Size(370, 127);
			groupBox1.TabIndex = 4;
			groupBox1.TabStop = false;
			groupBox1.Text = "Results";
			// 
			// tableLayoutPanel1
			// 
			tableLayoutPanel1.ColumnCount = 2;
			tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			tableLayoutPanel1.Controls.Add(m_lblLatency, 1, 4);
			tableLayoutPanel1.Controls.Add(label5, 0, 4);
			tableLayoutPanel1.Controls.Add(m_lblOutOfSyncPacketsCount, 1, 3);
			tableLayoutPanel1.Controls.Add(m_lblLostPacketsCount, 1, 2);
			tableLayoutPanel1.Controls.Add(m_lblReceivedPacketsCount, 1, 1);
			tableLayoutPanel1.Controls.Add(m_lblSentPackets, 1, 0);
			tableLayoutPanel1.Controls.Add(label1, 0, 0);
			tableLayoutPanel1.Controls.Add(label2, 0, 1);
			tableLayoutPanel1.Controls.Add(label3, 0, 2);
			tableLayoutPanel1.Controls.Add(label4, 0, 3);
			tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			tableLayoutPanel1.Location = new System.Drawing.Point(3, 19);
			tableLayoutPanel1.Name = "tableLayoutPanel1";
			tableLayoutPanel1.RowCount = 5;
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
			tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 20F));
			tableLayoutPanel1.Size = new System.Drawing.Size(364, 105);
			tableLayoutPanel1.TabIndex = 0;
			// 
			// m_lblLatency
			// 
			m_lblLatency.Anchor = System.Windows.Forms.AnchorStyles.None;
			m_lblLatency.AutoSize = true;
			m_lblLatency.Location = new System.Drawing.Point(234, 87);
			m_lblLatency.Name = "m_lblLatency";
			m_lblLatency.Size = new System.Drawing.Size(77, 15);
			m_lblLatency.TabIndex = 9;
			m_lblLatency.Text = "m_lblLatency";
			// 
			// label5
			// 
			label5.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label5.AutoSize = true;
			label5.Location = new System.Drawing.Point(128, 87);
			label5.Name = "label5";
			label5.RightToLeft = System.Windows.Forms.RightToLeft.No;
			label5.Size = new System.Drawing.Size(51, 15);
			label5.TabIndex = 8;
			label5.Text = "Latency:";
			// 
			// m_lblOutOfSyncPacketsCount
			// 
			m_lblOutOfSyncPacketsCount.Anchor = System.Windows.Forms.AnchorStyles.None;
			m_lblOutOfSyncPacketsCount.AutoSize = true;
			m_lblOutOfSyncPacketsCount.Location = new System.Drawing.Point(189, 66);
			m_lblOutOfSyncPacketsCount.Name = "m_lblOutOfSyncPacketsCount";
			m_lblOutOfSyncPacketsCount.Size = new System.Drawing.Size(167, 15);
			m_lblOutOfSyncPacketsCount.TabIndex = 7;
			m_lblOutOfSyncPacketsCount.Text = "m_lblOutOfSyncPacketsCount";
			// 
			// m_lblLostPacketsCount
			// 
			m_lblLostPacketsCount.Anchor = System.Windows.Forms.AnchorStyles.None;
			m_lblLostPacketsCount.AutoSize = true;
			m_lblLostPacketsCount.Location = new System.Drawing.Point(207, 45);
			m_lblLostPacketsCount.Name = "m_lblLostPacketsCount";
			m_lblLostPacketsCount.Size = new System.Drawing.Size(131, 15);
			m_lblLostPacketsCount.TabIndex = 6;
			m_lblLostPacketsCount.Text = "m_lblLostPacketsCount";
			// 
			// m_lblReceivedPacketsCount
			// 
			m_lblReceivedPacketsCount.Anchor = System.Windows.Forms.AnchorStyles.None;
			m_lblReceivedPacketsCount.AutoSize = true;
			m_lblReceivedPacketsCount.Location = new System.Drawing.Point(195, 24);
			m_lblReceivedPacketsCount.Name = "m_lblReceivedPacketsCount";
			m_lblReceivedPacketsCount.Size = new System.Drawing.Size(156, 15);
			m_lblReceivedPacketsCount.TabIndex = 5;
			m_lblReceivedPacketsCount.Text = "m_lblReceivedPacketsCount";
			// 
			// m_lblSentPackets
			// 
			m_lblSentPackets.Anchor = System.Windows.Forms.AnchorStyles.None;
			m_lblSentPackets.AutoSize = true;
			m_lblSentPackets.Location = new System.Drawing.Point(222, 3);
			m_lblSentPackets.Name = "m_lblSentPackets";
			m_lblSentPackets.Size = new System.Drawing.Size(102, 15);
			m_lblSentPackets.TabIndex = 4;
			m_lblSentPackets.Text = "sentPacketsCount";
			// 
			// label1
			// 
			label1.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label1.AutoSize = true;
			label1.Location = new System.Drawing.Point(103, 3);
			label1.Name = "label1";
			label1.Size = new System.Drawing.Size(76, 15);
			label1.TabIndex = 0;
			label1.Text = "Sent packets:";
			// 
			// label2
			// 
			label2.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label2.AutoSize = true;
			label2.Location = new System.Drawing.Point(79, 24);
			label2.Name = "label2";
			label2.Size = new System.Drawing.Size(100, 15);
			label2.TabIndex = 1;
			label2.Text = "Received packets:";
			// 
			// label3
			// 
			label3.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label3.AutoSize = true;
			label3.Location = new System.Drawing.Point(104, 45);
			label3.Name = "label3";
			label3.Size = new System.Drawing.Size(75, 15);
			label3.TabIndex = 2;
			label3.Text = "Lost packets:";
			// 
			// label4
			// 
			label4.Anchor = System.Windows.Forms.AnchorStyles.Right;
			label4.AutoSize = true;
			label4.Location = new System.Drawing.Point(65, 66);
			label4.Name = "label4";
			label4.Size = new System.Drawing.Size(114, 15);
			label4.TabIndex = 3;
			label4.Text = "Out of sync packets:";
			// 
			// button1
			// 
			button1.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right;
			button1.Location = new System.Drawing.Point(294, 145);
			button1.Name = "button1";
			button1.Size = new System.Drawing.Size(88, 27);
			button1.TabIndex = 0;
			button1.Text = "E&xit";
			button1.UseVisualStyleBackColor = true;
			button1.Click += button1_Click;
			// 
			// m_btnStart
			// 
			m_btnStart.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnStart.Location = new System.Drawing.Point(12, 145);
			m_btnStart.Name = "m_btnStart";
			m_btnStart.Size = new System.Drawing.Size(88, 27);
			m_btnStart.TabIndex = 2;
			m_btnStart.Text = "Start";
			m_btnStart.UseVisualStyleBackColor = true;
			m_btnStart.Click += m_btnStart_Click;
			// 
			// m_btnStop
			// 
			m_btnStop.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnStop.Enabled = false;
			m_btnStop.Location = new System.Drawing.Point(106, 145);
			m_btnStop.Name = "m_btnStop";
			m_btnStop.Size = new System.Drawing.Size(88, 27);
			m_btnStop.TabIndex = 3;
			m_btnStop.Text = "Stop";
			m_btnStop.UseVisualStyleBackColor = true;
			m_btnStop.Click += m_btnStop_Click;
			// 
			// m_tTimer
			// 
			m_tTimer.Tick += m_tTimer_Tick;
			// 
			// NetworkTestForm
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			ClientSize = new System.Drawing.Size(394, 197);
			Controls.Add(groupBox1);
			Controls.Add(m_btnStop);
			Controls.Add(m_btnStart);
			Controls.Add(statusStrip1);
			Controls.Add(button1);
			MaximizeBox = false;
			MinimizeBox = false;
			MinimumSize = new System.Drawing.Size(410, 236);
			Name = "NetworkTestForm";
			Text = "NetworkTestForm";
			FormClosed += NetworkTestForm_FormClosed;
			statusStrip1.ResumeLayout(false);
			statusStrip1.PerformLayout();
			groupBox1.ResumeLayout(false);
			tableLayoutPanel1.ResumeLayout(false);
			tableLayoutPanel1.PerformLayout();
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.ToolStripStatusLabel m_lblStatus;
		private System.Windows.Forms.Button m_btnStart;
		private System.Windows.Forms.Button m_btnStop;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private System.Windows.Forms.Label m_lblOutOfSyncPacketsCount;
		private System.Windows.Forms.Label m_lblLostPacketsCount;
		private System.Windows.Forms.Label m_lblReceivedPacketsCount;
		private System.Windows.Forms.Label m_lblSentPackets;
		private System.Windows.Forms.Timer m_tTimer;
		private System.Windows.Forms.Label m_lblLatency;
	}
}