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
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
    class ServoTurnoutProxy : SharpCommon.IServoTurnout
    {
        private readonly RemoteTurnoutDecoder m_clRemoteTurnout;
        private readonly IConsole m_clConsole;

        public ServoTurnoutProxy(RemoteTurnoutDecoder turnout, IConsole console)
        {
            m_clRemoteTurnout = turnout ?? throw new ArgumentNullException(nameof(turnout));
            m_clConsole = console ?? throw new ArgumentNullException(nameof(console));
        }

        public string Name => m_clRemoteTurnout.Name;

        public SharpCommon.ServoTurnoutFlags Flags => m_clRemoteTurnout.Flags;

        public uint StartPos => m_clRemoteTurnout.StartPos;

        public uint EndPos => m_clRemoteTurnout.EndPos;

        public uint MsOperationTime => m_clRemoteTurnout.m_msOperationTime;

        public async Task ActivateAsync()
        {
            await m_clConsole.RequestAsync("Activate-Item", m_clRemoteTurnout.SystemName, m_clRemoteTurnout.Name);
        }

        public async Task DeactivateAsync()
        {
            await m_clConsole.RequestAsync("Deactivate-Item", m_clRemoteTurnout.SystemName, m_clRemoteTurnout.Name);
        }

        public async Task FlipAsync()
        {
            await m_clConsole.RequestAsync("Flip-Item", m_clRemoteTurnout.SystemName, m_clRemoteTurnout.Name);
        }
    }

    class ServoProgrammerProxy : SharpCommon.IServoProgrammer
    {
        private readonly IConsole m_clConsole;

        private readonly RemoteTurnoutDecoder m_clTarget;

        private bool m_fStartRequested = false;
        private int m_iProgrammerTaskId = -1;

        public ServoProgrammerProxy(RemoteTurnoutDecoder turnout, IConsole console)
        {
            m_clTarget = turnout ?? throw new ArgumentNullException(nameof(turnout));
            m_clConsole = console ?? throw new ArgumentNullException(nameof(console));
        }

        public async Task DeployAsync(SharpCommon.ServoTurnoutFlags flags, uint startPos, uint endPos, uint msOperationTime)
        {
            if (m_iProgrammerTaskId < 0)
                throw new InvalidOperationException("m_iProgrammerTaskId invalid");

            await m_clConsole.RequestAsync(
                       "Deploy-ServoProgrammer",
                       m_iProgrammerTaskId,
                       (uint)flags,
                       startPos,
                       endPos,
                       msOperationTime
           );

            //
            //no more task id
            m_iProgrammerTaskId = -1;
            m_fStartRequested = false;
        }

        public bool IsRunning()
        {
            return m_fStartRequested;
        }

        public async Task SetPositionAsync(int position)
        {
            if (m_iProgrammerTaskId < 0)
                throw new InvalidOperationException("m_iProgrammerTaskId invalid");

            await m_clConsole.RequestAsync("Edit-ServoProgrammer", m_iProgrammerTaskId, "position", position);
        }

        public async Task StartAsync(SharpCommon.IServoTurnout turnout)
        {
            if (m_fStartRequested)
                throw new InvalidOperationException("Start already called");

            m_fStartRequested = true;
            var json = await m_clConsole.RequestAsync("Start-ServoProgrammer", m_clTarget.SystemName, m_clTarget.DeviceName, m_clTarget.Name);

            m_iProgrammerTaskId = (int)json["taskId"];
        }

        public async Task StopAsync()     
        {
            if(m_iProgrammerTaskId < 0)
                throw new InvalidOperationException("m_iProgrammerTaskId invalid");

            await m_clConsole.RequestAsync("Stop-ServoProgrammer", m_iProgrammerTaskId.ToString());

            m_iProgrammerTaskId = -1;
            m_fStartRequested = false;
        }
    }

    public class ServoProgrammerAction : RemoteDecoderCmdBaseAction
    {
        private List<SharpCommon.ServoTurnoutProgrammerForm> m_lstOpenForms;

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

            var turnout = (RemoteTurnoutDecoder)target;

            var form = new SharpCommon.ServoTurnoutProgrammerForm(
                new ServoProgrammerProxy(turnout, console),
                new ServoTurnoutProxy(turnout, console)
            );

            this.Register(form);
            form.FormClosed += Form_FormClosed;

            form.Show();

            return;
        }

        private void Form_FormClosed(object sender, FormClosedEventArgs e)
        {
            var form = (SharpCommon.ServoTurnoutProgrammerForm)sender;
            this.Unregister(form);

            form.FormClosed -= Form_FormClosed;
        }

        private void Register(SharpCommon.ServoTurnoutProgrammerForm form)
        {
            if (m_lstOpenForms == null)
                m_lstOpenForms = new();

            m_lstOpenForms.Add(form);
        }

        private void Unregister(SharpCommon.ServoTurnoutProgrammerForm form)
        {
            m_lstOpenForms.Remove(form);
        }
    }

}
