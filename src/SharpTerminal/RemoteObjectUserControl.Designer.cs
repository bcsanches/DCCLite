namespace SharpTerminal
{
    partial class RemoteObjectUserControl
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
            mObject.PropertyChanged -= RemoteDecoder_PropertyChanged;

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
            this.mPropertyGrid = new System.Windows.Forms.PropertyGrid();
            this.SuspendLayout();
            // 
            // m_lbTitle
            // 
            this.m_lbTitle.AutoSize = true;
            this.m_lbTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_lbTitle.Location = new System.Drawing.Point(3, 9);
            this.m_lbTitle.Name = "m_lbTitle";
            this.m_lbTitle.Size = new System.Drawing.Size(187, 24);
            this.m_lbTitle.TabIndex = 1;
            this.m_lbTitle.Text = "Remote Decoder - ";
            // 
            // mPropertyGrid
            // 
            this.mPropertyGrid.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.mPropertyGrid.Enabled = false;
            this.mPropertyGrid.Location = new System.Drawing.Point(7, 36);
            this.mPropertyGrid.Name = "mPropertyGrid";
            this.mPropertyGrid.Size = new System.Drawing.Size(505, 203);
            this.mPropertyGrid.TabIndex = 2;
            // 
            // RemoteDecoderUserControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.mPropertyGrid);
            this.Controls.Add(this.m_lbTitle);
            this.Name = "RemoteDecoderUserControl";
            this.Size = new System.Drawing.Size(515, 242);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label m_lbTitle;
        private System.Windows.Forms.PropertyGrid mPropertyGrid;
    }
}
