namespace SharpTerminal
{
    partial class RemoteLocationUserControl
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
			System.Windows.Forms.ColumnHeader columnHeader1;
			System.Windows.Forms.ColumnHeader columnHeader2;
			System.Windows.Forms.ColumnHeader columnHeader3;
			System.Windows.Forms.ColumnHeader columnHeader4;
			m_lbTitle = new System.Windows.Forms.Label();
			m_lvItems = new System.Windows.Forms.ListView();
			columnHeader1 = new System.Windows.Forms.ColumnHeader();
			columnHeader2 = new System.Windows.Forms.ColumnHeader();
			columnHeader3 = new System.Windows.Forms.ColumnHeader();
			columnHeader4 = new System.Windows.Forms.ColumnHeader();
			SuspendLayout();
			// 
			// columnHeader1
			// 
			columnHeader1.Text = "Address";
			// 
			// columnHeader2
			// 
			columnHeader2.Text = "Type";
			// 
			// columnHeader3
			// 
			columnHeader3.Text = "Name";
			// 
			// columnHeader4
			// 
			columnHeader4.Text = "Device";
			// 
			// m_lbTitle
			// 
			m_lbTitle.AutoSize = true;
			m_lbTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, 0);
			m_lbTitle.Location = new System.Drawing.Point(4, 10);
			m_lbTitle.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
			m_lbTitle.Name = "m_lbTitle";
			m_lbTitle.Size = new System.Drawing.Size(108, 24);
			m_lbTitle.TabIndex = 1;
			m_lbTitle.Text = "Location - ";
			// 
			// m_lvItems
			// 
			m_lvItems.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
			m_lvItems.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] { columnHeader1, columnHeader2, columnHeader3, columnHeader4 });
			m_lvItems.FullRowSelect = true;
			m_lvItems.GridLines = true;
			m_lvItems.Location = new System.Drawing.Point(4, 37);
			m_lvItems.MultiSelect = false;
			m_lvItems.Name = "m_lvItems";
			m_lvItems.Size = new System.Drawing.Size(381, 239);
			m_lvItems.TabIndex = 2;
			m_lvItems.UseCompatibleStateImageBehavior = false;
			m_lvItems.View = System.Windows.Forms.View.Details;
			// 
			// RemoteLocationUserControl
			// 
			AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			Controls.Add(m_lvItems);
			Controls.Add(m_lbTitle);
			Margin = new System.Windows.Forms.Padding(4, 3, 4, 3);
			Name = "RemoteLocationUserControl";
			Size = new System.Drawing.Size(388, 279);
			ResumeLayout(false);
			PerformLayout();
		}

		#endregion
		private System.Windows.Forms.Label m_lbTitle;
		private System.Windows.Forms.ListView m_lvItems;
	}
}
