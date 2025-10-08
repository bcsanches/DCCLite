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
		}
		
		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);			

			var emulators = EmulatorManager.GetEmulators();

			foreach (var emulator in emulators)
			{
				var item = m_lvEmulators.Items.Add(emulator.DeviceName);
				item.SubItems.Add(emulator.HasExited ? "Exited" : "Running");

				item.Tag = emulator;

				emulator.AddExitHandler(OnEmulatorExited);
			}
		}

		private void OnEmulatorExited(object sender, EventArgs e)
		{
			if(this.InvokeRequired)
			{
				this.Invoke(() =>
				{
					this.OnEmulatorExited(sender, e);
				});

				return;
			}			

			foreach (ListViewItem item in m_lvEmulators.Items)
			{
				if (item.Tag == sender)
				{
					item.SubItems[1].Text = "Exited";
					break;
				}
			}
		}
	}
}
