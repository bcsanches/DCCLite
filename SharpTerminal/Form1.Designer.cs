namespace SharpTerminal
{
    partial class Form1
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
            this.m_tbConsole = new System.Windows.Forms.TextBox();
            this.m_tbInput = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // m_tbConsole
            // 
            this.m_tbConsole.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_tbConsole.Location = new System.Drawing.Point(12, 12);
            this.m_tbConsole.Multiline = true;
            this.m_tbConsole.Name = "m_tbConsole";
            this.m_tbConsole.ReadOnly = true;
            this.m_tbConsole.Size = new System.Drawing.Size(760, 511);
            this.m_tbConsole.TabIndex = 0;
            // 
            // m_tbInput
            // 
            this.m_tbInput.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.m_tbInput.Enabled = false;
            this.m_tbInput.Location = new System.Drawing.Point(12, 529);
            this.m_tbInput.Name = "m_tbInput";
            this.m_tbInput.Size = new System.Drawing.Size(760, 20);
            this.m_tbInput.TabIndex = 1;
            this.m_tbInput.KeyUp += new System.Windows.Forms.KeyEventHandler(this.m_tbInput_KeyUp);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 561);
            this.Controls.Add(this.m_tbInput);
            this.Controls.Add(this.m_tbConsole);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox m_tbConsole;
        private System.Windows.Forms.TextBox m_tbInput;
    }
}

