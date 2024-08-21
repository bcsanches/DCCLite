using System.Collections;
using System.Linq;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Forms
{
	[SupportedOSPlatform("windows")]
	public partial class RemoteDeviceRenameForm : Form
	{		
		public RemoteDeviceRenameForm(System.Collections.Generic.IEnumerable<string> possibleNames = null)
		{
			InitializeComponent();

			if (possibleNames == null)
				return;

			m_cbNames.DataSource = possibleNames.ToList();
		}

		public string SelectedName
		{
			get { return m_cbNames.Text; }
		}
	}
}
