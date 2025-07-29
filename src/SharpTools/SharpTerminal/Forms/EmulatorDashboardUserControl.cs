using System;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Forms
{
	[SupportedOSPlatform("windows")]
	public partial class EmulatorDashboardUserControl : UserControl
	{
		public EmulatorDashboardUserControl()
		{
			InitializeComponent();

			m_lvEmulators.Columns.Add("Name");
		}
		
		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);			

			var emulators = EmulatorManager.GetEmulators();

			foreach (var emulator in emulators)
			{
				var item = m_lvEmulators.Items.Add(emulator.DeviceName);

				item.Tag = emulator;
			}
		}
	}
}
