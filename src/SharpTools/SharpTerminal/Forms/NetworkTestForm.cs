using System;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Forms
{
	[SupportedOSPlatform("windows")]
	public partial class NetworkTestForm : Form
	{
		readonly RemoteDevice mDevice;
		readonly IConsole mConsole;

		int m_iTaskId = -1;

		protected NetworkTestForm()
		{
			InitializeComponent();
		}

		public NetworkTestForm(RemoteDevice device, IConsole console) :
			this()
		{
			mDevice = device ?? throw new ArgumentNullException(nameof(device));
			mConsole = console ?? throw new ArgumentNullException(nameof(console));

			m_lblLostPacketsCount.Text = m_lblOutOfSyncPacketsCount.Text = m_lblReceivedPacketsCount.Text = m_lblSentPackets.Text = "0";

			m_lblLatency.Text = "";

			m_lblStatus.Text = "Idle";
		}

		private async void m_btnStart_Click(object sender, EventArgs e)
		{
			try
			{
				m_lblStatus.Text = "Starting...";

				m_btnStart.Enabled = false;
				var result = await mConsole.RequestAsync(["Start-NetworkTest", mDevice.Path]);

				m_iTaskId = (int)result["taskId"];
				m_btnStop.Enabled = true;

				m_tTimer.Interval = 1000;
				m_tTimer.Start();

				m_lblStatus.Text = "Running...";
			}
			catch (Exception ex)
			{
				m_btnStart.Enabled = true;

				m_lblStatus.Text = "Start failed: " + ex.Message;
			}
		}

		private async void m_btnStop_Click(object sender, EventArgs e)
		{
			//should never happen...
			if (m_iTaskId < 0)
				return;

			try
			{
				m_lblStatus.Text = "Stopping...";

				m_btnStop.Enabled = false;
				m_tTimer.Stop();
				await mConsole.RequestAsync(["Stop-NetworkTest", m_iTaskId]);

				m_lblStatus.Text = "Idle";
			}
			catch (Exception ex)
			{
				m_lblStatus.Text = "Stop failed: " + ex.Message;
			}

			m_btnStart.Enabled = true;
			m_iTaskId = -1;
		}

		private void NetworkTestForm_FormClosed(object sender, FormClosedEventArgs e)
		{
			if (m_iTaskId < 0)
				return;

			this.m_btnStop_Click(sender, e);
		}

		private async void m_tTimer_Tick(object sender, EventArgs e)
		{
			try
			{
				var result = await mConsole.RequestAsync(["Receive-NetworkTestData", m_iTaskId]);

				m_lblSentPackets.Text = result["sentPacketsCount"].ToString();
				m_lblReceivedPacketsCount.Text = result["receivedPacketsCount"].ToString();
				m_lblLostPacketsCount.Text = result["lostPacketsCount"].ToString();
				m_lblOutOfSyncPacketsCount.Text = result["outOfSyncPacketsCount"].ToString();
				m_lblLatency.Text = result["latency"].ToString() + "ms";
			}
			catch (Exception ex)
			{
				this.m_btnStop_Click(sender, e);

				m_lblStatus.Text = "Receive failed: " + ex.Message;
			}
		}

		private void button1_Click(object sender, EventArgs e)
		{
			this.Close();
		}
	}
}
