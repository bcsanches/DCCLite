using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Tycoon.Forms
{
	public partial class SpotReservationInfoForm : Form
	{
		public SpotReservationInfoForm()
		{
			InitializeComponent();
		}

		[SupportedOSPlatform("windows")]
		public string InfoText
		{
			get => m_tbInfo.Text;
			set => m_tbInfo.Text = value;
		}
	}
}
