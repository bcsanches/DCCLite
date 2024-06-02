namespace SharpDude
{
    partial class MainForm
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
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.GroupBox groupBox1;
            System.Windows.Forms.Label label1;
            System.Windows.Forms.GroupBox groupBox2;
            System.Windows.Forms.Label label5;
            System.Windows.Forms.Label label4;
            System.Windows.Forms.Label label3;
            System.Windows.Forms.Label label2;
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.tbAvrDude = new System.Windows.Forms.TextBox();
            this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
            this.tbVersion = new System.Windows.Forms.TextBox();
            this.cbComPorts = new System.Windows.Forms.ComboBox();
            this.cbArduinoTypes = new System.Windows.Forms.ComboBox();
            this.tbImageName = new System.Windows.Forms.TextBox();
            this.btnBurn = new System.Windows.Forms.Button();
            this.btnExit = new System.Windows.Forms.Button();
            this.tbOutput = new System.Windows.Forms.TextBox();
            this.arduinoBindingSource = new System.Windows.Forms.BindingSource(this.components);
            groupBox1 = new System.Windows.Forms.GroupBox();
            label1 = new System.Windows.Forms.Label();
            groupBox2 = new System.Windows.Forms.GroupBox();
            label5 = new System.Windows.Forms.Label();
            label4 = new System.Windows.Forms.Label();
            label3 = new System.Windows.Forms.Label();
            label2 = new System.Windows.Forms.Label();
            groupBox1.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            groupBox2.SuspendLayout();
            this.tableLayoutPanel2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.arduinoBindingSource)).BeginInit();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            groupBox1.Controls.Add(this.tableLayoutPanel1);
            groupBox1.Location = new System.Drawing.Point(12, 12);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new System.Drawing.Size(580, 65);
            groupBox1.TabIndex = 0;
            groupBox1.TabStop = false;
            groupBox1.Text = "AVRDude";
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 8.710801F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 91.2892F));
            this.tableLayoutPanel1.Controls.Add(this.tbAvrDude, 1, 0);
            this.tableLayoutPanel1.Controls.Add(label1, 0, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 16);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 1;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 46F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(574, 46);
            this.tableLayoutPanel1.TabIndex = 2;
            // 
            // tbAvrDude
            // 
            this.tbAvrDude.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.tbAvrDude.Location = new System.Drawing.Point(52, 13);
            this.tbAvrDude.Name = "tbAvrDude";
            this.tbAvrDude.ReadOnly = true;
            this.tbAvrDude.Size = new System.Drawing.Size(519, 20);
            this.tbAvrDude.TabIndex = 1;
            // 
            // label1
            // 
            label1.Anchor = System.Windows.Forms.AnchorStyles.Right;
            label1.AutoSize = true;
            label1.Location = new System.Drawing.Point(12, 16);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(34, 13);
            label1.TabIndex = 0;
            label1.Text = "Type:";
            // 
            // groupBox2
            // 
            groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            groupBox2.Controls.Add(this.tableLayoutPanel2);
            groupBox2.Location = new System.Drawing.Point(12, 83);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new System.Drawing.Size(580, 101);
            groupBox2.TabIndex = 1;
            groupBox2.TabStop = false;
            groupBox2.Text = "Arduino";
            // 
            // tableLayoutPanel2
            // 
            this.tableLayoutPanel2.ColumnCount = 4;
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 12.77641F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 87.22359F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 64F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 126F));
            this.tableLayoutPanel2.Controls.Add(this.tbVersion, 3, 1);
            this.tableLayoutPanel2.Controls.Add(label5, 2, 1);
            this.tableLayoutPanel2.Controls.Add(label4, 0, 1);
            this.tableLayoutPanel2.Controls.Add(this.cbComPorts, 3, 0);
            this.tableLayoutPanel2.Controls.Add(label3, 2, 0);
            this.tableLayoutPanel2.Controls.Add(label2, 0, 0);
            this.tableLayoutPanel2.Controls.Add(this.cbArduinoTypes, 1, 0);
            this.tableLayoutPanel2.Controls.Add(this.tbImageName, 1, 1);
            this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 16);
            this.tableLayoutPanel2.Name = "tableLayoutPanel2";
            this.tableLayoutPanel2.RowCount = 2;
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanel2.Size = new System.Drawing.Size(574, 82);
            this.tableLayoutPanel2.TabIndex = 0;
            // 
            // tbVersion
            // 
            this.tbVersion.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.tbVersion.Location = new System.Drawing.Point(450, 51);
            this.tbVersion.Name = "tbVersion";
            this.tbVersion.ReadOnly = true;
            this.tbVersion.Size = new System.Drawing.Size(121, 20);
            this.tbVersion.TabIndex = 9;
            // 
            // label5
            // 
            label5.Anchor = System.Windows.Forms.AnchorStyles.Right;
            label5.AutoSize = true;
            label5.Location = new System.Drawing.Point(399, 55);
            label5.Name = "label5";
            label5.Size = new System.Drawing.Size(45, 13);
            label5.TabIndex = 8;
            label5.Text = "Version:";
            // 
            // label4
            // 
            label4.Anchor = System.Windows.Forms.AnchorStyles.Right;
            label4.AutoSize = true;
            label4.Location = new System.Drawing.Point(7, 55);
            label4.Name = "label4";
            label4.Size = new System.Drawing.Size(39, 13);
            label4.TabIndex = 6;
            label4.Text = "Image:";
            // 
            // cbComPorts
            // 
            this.cbComPorts.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.cbComPorts.FormattingEnabled = true;
            this.cbComPorts.Location = new System.Drawing.Point(450, 10);
            this.cbComPorts.Name = "cbComPorts";
            this.cbComPorts.Size = new System.Drawing.Size(121, 21);
            this.cbComPorts.TabIndex = 5;
            // 
            // label3
            // 
            label3.Anchor = System.Windows.Forms.AnchorStyles.Right;
            label3.AutoSize = true;
            label3.Location = new System.Drawing.Point(415, 14);
            label3.Name = "label3";
            label3.Size = new System.Drawing.Size(29, 13);
            label3.TabIndex = 4;
            label3.Text = "Port:";
            // 
            // label2
            // 
            label2.Anchor = System.Windows.Forms.AnchorStyles.Right;
            label2.AutoSize = true;
            label2.Location = new System.Drawing.Point(7, 14);
            label2.Name = "label2";
            label2.Size = new System.Drawing.Size(39, 13);
            label2.TabIndex = 2;
            label2.Text = "Model:";
            // 
            // cbArduinoTypes
            // 
            this.cbArduinoTypes.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.cbArduinoTypes.DataBindings.Add(new System.Windows.Forms.Binding("Text", this.arduinoBindingSource, "BoardName", true));
            this.cbArduinoTypes.FormattingEnabled = true;
            this.cbArduinoTypes.Location = new System.Drawing.Point(52, 10);
            this.cbArduinoTypes.Name = "cbArduinoTypes";
            this.cbArduinoTypes.Size = new System.Drawing.Size(328, 21);
            this.cbArduinoTypes.TabIndex = 3;
            this.cbArduinoTypes.SelectedIndexChanged += new System.EventHandler(this.cbArduinoTypes_SelectedIndexChanged);
            // 
            // tbImageName
            // 
            this.tbImageName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.tbImageName.Location = new System.Drawing.Point(52, 51);
            this.tbImageName.Name = "tbImageName";
            this.tbImageName.ReadOnly = true;
            this.tbImageName.Size = new System.Drawing.Size(328, 20);
            this.tbImageName.TabIndex = 7;
            // 
            // btnBurn
            // 
            this.btnBurn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnBurn.Enabled = false;
            this.btnBurn.Location = new System.Drawing.Point(517, 476);
            this.btnBurn.Name = "btnBurn";
            this.btnBurn.Size = new System.Drawing.Size(75, 23);
            this.btnBurn.TabIndex = 2;
            this.btnBurn.Text = "&Burn";
            this.btnBurn.UseVisualStyleBackColor = true;
            this.btnBurn.Click += new System.EventHandler(this.btnBurn_Click);
            // 
            // btnExit
            // 
            this.btnExit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnExit.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnExit.Location = new System.Drawing.Point(12, 476);
            this.btnExit.Name = "btnExit";
            this.btnExit.Size = new System.Drawing.Size(75, 23);
            this.btnExit.TabIndex = 3;
            this.btnExit.Text = "E&xit";
            this.btnExit.UseVisualStyleBackColor = true;
            this.btnExit.Click += new System.EventHandler(this.btnExit_Click);
            // 
            // tbOutput
            // 
            this.tbOutput.BackColor = System.Drawing.Color.Blue;
            this.tbOutput.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbOutput.ForeColor = System.Drawing.Color.White;
            this.tbOutput.Location = new System.Drawing.Point(12, 190);
            this.tbOutput.Multiline = true;
            this.tbOutput.Name = "tbOutput";
            this.tbOutput.ReadOnly = true;
            this.tbOutput.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.tbOutput.Size = new System.Drawing.Size(580, 280);
            this.tbOutput.TabIndex = 4;
            // 
            // arduinoBindingSource
            // 
            this.arduinoBindingSource.DataSource = typeof(SharpDude.Arduino);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnExit;
            this.ClientSize = new System.Drawing.Size(604, 511);
            this.Controls.Add(this.tbOutput);
            this.Controls.Add(this.btnExit);
            this.Controls.Add(this.btnBurn);
            this.Controls.Add(groupBox2);
            this.Controls.Add(groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.MaximizeBox = false;
            this.Name = "MainForm";
            this.Text = "SharpDude";
            groupBox1.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            groupBox2.ResumeLayout(false);
            this.tableLayoutPanel2.ResumeLayout(false);
            this.tableLayoutPanel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.arduinoBindingSource)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.TextBox tbAvrDude;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
        private System.Windows.Forms.TextBox tbVersion;
        private System.Windows.Forms.ComboBox cbComPorts;
        private System.Windows.Forms.ComboBox cbArduinoTypes;
        private System.Windows.Forms.TextBox tbImageName;
        private System.Windows.Forms.Button btnBurn;
        private System.Windows.Forms.Button btnExit;
        private System.Windows.Forms.BindingSource arduinoBindingSource;
        private System.Windows.Forms.TextBox tbOutput;
    }
}