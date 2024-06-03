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
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public partial class RemoteShortcutUserControl : UserControl
    {
        private readonly RemoteShortcut mRemoteShortcut;
        private readonly IConsole mConsole;

        public RemoteShortcutUserControl()
        {
            InitializeComponent();
        }

        public RemoteShortcutUserControl(IConsole console, RemoteShortcut remoteShortcut) :
            this()
        {
            mRemoteShortcut = remoteShortcut ?? throw new ArgumentNullException(nameof(remoteShortcut));
            mConsole = console ?? throw new ArgumentNullException(nameof(console));

            m_lbTitle.Text += " - " + remoteShortcut.Name;

            m_lbLoadingMessage.Text = "Loading " + remoteShortcut.TargetName;
        }

        protected override async void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            //Design mode?
            if (mRemoteShortcut == null)
                return;

            var obj = await mRemoteShortcut.GetTargetAsync();
            if (obj == null)
            {
                m_lbLoadingMessage.Text = "Failed to load " + mRemoteShortcut.TargetName;

                return;
            }                

            var control = obj.CreateControl(mConsole);

            mPanel.Controls.Remove(m_lbLoadingMessage);

            control.Dock = DockStyle.Fill;
            mPanel.Controls.Add(control);
        }
    }
}
