using SharpTerminal.Tycoon;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Forms
{
	public partial class RemoteTycoonUserControl : UserControl
	{
		public RemoteTycoonUserControl()
		{
			InitializeComponent();
		}

		[SupportedOSPlatform("windows")]
		public RemoteTycoonUserControl(IConsole console, RemoteTycoonService remoteTycoon) :
			this()
		{
			m_lbTitle.Text = remoteTycoon.Name;
		}		
	}
}
