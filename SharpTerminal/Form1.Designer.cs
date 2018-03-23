namespace SharpTerminal
{
    partial class Console
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
                if(components != null)
                    components.Dispose();

                if (mClient != null)
                    mClient.Dispose();
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
            System.Windows.Forms.StatusStrip statusStrip1;
            this.m_lbStatus = new System.Windows.Forms.ToolStripStatusLabel();
            this.m_tbConsole = new System.Windows.Forms.TextBox();
            this.m_tbInput = new System.Windows.Forms.TextBox();
            this.m_btnQuit = new System.Windows.Forms.Button();
            this.m_btnSend = new System.Windows.Forms.Button();
            this.m_btnClear = new System.Windows.Forms.Button();
            statusStrip1 = new System.Windows.Forms.StatusStrip();
            statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // statusStrip1
            // 
            statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_lbStatus});
            statusStrip1.Location = new System.Drawing.Point(0, 539);
            statusStrip1.Name = "statusStrip1";
            statusStrip1.Size = new System.Drawing.Size(784, 22);
            statusStrip1.TabIndex = 5;
            statusStrip1.Text = "statusStrip1";
            // 
            // m_lbStatus
            // 
            this.m_lbStatus.Name = "m_lbStatus";
            this.m_lbStatus.Size = new System.Drawing.Size(118, 17);
            this.m_lbStatus.Text = "toolStripStatusLabel1";
            // 
            // m_tbConsole
            // 
            this.m_tbConsole.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_tbConsole.BackColor = System.Drawing.Color.MediumBlue;
            this.m_tbConsole.Font = new System.Drawing.Font("Lucida Console", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_tbConsole.ForeColor = System.Drawing.Color.White;
            this.m_tbConsole.Location = new System.Drawing.Point(12, 12);
            this.m_tbConsole.Multiline = true;
            this.m_tbConsole.Name = "m_tbConsole";
            this.m_tbConsole.ReadOnly = true;
            this.m_tbConsole.Size = new System.Drawing.Size(760, 468);
            this.m_tbConsole.TabIndex = 0;
            // 
            // m_tbInput
            // 
            this.m_tbInput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_tbInput.Location = new System.Drawing.Point(12, 486);
            this.m_tbInput.Name = "m_tbInput";
            this.m_tbInput.Size = new System.Drawing.Size(760, 20);
            this.m_tbInput.TabIndex = 1;
            this.m_tbInput.KeyUp += new System.Windows.Forms.KeyEventHandler(this.m_tbInput_KeyUp);
            // 
            // m_btnQuit
            // 
            this.m_btnQuit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.m_btnQuit.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_btnQuit.Location = new System.Drawing.Point(697, 513);
            this.m_btnQuit.Name = "m_btnQuit";
            this.m_btnQuit.Size = new System.Drawing.Size(75, 23);
            this.m_btnQuit.TabIndex = 4;
            this.m_btnQuit.Text = "&Quit";
            this.m_btnQuit.UseVisualStyleBackColor = true;
            this.m_btnQuit.Click += new System.EventHandler(this.m_btnQuit_Click);
            // 
            // m_btnSend
            // 
            this.m_btnSend.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.m_btnSend.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.m_btnSend.Location = new System.Drawing.Point(13, 512);
            this.m_btnSend.Name = "m_btnSend";
            this.m_btnSend.Size = new System.Drawing.Size(75, 23);
            this.m_btnSend.TabIndex = 2;
            this.m_btnSend.Text = "&Send";
            this.m_btnSend.UseVisualStyleBackColor = true;
            this.m_btnSend.Click += new System.EventHandler(this.m_btnSend_Click);
            // 
            // m_btnClear
            // 
            this.m_btnClear.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.m_btnClear.Location = new System.Drawing.Point(95, 513);
            this.m_btnClear.Name = "m_btnClear";
            this.m_btnClear.Size = new System.Drawing.Size(75, 23);
            this.m_btnClear.TabIndex = 3;
            this.m_btnClear.Text = "&Clear";
            this.m_btnClear.UseVisualStyleBackColor = true;
            this.m_btnClear.Click += new System.EventHandler(this.m_btnClear_Click);
            // 
            // Console
            // 
            this.AcceptButton = this.m_btnSend;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 561);
            this.Controls.Add(this.m_btnClear);
            this.Controls.Add(this.m_btnSend);
            this.Controls.Add(this.m_btnQuit);
            this.Controls.Add(statusStrip1);
            this.Controls.Add(this.m_tbInput);
            this.Controls.Add(this.m_tbConsole);
            this.Name = "Console";
            this.Text = "Form1";
            statusStrip1.ResumeLayout(false);
            statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox m_tbConsole;
        private System.Windows.Forms.TextBox m_tbInput;
        private System.Windows.Forms.ToolStripStatusLabel m_lbStatus;
        private System.Windows.Forms.Button m_btnQuit;
        private System.Windows.Forms.Button m_btnSend;
        private System.Windows.Forms.Button m_btnClear;
    }
}

