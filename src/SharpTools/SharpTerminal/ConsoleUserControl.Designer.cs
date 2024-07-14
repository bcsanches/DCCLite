namespace SharpTerminal
{
    partial class ConsoleUserControl
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
			m_tbConsole = new System.Windows.Forms.TextBox();
			m_btnClear = new System.Windows.Forms.Button();
			m_btnSend = new System.Windows.Forms.Button();
			m_tbInput = new System.Windows.Forms.TextBox();
			SuspendLayout();
			// 
			// m_tbConsole
			// 
			m_tbConsole.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_tbConsole.BackColor = System.Drawing.Color.MidnightBlue;
			m_tbConsole.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
			m_tbConsole.ForeColor = System.Drawing.Color.White;
			m_tbConsole.Location = new System.Drawing.Point(4, 3);
			m_tbConsole.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			m_tbConsole.Multiline = true;
			m_tbConsole.Name = "m_tbConsole";
			m_tbConsole.ReadOnly = true;
			m_tbConsole.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			m_tbConsole.Size = new System.Drawing.Size(630, 255);
			m_tbConsole.TabIndex = 1;
			// 
			// m_btnClear
			// 
			m_btnClear.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnClear.Location = new System.Drawing.Point(98, 296);
			m_btnClear.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			m_btnClear.Name = "m_btnClear";
			m_btnClear.Size = new System.Drawing.Size(88, 27);
			m_btnClear.TabIndex = 6;
			m_btnClear.Text = "&Clear";
			m_btnClear.UseVisualStyleBackColor = true;
			m_btnClear.Click += m_btnClear_Click;
			// 
			// m_btnSend
			// 
			m_btnSend.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left;
			m_btnSend.DialogResult = System.Windows.Forms.DialogResult.OK;
			m_btnSend.Location = new System.Drawing.Point(4, 296);
			m_btnSend.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			m_btnSend.Name = "m_btnSend";
			m_btnSend.Size = new System.Drawing.Size(88, 27);
			m_btnSend.TabIndex = 5;
			m_btnSend.Text = "&Send";
			m_btnSend.UseVisualStyleBackColor = true;
			m_btnSend.Click += m_btnSend_Click;
			// 
			// m_tbInput
			// 
			m_tbInput.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_tbInput.Location = new System.Drawing.Point(4, 266);
			m_tbInput.Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			m_tbInput.Name = "m_tbInput";
			m_tbInput.Size = new System.Drawing.Size(630, 23);
			m_tbInput.TabIndex = 4;
			m_tbInput.KeyUp += m_tbInput_KeyUp;
			m_tbInput.PreviewKeyDown += m_tbInput_PreviewKeyDown;
			// 
			// ConsoleUserControl
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			Controls.Add(m_btnClear);
			Controls.Add(m_btnSend);
			Controls.Add(m_tbInput);
			Controls.Add(m_tbConsole);
			Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			Name = "ConsoleUserControl";
			Size = new System.Drawing.Size(638, 326);
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion

		private System.Windows.Forms.TextBox m_tbConsole;
        private System.Windows.Forms.Button m_btnClear;
        private System.Windows.Forms.Button m_btnSend;
        private System.Windows.Forms.TextBox m_tbInput;
    }
}
