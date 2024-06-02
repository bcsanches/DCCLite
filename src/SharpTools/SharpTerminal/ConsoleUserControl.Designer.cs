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
            this.m_tbConsole = new System.Windows.Forms.TextBox();
            this.m_btnClear = new System.Windows.Forms.Button();
            this.m_btnSend = new System.Windows.Forms.Button();
            this.m_tbInput = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // m_tbConsole
            // 
            this.m_tbConsole.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_tbConsole.BackColor = System.Drawing.Color.MidnightBlue;
            this.m_tbConsole.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_tbConsole.ForeColor = System.Drawing.Color.White;
            this.m_tbConsole.Location = new System.Drawing.Point(3, 3);
            this.m_tbConsole.Multiline = true;
            this.m_tbConsole.Name = "m_tbConsole";
            this.m_tbConsole.ReadOnly = true;
            this.m_tbConsole.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.m_tbConsole.Size = new System.Drawing.Size(671, 425);
            this.m_tbConsole.TabIndex = 1;
            // 
            // m_btnClear
            // 
            this.m_btnClear.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.m_btnClear.Location = new System.Drawing.Point(84, 460);
            this.m_btnClear.Name = "m_btnClear";
            this.m_btnClear.Size = new System.Drawing.Size(75, 23);
            this.m_btnClear.TabIndex = 6;
            this.m_btnClear.Text = "&Clear";
            this.m_btnClear.UseVisualStyleBackColor = true;
            this.m_btnClear.Click += new System.EventHandler(this.m_btnClear_Click);
            // 
            // m_btnSend
            // 
            this.m_btnSend.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.m_btnSend.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.m_btnSend.Location = new System.Drawing.Point(3, 460);
            this.m_btnSend.Name = "m_btnSend";
            this.m_btnSend.Size = new System.Drawing.Size(75, 23);
            this.m_btnSend.TabIndex = 5;
            this.m_btnSend.Text = "&Send";
            this.m_btnSend.UseVisualStyleBackColor = true;
            this.m_btnSend.Click += new System.EventHandler(this.m_btnSend_Click);
            // 
            // m_tbInput
            // 
            this.m_tbInput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_tbInput.Location = new System.Drawing.Point(3, 434);
            this.m_tbInput.Name = "m_tbInput";
            this.m_tbInput.Size = new System.Drawing.Size(671, 20);
            this.m_tbInput.TabIndex = 4;
            this.m_tbInput.KeyUp += new System.Windows.Forms.KeyEventHandler(this.m_tbInput_KeyUp);
            this.m_tbInput.PreviewKeyDown += new System.Windows.Forms.PreviewKeyDownEventHandler(this.m_tbInput_PreviewKeyDown);
            // 
            // ConsoleUserControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.m_btnClear);
            this.Controls.Add(this.m_btnSend);
            this.Controls.Add(this.m_tbInput);
            this.Controls.Add(this.m_tbConsole);
            this.Name = "ConsoleUserControl";
            this.Size = new System.Drawing.Size(677, 486);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox m_tbConsole;
        private System.Windows.Forms.Button m_btnClear;
        private System.Windows.Forms.Button m_btnSend;
        private System.Windows.Forms.TextBox m_tbInput;
    }
}
