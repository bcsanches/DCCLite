namespace SharpTerminal
{
    partial class RemoteLoconetServiceUserControl
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
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.GroupBox groupBox1;
            this.m_gridMain = new System.Windows.Forms.DataGridView();
            this.m_bsDataSource = new System.Windows.Forms.BindingSource(this.components);
            this.m_lbTitle = new System.Windows.Forms.Label();
            this.Slot = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.locomotiveAddressDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.speedDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.forwardDataGridViewCheckBoxColumn = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.stateDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            groupBox1 = new System.Windows.Forms.GroupBox();
            groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.m_gridMain)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.m_bsDataSource)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            groupBox1.Controls.Add(this.m_gridMain);
            groupBox1.Location = new System.Drawing.Point(3, 36);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new System.Drawing.Size(509, 203);
            groupBox1.TabIndex = 2;
            groupBox1.TabStop = false;
            groupBox1.Text = "Slots";
            // 
            // m_gridMain
            // 
            this.m_gridMain.AutoGenerateColumns = false;
            this.m_gridMain.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.m_gridMain.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Slot,
            this.locomotiveAddressDataGridViewTextBoxColumn,
            this.speedDataGridViewTextBoxColumn,
            this.forwardDataGridViewCheckBoxColumn,
            this.stateDataGridViewTextBoxColumn});
            this.m_gridMain.DataSource = this.m_bsDataSource;
            this.m_gridMain.Dock = System.Windows.Forms.DockStyle.Fill;
            this.m_gridMain.Location = new System.Drawing.Point(3, 16);
            this.m_gridMain.MultiSelect = false;
            this.m_gridMain.Name = "m_gridMain";
            this.m_gridMain.ReadOnly = true;
            this.m_gridMain.RowHeadersVisible = false;
            this.m_gridMain.Size = new System.Drawing.Size(503, 184);
            this.m_gridMain.TabIndex = 0;
            // 
            // m_bsDataSource
            // 
            this.m_bsDataSource.DataSource = typeof(SharpTerminal.RemoteLoconetSlot);
            // 
            // m_lbTitle
            // 
            this.m_lbTitle.AutoSize = true;
            this.m_lbTitle.Font = new System.Drawing.Font("Microsoft Sans Serif", 14F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.m_lbTitle.Location = new System.Drawing.Point(3, 9);
            this.m_lbTitle.Name = "m_lbTitle";
            this.m_lbTitle.Size = new System.Drawing.Size(161, 24);
            this.m_lbTitle.TabIndex = 1;
            this.m_lbTitle.Text = "Loconet Service";
            // 
            // Slot
            // 
            this.Slot.DataPropertyName = "Index";
            this.Slot.HeaderText = "Slot";
            this.Slot.Name = "Slot";
            this.Slot.ReadOnly = true;
            // 
            // locomotiveAddressDataGridViewTextBoxColumn
            // 
            this.locomotiveAddressDataGridViewTextBoxColumn.DataPropertyName = "LocomotiveAddress";
            this.locomotiveAddressDataGridViewTextBoxColumn.HeaderText = "Address";
            this.locomotiveAddressDataGridViewTextBoxColumn.Name = "locomotiveAddressDataGridViewTextBoxColumn";
            this.locomotiveAddressDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // speedDataGridViewTextBoxColumn
            // 
            this.speedDataGridViewTextBoxColumn.DataPropertyName = "Speed";
            this.speedDataGridViewTextBoxColumn.HeaderText = "Speed";
            this.speedDataGridViewTextBoxColumn.Name = "speedDataGridViewTextBoxColumn";
            this.speedDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // forwardDataGridViewCheckBoxColumn
            // 
            this.forwardDataGridViewCheckBoxColumn.DataPropertyName = "Forward";
            this.forwardDataGridViewCheckBoxColumn.HeaderText = "Forward";
            this.forwardDataGridViewCheckBoxColumn.Name = "forwardDataGridViewCheckBoxColumn";
            this.forwardDataGridViewCheckBoxColumn.ReadOnly = true;
            // 
            // stateDataGridViewTextBoxColumn
            // 
            this.stateDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.stateDataGridViewTextBoxColumn.DataPropertyName = "State";
            this.stateDataGridViewTextBoxColumn.HeaderText = "State";
            this.stateDataGridViewTextBoxColumn.Name = "stateDataGridViewTextBoxColumn";
            this.stateDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // RemoteLoconetServiceUserControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(groupBox1);
            this.Controls.Add(this.m_lbTitle);
            this.Name = "RemoteLoconetServiceUserControl";
            this.Size = new System.Drawing.Size(515, 242);
            groupBox1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.m_gridMain)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.m_bsDataSource)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.DataGridView m_gridMain;
        private System.Windows.Forms.Label m_lbTitle;
        private System.Windows.Forms.BindingSource m_bsDataSource;
        private System.Windows.Forms.DataGridViewTextBoxColumn Slot;
        private System.Windows.Forms.DataGridViewTextBoxColumn locomotiveAddressDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn speedDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewCheckBoxColumn forwardDataGridViewCheckBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn stateDataGridViewTextBoxColumn;
    }
}
