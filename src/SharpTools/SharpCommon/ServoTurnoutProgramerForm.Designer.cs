// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

namespace SharpCommon
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
            System.Windows.Forms.Label label5;
            System.Windows.Forms.GroupBox groupBox2;
            this.m_cbInverted = new System.Windows.Forms.CheckBox();
            this.m_cbIgnoreSaveState = new System.Windows.Forms.CheckBox();
            this.m_cbActivateOnPowerUp = new System.Windows.Forms.CheckBox();
            this.m_cbInvertedFrog = new System.Windows.Forms.CheckBox();
            this.m_cbInvertedPower = new System.Windows.Forms.CheckBox();
            this.m_cbTestMode = new System.Windows.Forms.CheckBox();
            this.m_btnFlip = new System.Windows.Forms.Button();
            this.m_btnClose = new System.Windows.Forms.Button();
            this.m_btnThrow = new System.Windows.Forms.Button();
            this.m_btnCancel = new System.Windows.Forms.Button();
            this.m_btnOK = new System.Windows.Forms.Button();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.m_tbName = new System.Windows.Forms.TextBox();
            this.m_numStartAngle = new System.Windows.Forms.NumericUpDown();
            this.m_numEndAngle = new System.Windows.Forms.NumericUpDown();
            this.m_tbOperationTime = new System.Windows.Forms.MaskedTextBox();
            this.m_lblStatus = new System.Windows.Forms.Label();
            this.m_lnkStartPos = new System.Windows.Forms.LinkLabel();
            this.m_lnkEndPos = new System.Windows.Forms.LinkLabel();
            label1 = new System.Windows.Forms.Label();
            label2 = new System.Windows.Forms.Label();
            label3 = new System.Windows.Forms.Label();
            label4 = new System.Windows.Forms.Label();
            groupBox1 = new System.Windows.Forms.GroupBox();
            tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
            label5 = new System.Windows.Forms.Label();
            groupBox2 = new System.Windows.Forms.GroupBox();
            groupBox1.SuspendLayout();
            tableLayoutPanel2.SuspendLayout();
            groupBox2.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.m_numStartAngle)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.m_numEndAngle)).BeginInit();
            this.SuspendLayout();
            // 
            // label1
            // 
            label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            label1.AutoSize = true;
            label1.Location = new System.Drawing.Point(3, 32);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(123, 13);
            label1.TabIndex = 2;
            label1.Text = "&Start Angle:";
            // 
            // label2
            // 
            label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            label2.AutoSize = true;
            label2.Location = new System.Drawing.Point(3, 58);
            label2.Name = "label2";
            label2.Size = new System.Drawing.Size(123, 13);
            label2.TabIndex = 5;
            label2.Text = "&End Angle:";
            // 
            // label3
            // 
            label3.Anchor = System.Windows.Forms.AnchorStyles.Left;
            label3.AutoSize = true;
            label3.Location = new System.Drawing.Point(3, 6);
            label3.Name = "label3";
            label3.Size = new System.Drawing.Size(38, 13);
            label3.TabIndex = 0;
            label3.Text = "Servo:";
            // 
            // label4
            // 
            label4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            label4.AutoSize = true;
            label4.Location = new System.Drawing.Point(3, 84);
            label4.Name = "label4";
            label4.Size = new System.Drawing.Size(123, 13);
            label4.TabIndex = 8;
            label4.Text = "Operation &time (ms):";
            // 
            // groupBox1
            // 
            groupBox1.Controls.Add(tableLayoutPanel2);
            groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            groupBox1.Location = new System.Drawing.Point(3, 107);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new System.Drawing.Size(123, 135);
            groupBox1.TabIndex = 10;
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
            tableLayoutPanel2.Size = new System.Drawing.Size(117, 116);
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
            this.m_cbInverted.CheckedChanged += new System.EventHandler(this.Flags_CheckedChanged);
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
            this.m_cbIgnoreSaveState.CheckedChanged += new System.EventHandler(this.Flags_CheckedChanged);
            // 
            // m_cbActivateOnPowerUp
            // 
            this.m_cbActivateOnPowerUp.AutoSize = true;
            this.m_cbActivateOnPowerUp.Location = new System.Drawing.Point(3, 49);
            this.m_cbActivateOnPowerUp.Name = "m_cbActivateOnPowerUp";
            this.m_cbActivateOnPowerUp.Size = new System.Drawing.Size(111, 17);
            this.m_cbActivateOnPowerUp.TabIndex = 2;
            this.m_cbActivateOnPowerUp.Text = "Activate on power up";
            this.m_cbActivateOnPowerUp.UseVisualStyleBackColor = true;
            this.m_cbActivateOnPowerUp.CheckedChanged += new System.EventHandler(this.Flags_CheckedChanged);
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
            this.m_cbInvertedFrog.CheckedChanged += new System.EventHandler(this.Flags_CheckedChanged);
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
            this.m_cbInvertedPower.CheckedChanged += new System.EventHandler(this.Flags_CheckedChanged);
            // 
            // label5
            // 
            label5.Anchor = System.Windows.Forms.AnchorStyles.Left;
            label5.AutoSize = true;
            label5.Location = new System.Drawing.Point(3, 250);
            label5.Name = "label5";
            label5.Size = new System.Drawing.Size(40, 13);
            label5.TabIndex = 12;
            label5.Text = "Status:";
            // 
            // groupBox2
            // 
            this.tableLayoutPanel1.SetColumnSpan(groupBox2, 2);
            groupBox2.Controls.Add(this.m_cbTestMode);
            groupBox2.Controls.Add(this.m_btnFlip);
            groupBox2.Controls.Add(this.m_btnClose);
            groupBox2.Controls.Add(this.m_btnThrow);
            groupBox2.Dock = System.Windows.Forms.DockStyle.Fill;
            groupBox2.Location = new System.Drawing.Point(132, 107);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new System.Drawing.Size(255, 135);
            groupBox2.TabIndex = 11;
            groupBox2.TabStop = false;
            groupBox2.Text = "Test Controls";
            // 
            // m_cbTestMode
            // 
            this.m_cbTestMode.AutoSize = true;
            this.m_cbTestMode.Location = new System.Drawing.Point(6, 111);
            this.m_cbTestMode.Name = "m_cbTestMode";
            this.m_cbTestMode.Size = new System.Drawing.Size(76, 17);
            this.m_cbTestMode.TabIndex = 3;
            this.m_cbTestMode.Text = "Test mo&de";
            this.m_cbTestMode.UseVisualStyleBackColor = true;
            this.m_cbTestMode.CheckedChanged += new System.EventHandler(this.m_cbTestMode_CheckedChanged);
            // 
            // m_btnFlip
            // 
            this.m_btnFlip.Enabled = false;
            this.m_btnFlip.Location = new System.Drawing.Point(168, 19);
            this.m_btnFlip.Name = "m_btnFlip";
            this.m_btnFlip.Size = new System.Drawing.Size(75, 23);
            this.m_btnFlip.TabIndex = 2;
            this.m_btnFlip.Text = "&Flip";
            this.m_btnFlip.UseVisualStyleBackColor = true;
            this.m_btnFlip.Click += new System.EventHandler(this.m_btnFlip_Click);
            // 
            // m_btnClose
            // 
            this.m_btnClose.Enabled = false;
            this.m_btnClose.Location = new System.Drawing.Point(87, 19);
            this.m_btnClose.Name = "m_btnClose";
            this.m_btnClose.Size = new System.Drawing.Size(75, 23);
            this.m_btnClose.TabIndex = 1;
            this.m_btnClose.Text = "&Close";
            this.m_btnClose.UseVisualStyleBackColor = true;
            this.m_btnClose.Click += new System.EventHandler(this.m_btnClose_Click);
            // 
            // m_btnThrow
            // 
            this.m_btnThrow.Enabled = false;
            this.m_btnThrow.Location = new System.Drawing.Point(6, 19);
            this.m_btnThrow.Name = "m_btnThrow";
            this.m_btnThrow.Size = new System.Drawing.Size(75, 23);
            this.m_btnThrow.TabIndex = 0;
            this.m_btnThrow.Text = "&Throw";
            this.m_btnThrow.UseVisualStyleBackColor = true;
            this.m_btnThrow.Click += new System.EventHandler(this.m_btnThrow_Click);
            // 
            // m_btnCancel
            // 
            this.m_btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.m_btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.m_btnCancel.Location = new System.Drawing.Point(327, 286);
            this.m_btnCancel.Name = "m_btnCancel";
            this.m_btnCancel.Size = new System.Drawing.Size(75, 23);
            this.m_btnCancel.TabIndex = 0;
            this.m_btnCancel.Text = "&Cancel";
            this.m_btnCancel.UseVisualStyleBackColor = true;
            this.m_btnCancel.Click += new System.EventHandler(this.m_btnCancel_Click);
            // 
            // m_btnOK
            // 
            this.m_btnOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.m_btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.m_btnOK.Enabled = false;
            this.m_btnOK.Location = new System.Drawing.Point(246, 286);
            this.m_btnOK.Name = "m_btnOK";
            this.m_btnOK.Size = new System.Drawing.Size(75, 23);
            this.m_btnOK.TabIndex = 2;
            this.m_btnOK.Text = "&OK";
            this.m_btnOK.UseVisualStyleBackColor = true;
            this.m_btnOK.Click += new System.EventHandler(this.m_btnOK_Click);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanel1.ColumnCount = 3;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 44.44444F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 22.22222F));
            this.tableLayoutPanel1.Controls.Add(label4, 0, 3);
            this.tableLayoutPanel1.Controls.Add(label3, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.m_tbName, 1, 0);
            this.tableLayoutPanel1.Controls.Add(label1, 0, 1);
            this.tableLayoutPanel1.Controls.Add(label2, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.m_numStartAngle, 1, 1);
            this.tableLayoutPanel1.Controls.Add(this.m_numEndAngle, 1, 2);
            this.tableLayoutPanel1.Controls.Add(groupBox1, 0, 4);
            this.tableLayoutPanel1.Controls.Add(this.m_tbOperationTime, 1, 3);
            this.tableLayoutPanel1.Controls.Add(label5, 0, 5);
            this.tableLayoutPanel1.Controls.Add(this.m_lblStatus, 1, 5);
            this.tableLayoutPanel1.Controls.Add(groupBox2, 1, 4);
            this.tableLayoutPanel1.Controls.Add(this.m_lnkStartPos, 2, 1);
            this.tableLayoutPanel1.Controls.Add(this.m_lnkEndPos, 2, 2);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(12, 12);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 6;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 141F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 18F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(390, 268);
            this.tableLayoutPanel1.TabIndex = 1;
            // 
            // m_tbName
            // 
            this.m_tbName.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanel1.SetColumnSpan(this.m_tbName, 2);
            this.m_tbName.Location = new System.Drawing.Point(132, 3);
            this.m_tbName.Name = "m_tbName";
            this.m_tbName.ReadOnly = true;
            this.m_tbName.Size = new System.Drawing.Size(255, 20);
            this.m_tbName.TabIndex = 1;
            // 
            // m_numStartAngle
            // 
            this.m_numStartAngle.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.m_numStartAngle.Location = new System.Drawing.Point(132, 29);
            this.m_numStartAngle.Name = "m_numStartAngle";
            this.m_numStartAngle.Size = new System.Drawing.Size(55, 20);
            this.m_numStartAngle.TabIndex = 3;
            this.m_numStartAngle.ValueChanged += new System.EventHandler(this.m_numStartAngle_ValueChanged);
            // 
            // m_numEndAngle
            // 
            this.m_numEndAngle.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.m_numEndAngle.Location = new System.Drawing.Point(132, 55);
            this.m_numEndAngle.Name = "m_numEndAngle";
            this.m_numEndAngle.Size = new System.Drawing.Size(55, 20);
            this.m_numEndAngle.TabIndex = 6;
            this.m_numEndAngle.ValueChanged += new System.EventHandler(this.m_numEndAngle_ValueChanged);
            // 
            // m_tbOperationTime
            // 
            this.m_tbOperationTime.Location = new System.Drawing.Point(132, 81);
            this.m_tbOperationTime.Mask = "0000";
            this.m_tbOperationTime.Name = "m_tbOperationTime";
            this.m_tbOperationTime.Size = new System.Drawing.Size(100, 20);
            this.m_tbOperationTime.TabIndex = 9;
            this.m_tbOperationTime.ValidatingType = typeof(int);
            this.m_tbOperationTime.Validated += new System.EventHandler(this.m_tbOperationTime_Validated);
            // 
            // m_lblStatus
            // 
            this.m_lblStatus.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.m_lblStatus.AutoSize = true;
            this.tableLayoutPanel1.SetColumnSpan(this.m_lblStatus, 2);
            this.m_lblStatus.Location = new System.Drawing.Point(132, 250);
            this.m_lblStatus.Name = "m_lblStatus";
            this.m_lblStatus.Size = new System.Drawing.Size(35, 13);
            this.m_lblStatus.TabIndex = 13;
            this.m_lblStatus.Text = "label6";
            // 
            // m_lnkStartPos
            // 
            this.m_lnkStartPos.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.m_lnkStartPos.AutoSize = true;
            this.m_lnkStartPos.Location = new System.Drawing.Point(305, 32);
            this.m_lnkStartPos.Name = "m_lnkStartPos";
            this.m_lnkStartPos.Size = new System.Drawing.Size(82, 13);
            this.m_lnkStartPos.TabIndex = 4;
            this.m_lnkStartPos.TabStop = true;
            this.m_lnkStartPos.Text = "&move";
            this.m_lnkStartPos.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.m_lnkStartPos_LinkClicked);
            // 
            // m_lnkEndPos
            // 
            this.m_lnkEndPos.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
            this.m_lnkEndPos.AutoSize = true;
            this.m_lnkEndPos.Location = new System.Drawing.Point(305, 58);
            this.m_lnkEndPos.Name = "m_lnkEndPos";
            this.m_lnkEndPos.Size = new System.Drawing.Size(82, 13);
            this.m_lnkEndPos.TabIndex = 7;
            this.m_lnkEndPos.TabStop = true;
            this.m_lnkEndPos.Text = "m&ove";
            this.m_lnkEndPos.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.m_lnkEndPos_LinkClicked);
            // 
            // ServoTurnoutProgrammerForm
            // 
            this.AcceptButton = this.m_btnOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.m_btnCancel;
            this.ClientSize = new System.Drawing.Size(414, 321);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.m_btnOK);
            this.Controls.Add(this.m_btnCancel);
            this.KeyPreview = true;
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(430, 360);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(430, 335);
            this.Name = "ServoTurnoutProgrammerForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Servo Programmer";
            groupBox1.ResumeLayout(false);
            tableLayoutPanel2.ResumeLayout(false);
            tableLayoutPanel2.PerformLayout();
            groupBox2.ResumeLayout(false);
            groupBox2.PerformLayout();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.m_numStartAngle)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.m_numEndAngle)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Button m_btnCancel;
        private System.Windows.Forms.Button m_btnOK;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.TextBox m_tbName;
        private System.Windows.Forms.NumericUpDown m_numStartAngle;
        private System.Windows.Forms.NumericUpDown m_numEndAngle;
        private System.Windows.Forms.CheckBox m_cbInverted;
        private System.Windows.Forms.CheckBox m_cbIgnoreSaveState;
        private System.Windows.Forms.CheckBox m_cbActivateOnPowerUp;
        private System.Windows.Forms.CheckBox m_cbInvertedFrog;
        private System.Windows.Forms.CheckBox m_cbInvertedPower;
        private System.Windows.Forms.MaskedTextBox m_tbOperationTime;
        private System.Windows.Forms.Label m_lblStatus;
        private System.Windows.Forms.CheckBox m_cbTestMode;
        private System.Windows.Forms.Button m_btnFlip;
        private System.Windows.Forms.Button m_btnClose;
        private System.Windows.Forms.Button m_btnThrow;
        private System.Windows.Forms.LinkLabel m_lnkStartPos;
        private System.Windows.Forms.LinkLabel m_lnkEndPos;
    }
}

