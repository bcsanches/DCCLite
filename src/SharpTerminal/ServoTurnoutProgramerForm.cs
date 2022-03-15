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
    public class ServoProgrammerAction: RemoteDecoderCmdBaseAction
    {
        private List<ServoTurnoutProgrammerForm> m_lstOpenForms;

        public ServoProgrammerAction(string label, string description):
            base(label, description)
        {
            //empty
        }

        public override void Execute(IConsole console, RemoteObject target)
        {            
            if(m_lstOpenForms != null)
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

    public partial class ServoTurnoutProgrammerForm: Form
    {
        private ServoProgrammerAction   m_clOwner;
        private RemoteTurnoutDecoder    m_clTarget;
        private IConsole                m_clConsole;

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

            m_clConsole.ProcessCmd("Start-ServoProgrammer", target.SystemName, target.DeviceName, target.Name);
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);            
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            if (MessageBox.Show(this, "Are you sure? Changes will not be saved", "Are you sure?", MessageBoxButtons.YesNo, MessageBoxIcon.Question) != DialogResult.Yes)
            {
                e.Cancel = true;

                return;
            }

            base.OnFormClosing(e);

            m_clOwner.Unregister(this);

            m_clConsole.ProcessCmd("Stop-ServoProgrammer", m_clTarget.SystemName, m_clTarget.Name);
        }
    }
}
