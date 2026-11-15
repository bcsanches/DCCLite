using System;
using System.Data;
using System.Runtime.Versioning;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public partial class DashboardUserControl : UserControl
	{
		public DashboardUserControl()
		{
			InitializeComponent();
		}

		public DashboardUserControl(IConsole console, RemoteRoot root) :
			this()
		{
			mConsole = console ?? throw new ArgumentNullException(nameof(console));
			mRoot = root ?? throw new ArgumentNullException(nameof(root));

			this.FillLogListView();
			this.ShowListViewLastLine();

			RemoteLogBuffer.MsgArrived += RemoteLogBuffer_MsgArrived;
		}

		private readonly IConsole mConsole;
		private readonly RemoteRoot mRoot;

		private int mCurrentChar = 0;

		private static char[] gChars = { '|', '/', '-', '\\' };

		private void mLoadTimer_Tick(object sender, EventArgs e)
		{
			++mCurrentChar;

			if (mCurrentChar == gChars.Length)
				mCurrentChar = 0;

			m_lbTitle.Text = "Loading data " + gChars[mCurrentChar];
		}

		private async void LoadNames()
		{
			mLoadTimer_Tick(null, null);

			mLoadTimer.Start();

			//Visual Studio Designer?
			if (mConsole == null)
				return;

			var result = await mConsole.RequestAsync("Get-RNames");

			if (result["classname"] != "RNames")
			{
				m_lbTitle.Text = "Error loading data";

				MessageBox.Show("Error: classname is " + result["classname"]);
			}
			else
			{
				m_lbTitle.Text = "Broker data";

				var namesData = result["rnames"];

				//
				//Use a DataTable because it is much, much faster than adding directly to the DataGridView
				// https://stackoverflow.com/a/36770807/440867
				var dataTable = new DataTable();

				dataTable.Columns.Add("Index");
				dataTable.Columns["Index"].DataType = typeof(int);

				dataTable.Columns.Add("Pos");
				dataTable.Columns["Pos"].DataType = typeof(int);

				dataTable.Columns.Add("Cluster");
				dataTable.Columns["Cluster"].DataType = typeof(int);

				dataTable.Columns.Add("Length");
				dataTable.Columns["Length"].DataType = typeof(int);

				dataTable.Columns.Add("Name");

				for (int i = 0; i < namesData.Count; ++i)
				{
					var nameData = namesData[i];

					var row = dataTable.NewRow();
					row["Pos"] = (int)nameData["position"];
					row["Index"] = (int)nameData["index"];
					row["Cluster"] = (int)nameData["cluster"];

					var rname = nameData["name"].ToString();

					row["Length"] = rname.Length;
					row["Name"] = rname;

					dataTable.Rows.Add(row);
				}

				m_gridMain.SuspendLayout();
				m_gridMain.Columns.Clear();
				m_gridMain.DataSource = dataTable;

				for (int i = 0; i < 4; ++i)
					m_gridMain.Columns[i].AutoSizeMode = DataGridViewAutoSizeColumnMode.AllCells;

				m_gridMain.Columns[4].AutoSizeMode = DataGridViewAutoSizeColumnMode.Fill;

				m_gridMain.ResumeLayout();
			}
			mLoadTimer.Stop();
		}


		private void m_pgNames_Enter(object sender, EventArgs e)
		{
			m_pgNames.Enter -= m_pgNames_Enter;

			LoadNames();
		}

		private void AddLogLine(LogMsg lmsg)
		{
			var item = m_lvLog.Items.Add(lmsg.Time.ToString());
			item.SubItems.Add(lmsg.Msg);
		}

		private void FillLogListView()
		{
			var msgs = RemoteLogBuffer.GetMsgs();
			foreach (var msg in msgs)
			{
				this.AddLogLine(msg);
			}
		}

		private void ShowListViewLastLine()
		{
			if (m_lvLog.Items.Count > 0)
				m_lvLog.EnsureVisible(m_lvLog.Items.Count - 1);
		}

		private void m_tbFilter_TextChanged(object sender, EventArgs e)
		{
			var filter = m_tbFilter.Text;

			m_lvLog.SuspendLayout();
			m_lvLog.Items.Clear();

			if (string.IsNullOrWhiteSpace(filter))
			{
				this.FillLogListView();
			}
			else
			{
				var msgs = RemoteLogBuffer.GetMsgs();				

				foreach (var m in msgs)
				{
					if (m.Msg.Contains(filter))
						this.AddLogLine(m);
				}
			}

			this.ShowListViewLastLine();

			m_lvLog.ResumeLayout();
		}		

		private void RemoteLogBuffer_MsgArrived(object sender, MsgArrivedEventArgs e)
		{
			if(this.InvokeRequired)
			{
				this.Invoke(new EventHandler<MsgArrivedEventArgs>(RemoteLogBuffer_MsgArrived), sender, e);
				return;
			}

			m_lvLog.SuspendLayout();

			AddLogLine(e.Msg);
			this.ShowListViewLastLine();

			m_lvLog.ResumeLayout();
		}
	}
}
