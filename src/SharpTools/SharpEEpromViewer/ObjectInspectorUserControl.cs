

using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpEEPromViewer
{
	[SupportedOSPlatform("windows")]
	public partial class ObjectInspectorUserControl : UserControl
    {        
        public ObjectInspectorUserControl()
        {
            InitializeComponent();
        }

        internal ObjectInspectorUserControl(object obj) :
            this()
        {
            if(obj == null)
                throw new System.ArgumentNullException(nameof(obj));

            m_lbTitle.Text += (obj is Lump lump) ? lump.Name : obj.GetType().Name;

            mPropertyGrid.SelectedObject = obj;            
        }

        private void RemoteDecoder_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            mPropertyGrid.Refresh();
        }
    }
}
