namespace SharpTerminal.Forms
{
	partial class RemoteDeviceRenameForm
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
			label1 = new System.Windows.Forms.Label();
			m_cbNames = new System.Windows.Forms.ComboBox();
			m_btnOk = new System.Windows.Forms.Button();
			m_btnCancel = new System.Windows.Forms.Button();
			SuspendLayout();
			// 
			// label1
			// 
			label1.AutoSize = true;
			label1.Location = new System.Drawing.Point(12, 9);
			label1.Name = "label1";
			label1.Size = new System.Drawing.Size(42, 15);
			label1.TabIndex = 0;
			label1.Text = "&Name:";
			// 
			// m_cbNames
			// 
			m_cbNames.FormattingEnabled = true;
			m_cbNames.Location = new System.Drawing.Point(60, 6);
			m_cbNames.Name = "m_cbNames";
			m_cbNames.Size = new System.Drawing.Size(244, 23);
			m_cbNames.TabIndex = 1;
			// 
			// m_btnOk
			// 
			m_btnOk.DialogResult = System.Windows.Forms.DialogResult.OK;
			m_btnOk.Location = new System.Drawing.Point(148, 58);
			m_btnOk.Name = "m_btnOk";
			m_btnOk.Size = new System.Drawing.Size(75, 23);
			m_btnOk.TabIndex = 2;
			m_btnOk.Text = "&OK";
			m_btnOk.UseVisualStyleBackColor = true;
			// 
			// m_btnCancel
			// 
			m_btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			m_btnCancel.Location = new System.Drawing.Point(229, 58);
			m_btnCancel.Name = "m_btnCancel";
			m_btnCancel.Size = new System.Drawing.Size(75, 23);
			m_btnCancel.TabIndex = 3;
			m_btnCancel.Text = "&Cancel";
			m_btnCancel.UseVisualStyleBackColor = true;
			// 
			// RemoteDeviceRenameForm
			// 
			AcceptButton = m_btnOk;
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			CancelButton = m_btnCancel;
			ClientSize = new System.Drawing.Size(316, 93);
			ControlBox = false;
			Controls.Add(m_btnCancel);
			Controls.Add(m_btnOk);
			Controls.Add(m_cbNames);
			Controls.Add(label1);
			MaximizeBox = false;
			MaximumSize = new System.Drawing.Size(332, 132);
			MinimizeBox = false;
			MinimumSize = new System.Drawing.Size(332, 132);
			Name = "RemoteDeviceRenameForm";
			Text = "Enter new device name:";
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.ComboBox m_cbNames;
		private System.Windows.Forms.Button m_btnOk;
		private System.Windows.Forms.Button m_btnCancel;
	}
}