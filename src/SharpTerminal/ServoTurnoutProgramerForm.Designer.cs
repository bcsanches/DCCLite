// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

namespace SharpTerminal
{
    partial class ServoTurnoutProgrammerForm
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
            System.Windows.Forms.Label label1;
            System.Windows.Forms.Label label2;
            System.Windows.Forms.Label label3;
            System.Windows.Forms.Label label4;
            System.Windows.Forms.GroupBox groupBox1;
            System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
            this.m_btnCancel = new System.Windows.Forms.Button();
            this.m_btnOK = new System.Windows.Forms.Button();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown2 = new System.Windows.Forms.NumericUpDown();
            this.m_tbName = new System.Windows.Forms.TextBox();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.m_cbInverted = new System.Windows.Forms.CheckBox();
            this.m_cbIgnoreSaveState = new System.Windows.Forms.CheckBox();
            this.m_cbActivateOnPowerUp = new System.Windows.Forms.CheckBox();
            this.m_cbInvertedFrog = new System.Windows.Forms.CheckBox();
            this.m_cbInvertedPower = new System.Windows.Forms.CheckBox();
            label1 = new System.Windows.Forms.Label();
            label2 = new System.Windows.Forms.Label();
            label3 = new System.Windows.Forms.Label();
            label4 = new System.Windows.Forms.Label();
            groupBox1 = new System.Windows.Forms.GroupBox();
            tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
            this.tableLayoutPanel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).BeginInit();
            groupBox1.SuspendLayout();
            tableLayoutPanel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_btnCancel
            // 
            this.m_btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.m_btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_btnCancel.Location = new System.Drawing.Point(327, 261);
            this.m_btnCancel.Name = "m_btnCancel";
            this.m_btnCancel.Size = new System.Drawing.Size(75, 23);
            this.m_btnCancel.TabIndex = 1;
            this.m_btnCancel.Text = "&Cancel";
            this.m_btnCancel.UseVisualStyleBackColor = true;
            // 
            // m_btnOK
            // 
            this.m_btnOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.m_btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.m_btnOK.Enabled = false;
            this.m_btnOK.Location = new System.Drawing.Point(246, 261);
            this.m_btnOK.Name = "m_btnOK";
            this.m_btnOK.Size = new System.Drawing.Size(75, 23);
            this.m_btnOK.TabIndex = 2;
            this.m_btnOK.Text = "&OK";
            this.m_btnOK.UseVisualStyleBackColor = true;
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 32.82051F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 67.17949F));
            this.tableLayoutPanel1.Controls.Add(label4, 0, 3);
            this.tableLayoutPanel1.Controls.Add(label3, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.m_tbName, 1, 0);
            this.tableLayoutPanel1.Controls.Add(label1, 0, 1);
            this.tableLayoutPanel1.Controls.Add(label2, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.numericUpDown1, 1, 1);
            this.tableLayoutPanel1.Controls.Add(this.numericUpDown2, 1, 2);
            this.tableLayoutPanel1.Controls.Add(this.textBox1, 1, 3);
            this.tableLayoutPanel1.Controls.Add(groupBox1, 0, 4);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(12, 12);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 5;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(390, 243);
            this.tableLayoutPanel1.TabIndex = 3;
            // 
            // label1
            // 
            label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            label1.AutoSize = true;
            label1.Location = new System.Drawing.Point(3, 32);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(122, 13);
            label1.TabIndex = 0;
            label1.Text = "Start Angle:";
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.numericUpDown1.Location = new System.Drawing.Point(131, 29);
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size(55, 20);
            this.numericUpDown1.TabIndex = 1;
            // 
            // label2
            // 
            label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            label2.AutoSize = true;
            label2.Location = new System.Drawing.Point(3, 58);
            label2.Name = "label2";
            label2.Size = new System.Drawing.Size(122, 13);
            label2.TabIndex = 2;
            label2.Text = "End Angle:";
            // 
            // numericUpDown2
            // 
            this.numericUpDown2.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.numericUpDown2.Location = new System.Drawing.Point(131, 55);
            this.numericUpDown2.Name = "numericUpDown2";
            this.numericUpDown2.Size = new System.Drawing.Size(55, 20);
            this.numericUpDown2.TabIndex = 3;
            // 
            // label3
            // 
            label3.Anchor = System.Windows.Forms.AnchorStyles.Left;
            label3.AutoSize = true;
            label3.Location = new System.Drawing.Point(3, 6);
            label3.Name = "label3";
            label3.Size = new System.Drawing.Size(38, 13);
            label3.TabIndex = 4;
            label3.Text = "Servo:";
            // 
            // m_tbName
            // 
            this.m_tbName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.m_tbName.Location = new System.Drawing.Point(131, 3);
            this.m_tbName.Name = "m_tbName";
            this.m_tbName.ReadOnly = true;
            this.m_tbName.Size = new System.Drawing.Size(256, 20);
            this.m_tbName.TabIndex = 5;
            // 
            // label4
            // 
            label4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            label4.AutoSize = true;
            label4.Location = new System.Drawing.Point(3, 84);
            label4.Name = "label4";
            label4.Size = new System.Drawing.Size(122, 13);
            label4.TabIndex = 6;
            label4.Text = "Operation time (ms):";
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(131, 81);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(100, 20);
            this.textBox1.TabIndex = 7;
            // 
            // groupBox1
            // 
            this.tableLayoutPanel1.SetColumnSpan(groupBox1, 2);
            groupBox1.Controls.Add(tableLayoutPanel2);
            groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            groupBox1.Location = new System.Drawing.Point(3, 107);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new System.Drawing.Size(384, 133);
            groupBox1.TabIndex = 8;
            groupBox1.TabStop = false;
            groupBox1.Text = "Flags";
            // 
            // tableLayoutPanel2
            // 
            tableLayoutPanel2.ColumnCount = 1;
            tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            tableLayoutPanel2.Controls.Add(this.m_cbInverted, 0, 0);
            tableLayoutPanel2.Controls.Add(this.m_cbIgnoreSaveState, 0, 1);
            tableLayoutPanel2.Controls.Add(this.m_cbActivateOnPowerUp, 0, 2);
            tableLayoutPanel2.Controls.Add(this.m_cbInvertedFrog, 0, 3);
            tableLayoutPanel2.Controls.Add(this.m_cbInvertedPower, 0, 4);
            tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
            tableLayoutPanel2.Location = new System.Drawing.Point(3, 16);
            tableLayoutPanel2.Name = "tableLayoutPanel2";
            tableLayoutPanel2.RowCount = 5;
            tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
            tableLayoutPanel2.Size = new System.Drawing.Size(378, 114);
            tableLayoutPanel2.TabIndex = 0;
            // 
            // m_cbInverted
            // 
            this.m_cbInverted.AutoSize = true;
            this.m_cbInverted.Location = new System.Drawing.Point(3, 3);
            this.m_cbInverted.Name = "m_cbInverted";
            this.m_cbInverted.Size = new System.Drawing.Size(65, 17);
            this.m_cbInverted.TabIndex = 0;
            this.m_cbInverted.Text = "Inverted";
            this.m_cbInverted.UseVisualStyleBackColor = true;
            // 
            // m_cbIgnoreSaveState
            // 
            this.m_cbIgnoreSaveState.AutoSize = true;
            this.m_cbIgnoreSaveState.Location = new System.Drawing.Point(3, 26);
            this.m_cbIgnoreSaveState.Name = "m_cbIgnoreSaveState";
            this.m_cbIgnoreSaveState.Size = new System.Drawing.Size(108, 17);
            this.m_cbIgnoreSaveState.TabIndex = 1;
            this.m_cbIgnoreSaveState.Text = "Ignore save state";
            this.m_cbIgnoreSaveState.UseVisualStyleBackColor = true;
            // 
            // m_cbActivateOnPowerUp
            // 
            this.m_cbActivateOnPowerUp.AutoSize = true;
            this.m_cbActivateOnPowerUp.Location = new System.Drawing.Point(3, 49);
            this.m_cbActivateOnPowerUp.Name = "m_cbActivateOnPowerUp";
            this.m_cbActivateOnPowerUp.Size = new System.Drawing.Size(127, 17);
            this.m_cbActivateOnPowerUp.TabIndex = 2;
            this.m_cbActivateOnPowerUp.Text = "Activate on power up";
            this.m_cbActivateOnPowerUp.UseVisualStyleBackColor = true;
            // 
            // m_cbInvertedFrog
            // 
            this.m_cbInvertedFrog.AutoSize = true;
            this.m_cbInvertedFrog.Location = new System.Drawing.Point(3, 72);
            this.m_cbInvertedFrog.Name = "m_cbInvertedFrog";
            this.m_cbInvertedFrog.Size = new System.Drawing.Size(86, 17);
            this.m_cbInvertedFrog.TabIndex = 3;
            this.m_cbInvertedFrog.Text = "Inverted frog";
            this.m_cbInvertedFrog.UseVisualStyleBackColor = true;
            // 
            // m_cbInvertedPower
            // 
            this.m_cbInvertedPower.AutoSize = true;
            this.m_cbInvertedPower.Location = new System.Drawing.Point(3, 95);
            this.m_cbInvertedPower.Name = "m_cbInvertedPower";
            this.m_cbInvertedPower.Size = new System.Drawing.Size(97, 17);
            this.m_cbInvertedPower.TabIndex = 4;
            this.m_cbInvertedPower.Text = "Inverted power";
            this.m_cbInvertedPower.UseVisualStyleBackColor = true;
            // 
            // ServoTurnoutProgrammerForm
            // 
            this.AcceptButton = this.m_btnOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.m_btnCancel;
            this.ClientSize = new System.Drawing.Size(414, 296);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.m_btnOK);
            this.Controls.Add(this.m_btnCancel);
            this.KeyPreview = true;
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(430, 335);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(430, 335);
            this.Name = "ServoTurnoutProgrammerForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Servo Programmer";
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).EndInit();
            groupBox1.ResumeLayout(false);
            tableLayoutPanel2.ResumeLayout(false);
            tableLayoutPanel2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button m_btnCancel;
        private System.Windows.Forms.Button m_btnOK;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.TextBox m_tbName;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
        private System.Windows.Forms.NumericUpDown numericUpDown2;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.CheckBox m_cbInverted;
        private System.Windows.Forms.CheckBox m_cbIgnoreSaveState;
        private System.Windows.Forms.CheckBox m_cbActivateOnPowerUp;
        private System.Windows.Forms.CheckBox m_cbInvertedFrog;
        private System.Windows.Forms.CheckBox m_cbInvertedPower;
    }
}

