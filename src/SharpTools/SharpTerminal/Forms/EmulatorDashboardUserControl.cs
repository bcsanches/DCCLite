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

		private void RefreshEmulatorStatus(Emulator emulator)
		{
			foreach (ListViewItem item in m_lvEmulators.Items)
			{
				if (item.Tag == emulator)
				{
					item.SubItems[1].Text = emulator.HasExited ? "Exited" : "Running";
					break;
				}
			}
		}

		private void OnEmulatorExited(object sender, EventArgs e)
		{
			if (this.InvokeRequired)
			{
				this.Invoke(() =>
				{
					this.OnEmulatorExited(sender, e);
				});

				return;
			}

			RefreshEmulatorStatus(sender as Emulator);			

			m_lvEmulators_SelectedIndexChanged(sender, e);
		}

		private Emulator SelectedEmulator
		{
			get
			{
				return m_lvEmulators.SelectedItems.Count > 0 ? m_lvEmulators.SelectedItems[0].Tag as Emulator : null;
			}
		}

		private void m_lvEmulators_SelectedIndexChanged(object sender, EventArgs e)
		{
			var emulator = this.SelectedEmulator;
			if (emulator == null)
			{
				m_btnKill.Enabled = false;
				m_btnRestart.Enabled = false;
			}
			else
			{
				var exited = emulator.HasExited;

				m_btnKill.Enabled = !exited;
				m_btnRestart.Enabled = exited;
			}

		}

		private void m_btnKill_Click(object sender, EventArgs e)
		{
			var emulator = this.SelectedEmulator;
			emulator.Kill();
		}

		private void m_btnRestart_Click(object sender, EventArgs e)
		{
			var emulator = this.SelectedEmulator;
			emulator.Restart();

			this.RefreshEmulatorStatus(emulator);
		}
	}
}
