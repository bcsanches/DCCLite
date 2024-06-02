namespace SharpTerminal
{
    partial class RemoteShortcutUserControl
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
            this.m_lbTitle = new System.Windows.Forms.Label();
            this.m_lbLoadingMessage = new System.Windows.Forms.Label();
            this.mPanel = new System.Windows.Forms.Panel();
            this.mPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_lbTitle
            // 
            this.m_lbTitle.AutoSize = true;
            this.m_lbTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_lbTitle.Location = new System.Drawing.Point(3, 9);
            this.m_lbTitle.Name = "m_lbTitle";
            this.m_lbTitle.Size = new System.Drawing.Size(106, 24);
            this.m_lbTitle.TabIndex = 1;
            this.m_lbTitle.Text = "Shortcut - ";
            // 
            // m_lbLoadingMessage
            // 
            this.m_lbLoadingMessage.AutoSize = true;
            this.m_lbLoadingMessage.Location = new System.Drawing.Point(3, 0);
            this.m_lbLoadingMessage.Name = "m_lbLoadingMessage";
            this.m_lbLoadingMessage.Size = new System.Drawing.Size(54, 13);
            this.m_lbLoadingMessage.TabIndex = 2;
            this.m_lbLoadingMessage.Text = "Loading...";
            // 
            // mPanel
            // 
            this.mPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.mPanel.Controls.Add(this.m_lbLoadingMessage);
            this.mPanel.Location = new System.Drawing.Point(3, 36);
            this.mPanel.Name = "mPanel";
            this.mPanel.Size = new System.Drawing.Size(509, 203);
            this.mPanel.TabIndex = 3;
            // 
            // RemoteShortcutUserControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.mPanel);
            this.Controls.Add(this.m_lbTitle);
            this.Name = "RemoteShortcutUserControl";
            this.Size = new System.Drawing.Size(515, 242);
            this.mPanel.ResumeLayout(false);
            this.mPanel.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label m_lbTitle;
        private System.Windows.Forms.Label m_lbLoadingMessage;
        private System.Windows.Forms.Panel mPanel;
    }
}
