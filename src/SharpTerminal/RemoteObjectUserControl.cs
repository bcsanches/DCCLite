
using System.Drawing;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class RemoteObjectUserControl : UserControl
    {
        private readonly RemoteObject mObject;

        public RemoteObjectUserControl()
        {
            InitializeComponent();
        }

        public RemoteObjectUserControl(RemoteObject remoteObject) :
            this()
        {
            mObject = remoteObject ?? throw new System.ArgumentNullException(nameof(remoteObject));

            m_lbTitle.Text += " - " + remoteObject.Name;

            mPropertyGrid.SelectedObject = remoteObject;

            remoteObject.PropertyChanged += RemoteDecoder_PropertyChanged;
        }

        private void RemoteDecoder_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            mPropertyGrid.Refresh();
        }
    }
}
