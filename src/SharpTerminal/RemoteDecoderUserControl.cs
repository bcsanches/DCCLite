
using System.Drawing;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class RemoteDecoderUserControl : UserControl
    {
        private readonly RemoteDecoder mDecoder;

        public RemoteDecoderUserControl()
        {
            InitializeComponent();
        }

        public RemoteDecoderUserControl(RemoteDecoder remoteDecoder) :
            this()
        {            
            mDecoder = remoteDecoder ?? throw new System.ArgumentNullException(nameof(remoteDecoder));

            m_lbTitle.Text += " - " + remoteDecoder.Name;

            mPropertyGrid.SelectedObject = remoteDecoder;

            remoteDecoder.PropertyChanged += RemoteDecoder_PropertyChanged;
        }

        private void RemoteDecoder_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            mPropertyGrid.Refresh();
        }
    }
}
