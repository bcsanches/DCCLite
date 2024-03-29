﻿
using System;
using System.Windows.Forms;

namespace SharpTerminal
{
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
