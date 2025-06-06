﻿// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System;
using System.Runtime.Versioning;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpCommon
{
	[SupportedOSPlatform("windows")]
	public partial class ServoTurnoutProgrammerForm: Form
    {
        IServoProgrammer    m_clProgrammer;
        IServoTurnout       m_clTarget;

        //
        //Backup servo original data, because test mode causes a refresh
        private readonly ServoTurnoutFlags  m_fOriginalFlags;
        private readonly uint               m_uOriginalStartPos;            
        private readonly uint               m_uOriginalEndPos;
        private readonly uint               m_uOriginalMsOperationTime;


        private bool m_fDataChanged;
        private bool m_fDataCommited;
        private bool m_fCommitedOnce;        

        private bool m_fFailed;

        private void OnDataChanged()
        {
            m_fDataChanged = true;
            m_fDataCommited = false;

            m_btnOK.Enabled = true;
        }

        private void OnDataCommited()
        {            
            m_fDataCommited = true;
            m_fCommitedOnce = true;
        }

        public IServoTurnout Target { get { return m_clTarget; } }

        protected ServoTurnoutProgrammerForm()
        {
            InitializeComponent();                       
        }

        private static void ConfigureCheckBox(CheckBox checkBox, ServoTurnoutFlags flags, ServoTurnoutFlags bit)
        {
            checkBox.Checked = (flags & bit) == bit;
            checkBox.Tag = bit;
        }

        public ServoTurnoutProgrammerForm(IServoProgrammer programmer, IServoTurnout target)
        {            
            m_clTarget = target ?? throw new ArgumentNullException(nameof(target));
            m_clProgrammer = programmer ?? throw new ArgumentNullException(nameof(programmer));            

            m_fOriginalFlags = m_clTarget.Flags;
            m_uOriginalStartPos = m_clTarget.StartPos;
            m_uOriginalEndPos = m_clTarget.EndPos;
            m_uOriginalMsOperationTime = m_clTarget.MsOperationTime;

            InitializeComponent();

            this.Text += " - " + m_clTarget.Name;

            EnableProgMode(false);

            m_numStartAngle.Maximum = 1000;
            m_numEndAngle.Maximum = 1000;

            m_tbName.Text = m_clTarget.Name;
            m_numStartAngle.Value = m_clTarget.StartPos;
            m_numEndAngle.Value = m_clTarget.EndPos;

            m_numStartAngle.Maximum = m_numEndAngle.Value - 1;
            m_numEndAngle.Minimum = m_numStartAngle.Value + 1;

            m_tbOperationTime.Text = m_clTarget.MsOperationTime.ToString();

            ConfigureCheckBox(m_cbInverted, m_fOriginalFlags, ServoTurnoutFlags.SRVT_INVERTED_OPERATION);
            ConfigureCheckBox(m_cbIgnoreSaveState, m_fOriginalFlags, ServoTurnoutFlags.SRVT_IGNORE_SAVED_STATE);
            ConfigureCheckBox(m_cbInvertedFrog, m_fOriginalFlags, ServoTurnoutFlags.SRVT_INVERTED_FROG);
            ConfigureCheckBox(m_cbInvertedPower, m_fOriginalFlags, ServoTurnoutFlags.SRVT_INVERTED_POWER);
            ConfigureCheckBox(m_cbActivateOnPowerUp, m_fOriginalFlags, ServoTurnoutFlags.SRVT_ACTIVATE_ON_POWER_UP);

            m_lblStatus.Text = "Connecting...";

            //
            //reset all flags because loading data on the form will change those
            m_fDataChanged = false;
            m_fDataCommited = false;
            m_fCommitedOnce = false;

            m_btnOK.Enabled = false;
        }

        private void EnableProgMode(bool enable)
        {
            m_numStartAngle.Enabled = enable;
            m_numEndAngle.Enabled = enable;
            m_tbOperationTime.Enabled = enable;
            m_cbActivateOnPowerUp.Enabled = enable;
            m_cbIgnoreSaveState.Enabled = enable;
            m_cbInverted.Enabled = enable;
            m_cbInvertedFrog.Enabled = enable;
            m_cbInvertedPower.Enabled = enable;
            m_lnkStartPos.Enabled = enable;
            m_lnkEndPos.Enabled = enable;

            m_btnOK.Enabled = enable && m_fDataChanged;
        }

        private void EnableTestMode(bool enable)
        {
            m_btnFlip.Enabled = enable;
            m_btnClose.Enabled = enable;
            m_btnThrow.Enabled = enable;    
        }        

        private async Task StartServoProgrammerAsync()
        {
            for (; ; )
            {
                try
                {
                    await m_clProgrammer.StartAsync(m_clTarget);                    

                    EnableProgMode(true);

                    break;
                }
                catch (Exception ex)
                {
                    if (MessageBox.Show(this, "Failed to start programming mode: " + ex.Message + ".\n Retry??", "Error", MessageBoxButtons.RetryCancel) != DialogResult.Retry)
                    {
                        this.DialogResult = DialogResult.Cancel;
                        this.Close();

                        return;
                    }
                }
            }
        }

        protected override async void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            if (this.DesignMode)
                return;

            await this.StartServoProgrammerAsync();                                  
        }

        protected override async void OnFormClosing(FormClosingEventArgs e)
        {
            bool okPressed = this.DialogResult == DialogResult.OK;

            if (m_fFailed)
                return;

            if (!okPressed && !m_fFailed && m_fDataChanged && (MessageBox.Show(this, "Are you sure? Changes will not be saved", "Are you sure?", MessageBoxButtons.YesNo, MessageBoxIcon.Question) != DialogResult.Yes))
            {
                e.Cancel = true;

                return;
            }

            //
            //If no ok button used, ignore any data changed
            if (this.DialogResult != DialogResult.OK)
                m_fDataChanged = false;

            try
            {
                //Ok pressed 
                if(okPressed)
                {
                    //do we have changes that were not commited yet?
                    if (m_fDataChanged && !m_fDataCommited)
                    {
                        //Is programmer task not running?
                        if (!m_clProgrammer.IsRunning())                                                
                        {
                            //start it
                            await m_clProgrammer.StartAsync(m_clTarget);
                        }

                        //pending changes
                        await this.DeployFormServoDataAsync();
                    }                    
                }
                //user cancelled, has any data been persisted?
                else if (m_fCommitedOnce)
                {
                    //Is programmer task not running?
                    if (!m_clProgrammer.IsRunning())
                    {
                        //start it
                        await m_clProgrammer.StartAsync(m_clTarget);
                    }

                    //rollback changes
                    await this.DeployServoDataAsync(m_fOriginalFlags, m_uOriginalStartPos, m_uOriginalEndPos, m_uOriginalMsOperationTime);
                }

                //Is programmer still running?
                if (m_clProgrammer.IsRunning())
                    await m_clProgrammer.StopAsync();
            }
            catch(Exception ex)
            {
                MessageBox.Show(null, "Data update failed: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            
            base.OnFormClosing(e);                        
        }

        private void m_numStartAngle_ValueChanged(object sender, EventArgs e)
        {
            int pos = (int)m_numStartAngle.Value;
            m_numEndAngle.Minimum = pos + 1;

            this.OnDataChanged();

            this.UpdatePosition(pos);
        }

        private void m_numEndAngle_ValueChanged(object sender, EventArgs e)
        {
            int pos = (int)m_numEndAngle.Value;
            m_numStartAngle.Maximum = pos - 1;

            this.OnDataChanged();

            this.UpdatePosition(pos);
        }

        private void GotoFailureMode(string reason)
        {
            m_fFailed = true;

            this.OnDataCommited();

            this.EnableProgMode(false);
            this.EnableTestMode(false);
            
            m_cbTestMode.Enabled = false;

            m_lblStatus.Text = reason;

            MessageBox.Show(this, reason, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        private async void UpdatePosition(int position)
        {
            //Constructor will fire events when initialzing, ignore...
            if (!m_clProgrammer.IsRunning())
                return;

            try
            {
                await m_clProgrammer.SetPositionAsync(position);                
            }
            catch(Exception ex)
            {
                GotoFailureMode("Request failed: " + ex.Message);                
            }            
        }

        private ServoTurnoutFlags ExtractFlags()
        {
            ServoTurnoutFlags flags = 0;
            
            flags |= m_cbInverted.Checked ? ServoTurnoutFlags.SRVT_INVERTED_OPERATION : 0;
            flags |= m_cbIgnoreSaveState.Checked ? ServoTurnoutFlags.SRVT_IGNORE_SAVED_STATE : 0;
            flags |= m_cbActivateOnPowerUp.Checked ? ServoTurnoutFlags.SRVT_ACTIVATE_ON_POWER_UP : 0;
            flags |= m_cbInvertedFrog.Checked ? ServoTurnoutFlags.SRVT_INVERTED_FROG : 0;
            flags |= m_cbInvertedPower.Checked ? ServoTurnoutFlags.SRVT_INVERTED_POWER : 0;

            return flags;
        }

        private async Task DeployFormServoDataAsync()
        {
            await this.DeployServoDataAsync(
                this.ExtractFlags(), 
                (uint)m_numStartAngle.Value, 
                (uint)m_numEndAngle.Value, 
                uint.Parse(m_tbOperationTime.Text)
            );
        }

        private async Task DeployServoDataAsync(ServoTurnoutFlags flags, uint startPos, uint endPos, uint operationTime)
        {
            await m_clProgrammer.DeployAsync(flags, startPos, endPos, operationTime);            

            //data change deployed...
            this.OnDataCommited();
        }

        private async void m_cbTestMode_CheckedChanged(object sender, EventArgs e)
        {
            m_cbTestMode.Enabled = false;

            if (m_cbTestMode.Checked)
            {
                this.EnableProgMode(false);                

                try
                {
                    await DeployFormServoDataAsync();                    
                }
                catch (Exception ex)
                {
                    GotoFailureMode("Deploy failed: " + ex.Message);                    

                    return;
                }

                this.EnableTestMode(true);                
            }
            else
            {
                this.EnableTestMode(false);

                await StartServoProgrammerAsync();

                this.EnableProgMode(true);
            }

            m_cbTestMode.Enabled = true;
        }

        private async void m_btnThrow_Click(object sender, EventArgs e)
        {
            m_cbTestMode.Enabled = false;

            await m_clTarget.ActivateAsync();

            m_cbTestMode.Enabled = true;
        }

        private async void m_btnClose_Click(object sender, EventArgs e)
        {
            m_cbTestMode.Enabled = false;

            await m_clTarget.DeactivateAsync();

            m_cbTestMode.Enabled = true;
        }

        private async void m_btnFlip_Click(object sender, EventArgs e)
        {
            m_cbTestMode.Enabled = false;

            await m_clTarget.FlipAsync();            

            m_cbTestMode.Enabled = true;
        }

        private void m_btnCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void m_btnOK_Click(object sender, EventArgs e)
        {
            this.EnableProgMode(false);
            this.EnableTestMode(false);

            this.Close();
        }

        private void m_tbOperationTime_Validated(object sender, EventArgs e)
        {
            this.OnDataChanged();
        }

        private void Flags_CheckedChanged(object sender, EventArgs e)
        {
            this.OnDataChanged();
        }

        private void m_lnkStartPos_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            this.UpdatePosition((int)m_numStartAngle.Value);
        }

        private void m_lnkEndPos_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            this.UpdatePosition((int)m_numEndAngle.Value);
        }
    }   
}
