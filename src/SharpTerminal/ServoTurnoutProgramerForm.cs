// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace SharpTerminal
{    
    public partial class ServoTurnoutProgrammerForm: Form
    {
        private ServoProgrammerAction   m_clOwner;
        private RemoteTurnoutDecoder    m_clTarget;
        private IConsole                m_clConsole;

        private int m_iProgrammerTaskId = -1;

        private bool m_fFailed;

        public RemoteTurnoutDecoder Target { get { return m_clTarget; } }

        protected ServoTurnoutProgrammerForm()
        {
            InitializeComponent();                       
        }

        public ServoTurnoutProgrammerForm(ServoProgrammerAction owner, IConsole console, RemoteTurnoutDecoder target)
        {
            m_clOwner = owner ?? throw new ArgumentNullException(nameof(owner));
            m_clOwner.Register(this);

            m_clTarget = target ?? throw new ArgumentNullException(nameof(target));
            m_clConsole = console ?? throw new ArgumentNullException(nameof(console));

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

            m_tbOperationTime.Text = m_clTarget.m_msOperationTime.ToString();

            m_cbInverted.Checked = m_clTarget.InvertedOperation;
            m_cbInverted.Tag = ServoTurnoutFlags.SRVT_INVERTED_OPERATION;

            m_cbIgnoreSaveState.Checked = m_clTarget.IgnoreSaveState;
            m_cbIgnoreSaveState.Tag = ServoTurnoutFlags.SRVT_IGNORE_SAVED_STATE;

            m_cbInvertedFrog.Checked = m_clTarget.InvertedFrog;
            m_cbInvertedFrog.Tag = ServoTurnoutFlags.SRVT_INVERTED_FROG;

            m_cbInvertedPower.Checked = m_clTarget.InvertedPower;
            m_cbInvertedPower.Tag = ServoTurnoutFlags.SRVT_INVERTED_POWER;

            m_cbActivateOnPowerUp.Checked = m_clTarget.ActivateOnPowerUp;
            m_cbActivateOnPowerUp.Tag = ServoTurnoutFlags.SRVT_ACTIVATE_ON_POWER_UP;

            m_lblStatus.Text = "Connecting...";
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
        }

        private void EnableTestMode(bool enable)
        {
            m_btnFlip.Enabled = enable;
            m_btnClose.Enabled = enable;
            m_btnThrow.Enabled = enable;    
        }

        private async void AsyncStartServoProgrammer()
        {
            for (; ; )
            {
                try
                {
                    var json = await m_clConsole.RequestAsync("Start-ServoProgrammer", m_clTarget.SystemName, m_clTarget.DeviceName, m_clTarget.Name);

                    if (json.ContainsKey("taskId"))
                    {
                        m_iProgrammerTaskId = (int)json["taskId"];
                        m_lblStatus.Text = "Connected, task Id " + m_iProgrammerTaskId.ToString();

                        EnableProgMode(true);
                    }

                    break;
                }
                catch (Exception ex)
                {
                    if (MessageBox.Show(this, "Failed to start programming mode: " + ex.Message + ".\n Retry??", "Error", MessageBoxButtons.RetryCancel) != DialogResult.Retry)
                    {
                        this.DialogResult = DialogResult.Cancel;
                        this.Close();
                    }
                }
            }
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            if (this.DesignMode)
                return;

            this.AsyncStartServoProgrammer();                                  
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            if (!m_fFailed && (MessageBox.Show(this, "Are you sure? Changes will not be saved", "Are you sure?", MessageBoxButtons.YesNo, MessageBoxIcon.Question) != DialogResult.Yes))
            {
                e.Cancel = true;

                return;
            }

            base.OnFormClosing(e);

            m_clOwner.Unregister(this);

            m_clConsole.ProcessCmd("Stop-ServoProgrammer", m_iProgrammerTaskId.ToString());
        }

        private void m_numStartAngle_ValueChanged(object sender, EventArgs e)
        {
            int pos = (int)m_numStartAngle.Value;
            m_numEndAngle.Minimum = pos + 1;

            this.UpdatePosition(pos);
        }

        private void m_numEndAngle_ValueChanged(object sender, EventArgs e)
        {
            int pos = (int)m_numEndAngle.Value;

            m_numStartAngle.Maximum = pos - 1;
            this.UpdatePosition(pos);
        }

        private void GotoFailureMode(string reason)
        {
            m_fFailed = true;

            this.EnableProgMode(false);
            this.EnableTestMode(false);
            
            m_cbTestMode.Enabled = false;

            m_lblStatus.Text = reason;

            MessageBox.Show(this, reason, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        private async void UpdatePosition(int position)
        {
            //Constructor fill fire events when initialzing, ignore...
            if (m_iProgrammerTaskId < 0)
                return;

            try
            {
                await m_clConsole.RequestAsync("Edit-ServoProgrammer", m_iProgrammerTaskId, "position", position);
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

        private async void m_cbTestMode_CheckedChanged(object sender, EventArgs e)
        {
            m_cbTestMode.Enabled = false;

            if (m_cbTestMode.Checked)
            {
                this.EnableProgMode(false);                

                try
                {
                    await m_clConsole.RequestAsync(
                        "Deploy-ServoProgrammer", 
                        m_iProgrammerTaskId, 
                        this.ExtractFlags(), 
                        m_numStartAngle.Value, 
                        m_numEndAngle.Value, 
                        int.Parse(m_tbOperationTime.Text)
                    );
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

                AsyncStartServoProgrammer();

                this.EnableProgMode(true);
            }

            m_cbTestMode.Enabled = true;
        }
    }

    public class ServoProgrammerAction : RemoteDecoderCmdBaseAction
    {
        private List<ServoTurnoutProgrammerForm> m_lstOpenForms;

        public ServoProgrammerAction(string label, string description) :
            base(label, description)
        {
            //empty
        }

        public override void Execute(IConsole console, RemoteObject target)
        {
            if (m_lstOpenForms != null)
            {
                var existingForm = (from x in m_lstOpenForms where x.Target.Name.CompareTo(target.Name) == 0 select x).FirstOrDefault();

                if (existingForm != null)
                {
                    existingForm.Focus();

                    return;
                }
            }

            var form = new ServoTurnoutProgrammerForm(this, console, (RemoteTurnoutDecoder)target);

            form.Show();

            return;
        }

        internal void Register(ServoTurnoutProgrammerForm form)
        {
            if (m_lstOpenForms == null)
                m_lstOpenForms = new();

            m_lstOpenForms.Add(form);
        }

        internal void Unregister(ServoTurnoutProgrammerForm form)
        {
            m_lstOpenForms.Remove(form);
        }
    }

}
