

using System;
using System.Windows.Forms;

namespace SharpTerminal
{
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
                        Tag = action
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
