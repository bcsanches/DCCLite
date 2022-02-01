
using System;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class RemoteShortcutUserControl : UserControl
    {
        private readonly RemoteShortcut mRemoteShortcut;

        public RemoteShortcutUserControl()
        {
            InitializeComponent();
        }

        public RemoteShortcutUserControl(RemoteShortcut remoteShortcut) :
            this()
        {
            mRemoteShortcut = remoteShortcut ?? throw new ArgumentNullException(nameof(remoteShortcut));

            m_lbTitle.Text += " - " + remoteShortcut.Name;

            m_lbLoadingMessage.Text = "Loading " + remoteShortcut.Target;
        }

        protected override async void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            //Design mode?
            if (mRemoteShortcut == null)
                return;

            var obj = await RemoteObjectManager.GetRemoteObjectAsync(mRemoteShortcut.Target);
            if (obj == null)
                return;

            var control = obj.CreateControl();
            if (control == null)
            {
                m_lbLoadingMessage.Text = "Object " + mRemoteShortcut.Target + "loaded - no control";

                return;
            }                

            mPanel.Controls.Remove(m_lbLoadingMessage);

            control.Dock = DockStyle.Fill;
            mPanel.Controls.Add(control);
        }
    }
}
