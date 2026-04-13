namespace SharpTerminal.Tycoon.Forms
{
	partial class SpotReservationInfoForm
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
			m_btnCancel = new System.Windows.Forms.Button();
			m_btnOk = new System.Windows.Forms.Button();
			label1 = new System.Windows.Forms.Label();
			m_tbInfo = new System.Windows.Forms.TextBox();
			SuspendLayout();
			// 
			// m_btnCancel
			// 
			m_btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			m_btnCancel.Location = new System.Drawing.Point(293, 36);
			m_btnCancel.Name = "m_btnCancel";
			m_btnCancel.Size = new System.Drawing.Size(75, 23);
			m_btnCancel.TabIndex = 3;
			m_btnCancel.Text = "&Cancel";
			m_btnCancel.UseVisualStyleBackColor = true;
			// 
			// m_btnOk
			// 
			m_btnOk.DialogResult = System.Windows.Forms.DialogResult.OK;
			m_btnOk.Location = new System.Drawing.Point(212, 36);
			m_btnOk.Name = "m_btnOk";
			m_btnOk.Size = new System.Drawing.Size(75, 23);
			m_btnOk.TabIndex = 2;
			m_btnOk.Text = "&OK";
			m_btnOk.UseVisualStyleBackColor = true;
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Location = new System.Drawing.Point(12, 9);
			label1.Name = "label1";
			label1.Size = new System.Drawing.Size(31, 15);
			label1.TabIndex = 0;
			label1.Text = "&Info:";
			// 
			// m_tbInfo
			// 
			m_tbInfo.Location = new System.Drawing.Point(60, 6);
			m_tbInfo.Name = "m_tbInfo";
			m_tbInfo.Size = new System.Drawing.Size(308, 23);
			m_tbInfo.TabIndex = 1;
			// 
			// SpotReservationInfoForm
			// 
			AcceptButton = m_btnOk;
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			CancelButton = m_btnCancel;
			ClientSize = new System.Drawing.Size(380, 71);
			Controls.Add(m_tbInfo);
			Controls.Add(m_btnCancel);
			Controls.Add(m_btnOk);
			Controls.Add(label1);
			MaximizeBox = false;
			MaximumSize = new System.Drawing.Size(396, 110);
			MinimizeBox = false;
			MinimumSize = new System.Drawing.Size(396, 110);
			Name = "SpotReservationInfoForm";
			SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
			Text = "Enter aditional information (optional)";
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.Button m_btnCancel;
		private System.Windows.Forms.Button m_btnOk;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox m_tbInfo;
	}
}