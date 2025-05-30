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
	public partial class RemoteObjectUserControl : UserControl
    {
        private readonly RemoteObject mObject;
        private readonly IConsole mConsole;

        public RemoteObjectUserControl()
        {
            InitializeComponent();
        }

        public RemoteObjectUserControl(IConsole console, RemoteObject remoteObject) :
            this()
        {
            mObject = remoteObject ?? throw new System.ArgumentNullException(nameof(remoteObject));
            mConsole = console ?? throw new System.ArgumentNullException(nameof(console));

            m_lbTitle.Text = remoteObject.ClassName + " - " + remoteObject.Name;

            mPropertyGrid.SelectedObject = remoteObject;

            remoteObject.PropertyChanged += RemoteDecoder_PropertyChanged;

            var actions = remoteObject.GetActions();
            if (actions != null)
            {
                foreach (var action in actions)
                {
                    var button = new Button
                    {
                        Text = action.GetLabel(),
                        Tag = action,
                        AutoSize = true
                    };

                    button.Click += pnlButtons_ButtonClick; 

                    m_pnlButtons.Controls.Add(button);                                        
                }
            }            
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            mPropertyGrid.Enabled = false;
        }

        private void pnlButtons_ButtonClick(object sender, System.EventArgs e)
        {
            var action = (IRemoteObjectAction)((Button)sender).Tag;
            action.Execute(mConsole, mObject);
        }

        private void RemoteDecoder_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            mPropertyGrid.Refresh();
        }
    }
}
